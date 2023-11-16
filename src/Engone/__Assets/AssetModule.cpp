#include "Engone/Assets/AssetModule.h"

namespace engone {
	
	AssetStorage::AssetStorage() {
		m_running = true;
	}
	AssetStorage::~AssetStorage() {
		m_running = false;
		cleanup();
	}
	void AssetStorage::cleanup() {
		//ENGONE_DEBUG(log::out << "AssetStorage::cleanup - begun\n", ASSET_LEVEL,1)
		m_ioProcMutex.lock();
		for (int i = 0; i < m_ioProcessors.size(); i++) {
			//delete m_ioProcessors[i];
			ALLOC_DELETE(AssetProcessor, m_ioProcessors[i]);
		}
		m_ioProcessors.clear();
		m_ioProcMutex.unlock();

		m_dataProcMutex.lock();
		for (int i = 0; i < m_dataProcessors.size(); i++) {
			//delete m_dataProcessors[i];
			ALLOC_DELETE(AssetProcessor, m_dataProcessors[i]);
		}
		m_dataProcessors.clear();
		m_dataProcMutex.unlock();

		m_assetMutex.lock();
		for (auto& map : m_assets) {
			for (auto& pair : map.second) {
				if (pair.second) {
					// GetTracker().untrack(Asset::GetTrackerId(pair.second->type), pair.second);
					//GetTracker().untrack(, pair.second);
					
					// at this point we don't know whether free, Free or delete should be called.
					// crash happens if we call the wrong one. fix this later.
					//ALLOC_DELETE(Asset, pair.second); // ISSUE: BIG ISSUE THIS WILL NOT DELETE OPENGL BUFFERS UNLESS CLEANUP IS DONE ON THE RENDER THREAD!
				}
			}
		}
		m_assets.clear();
		// Possible fix is to make a vector of assets which needs context and give it to render thread somehow.
		// Storage can then be cleaned properly. Problems may arise if a model asset want to delete collider assets. but they would already have been deleted
		// if they existed on here. So then, all assets which was created by model assets will not be deleted here. Ownership system for all assets.
		// Most owned by asset storage.
		// When window is deleted, the context is deleted and so the buffers.
		// But say you want to only clean AssetStorage without destroying the window, then this WOULD be an issue.
		//log::out << log::RED << "AssetStorage::cleanup - some assets are not deleted on the opengl context!\n";
		m_assetMutex.unlock();
		//ENGONE_DEBUG(log::out << "AssetStorage::cleanup - finished\n", ASSET_LEVEL, 1)
	}
	void AssetStorage::addIOProcessor() {
		m_ioProcMutex.lock();
		if (m_running) {
			m_ioProcessors.push_back(ALLOC_NEW(AssetProcessor)(this, Asset::LoadIO));
			//m_ioProcessors.back()->start();
		}
		m_ioProcMutex.unlock();
	}
	void AssetStorage::addDataProcessor() {
		m_dataProcMutex.lock();
		if (m_running) {
			m_dataProcessors.push_back(ALLOC_NEW(AssetProcessor)(this, Asset::LoadData));
			//m_dataProcessors.back()->start();
		}
		m_dataProcMutex.unlock();
	}
	void AssetStorage::addGraphicProcessor() {
		m_graphicProcMutex.lock();
		if (m_running) {
			if (m_graphicProcessors.size() != 0) {
				log::out << log::RED << "AssetStorage::addGraphicProcessor - OpenGL GraphicProcessors does not have their own thread. More processors would not improve anything.\n";
			} else {
				m_graphicProcessors.push_back(ALLOC_NEW(AssetProcessor)(this, Asset::LoadGraphic));
				//m_graphicProcessors.back()->start(); // <- don't want to start it. It should run in OpenGL context thread
			}
		}
		m_graphicProcMutex.unlock();
	}
	void AssetStorage::printTasks() {
		int totalTasks = 0;
		m_ioProcMutex.lock();
		m_dataProcMutex.lock();
		m_graphicProcMutex.lock();

		for (auto p : m_ioProcessors) {
			totalTasks += p->numQueuedTasks();
		}
		for (auto p : m_dataProcessors) {
			totalTasks += p->numQueuedTasks();
		}
		for (auto p : m_graphicProcessors) {
			totalTasks += p->numQueuedTasks();
		}

		log::out << "Total Tasks: " << totalTasks<<"\n";
		log::out << "Processors: io/data/graphic: " << m_ioProcessors.size() << 
			" / " << m_dataProcessors.size() << " / "<< m_graphicProcessors.size() << "\n";

		m_ioProcMutex.unlock();
		m_dataProcMutex.unlock();
		m_graphicProcMutex.unlock();
	}
	void AssetStorage::processTask(AssetTask task) {
		//if (task.asset->m_path.empty()) return;
		if (task.asset->m_state == Asset::Failed) return;

		if (task.asset->m_flags & Asset::LoadIO) {
			m_ioProcMutex.lock();
			if (m_ioProcessors.size() == 0) {
				m_ioProcMutex.unlock();
				addIOProcessor();
				m_ioProcMutex.lock();
			}
			int index = m_ioNext;
			m_ioNext = (m_ioNext + 1) % m_ioProcessors.size();
			m_ioProcMutex.unlock();

			m_ioProcessors[index]->queue(task);
			return; // Note that we return because a task can only be processed by one processor at a time.
		}
		if (task.asset->m_flags & Asset::LoadData) {
			m_dataProcMutex.lock();
			if (m_dataProcessors.size() == 0) {
				m_dataProcMutex.unlock();
				addDataProcessor();
				m_dataProcMutex.lock();
			}
			int index = m_dataNext;
			m_dataNext = (m_dataNext + 1) % m_dataProcessors.size();
			m_dataProcMutex.unlock();

			m_dataProcessors[index]->queue(task);
			return;
		}
		if (task.asset->m_flags & Asset::LoadGraphic) {
			m_graphicProcMutex.lock();
			if (m_graphicProcessors.size() == 0) {
				m_graphicProcMutex.unlock();
				addGraphicProcessor();
				m_graphicProcMutex.lock();
			}
			int index = m_graphicNext;
			m_graphicNext = (m_graphicNext + 1) % m_graphicProcessors.size();
			m_graphicProcMutex.unlock();

			m_graphicProcessors[index]->queue(task);
			return;
		}
	}
	
	Asset* AssetStorage::load(AssetType type, const std::string& path, uint8_t flags) {
		// ask to load an asset as a task through processors.
		// reload asset if already loaded
		m_assetMutex.lock();
		std::string _path = modifyPath(type,path);
		
		//log::out << "load "<<path << "\n";

		Asset* asset = nullptr;
		
		//-- Find asset, (simular to AssetStorage::get)
		auto& list = m_assets[type];
		auto find = list.find(path);
		if (find != list.end()) {
			asset = find->second;
		}

		if (!asset) {
			//log::out << "Load " << path << "\n";
			//asset = ALLOC_NEW(T)();
			asset = allocateAsset(type);

			asset->setStorage(this);
			//GetTracker().track(asset);
			m_assets[type][path] = asset;
			_path += GetFormat(type);
			asset->setPath(_path);
			asset->setLoadName(path);
		}
		m_assetMutex.unlock();

		if (asset->m_state == Asset::Processing || asset->m_state == Asset::Loaded) // return if asset already is being processed
			return asset;

		asset->m_state = Asset::Processing;
		if (flags & SYNC) {
			// load returns flags but we can safely ignore them.

			//asset->load(Asset::LoadAll);
			asset->m_flags = asset->load(Asset::LoadIO | Asset::LoadData);
			if (asset->m_error != Error::NoError) {
				asset->m_flags = Asset::LoadNone;
				log::out << log::RED << "Failed loading: " << asset->getPath() << "\n";
			}
			if (asset->m_state & Asset::Loaded) {
				if (asset->getPath().size() != 0) {
					//ENGONE_DEBUG(log::out << "Loaded " << asset->getPath() << "\n", ASSET_LEVEL, 2)
				}
			} else {
				//ENGONE_DEBUG(log::out << "AssetModule::load - not sure?\n", ASSET_LEVEL, 1)
			}
			// may need to load graphics
			AssetTask task(asset);
			processTask(task);
		} else {
			asset->m_flags = Asset::LoadIO;
			AssetTask task(asset);
			processTask(task);
		}
		return asset;
	}
	void AssetStorage::unload(AssetType type, const std::string& path, uint8_t flags) {
		m_assetMutex.lock();
		std::string _path = modifyPath(type,path);

		auto& list = m_assets[type];
		auto find = list.find(path);
		if (find != list.end()) {
			if (find->second->m_state == Asset::Processing) {
				//log::out << "AssetStorage::unload - Asset cannot be unloaded while being processed!\n";
			} else {
				ALLOC_DELETE(Asset, find->second);
				//delete find->second; // can asset be deleted?
				//GetTracker().untrack(find->second);
				list.erase(find);
			}
		}
		m_assetMutex.unlock();
	}
	Asset* AssetStorage::get(AssetType type, const std::string& str, uint8_t flags) {
		m_assetMutex.lock();
		auto& list = m_assets[type];
		auto find = list.find(str);
		Asset* out = nullptr;
		if (find != list.end()) {
			out = find->second;
		}
		m_assetMutex.unlock();
		return out;
	}
	Asset* AssetStorage::set(AssetType type, const std::string& name, Asset* asset, uint8_t flags) {
		asset->m_path.clear(); // no path, asset cannot reload, you can of course change it's data yourself

		m_assetMutex.lock();

		auto& list = m_assets[type];
		auto find = list.find(name);
		Asset* t = nullptr;
		bool doStuff = false;
		if (find != list.end()) {
			t = find->second;
			if (t == asset) {
				// fine
			} else {
				// not fine
				// should maybe warn here
				//log::out << log::YELLOW << "AssetStorage::set - overwriting asset, deleting old one.\n";
				//ALLOC_DELETE(T,t); // ISSUE: DELETING DOES NOT WORK IF IT IS OPENGL STUFF
				doStuff = false;
				//doStuff = true;
				//t = asset;
			}
		} else {
			t = asset;
			doStuff = true;
		}
		if (doStuff) {
			t->setStorage(this);
			//GetTracker().track(asset);
			m_assets[type][name] = t;

			if (t->m_flags == Asset::LoadNone) {
				t->m_state = Asset::Loaded;
			} else {
				t->m_state = Asset::Processing;
				if (flags & SYNC) {
					//t->load(Asset::LoadAll);
					t->m_flags = t->load(Asset::LoadIO | Asset::LoadData);
					if (t->m_error != Error::NoError) {
						t->m_flags = Asset::LoadNone;
						log::out << log::RED << "Failed loading: " << t->getPath() << "\n";
					}
					if (t->m_state & Asset::Loaded) {
						if (t->getPath().size() != 0) {
							//ENGONE_DEBUG(log::out << "Loaded " << t->getPath() << "\n", ASSET_LEVEL, 2)
						}
					} else {
						//ENGONE_DEBUG(log::out << "AssetModule::load - not sure?\n", ASSET_LEVEL, 1)
					}
					AssetTask task(t);
					processTask(task);
				} else {
					AssetTask task(t);
					processTask(t);
				}
			}
		}
		m_assetMutex.unlock();

		return t;
	}
	Asset* AssetStorage::allocateAsset(AssetType type) {
		switch (type) {
		case AssetTexture: return ALLOC_NEW(TextureAsset)();
		case AssetFont: return ALLOC_NEW(FontAsset)();
		case AssetShader: return nullptr; //ALLOC_NEW(ShaderAsset)();
		case AssetMaterial: return ALLOC_NEW(MaterialAsset)();
		case AssetMesh: return ALLOC_NEW(MeshAsset)();
		case AssetAnimation: return ALLOC_NEW(AnimationAsset)();
		case AssetArmature: return ALLOC_NEW(ArmatureAsset)();
		case AssetCollider: return ALLOC_NEW(ColliderAsset)();
		case AssetModel: return ALLOC_NEW(ModelAsset)();
		}
		return nullptr;
	}
	std::string AssetStorage::modifyPath(AssetType type, const std::string& str) {
		std::string out;
		// optimize this
		if (!m_storagePath.empty() && str.find(m_storagePath)!=0)
			out += m_storagePath;

		if (type == ModelAsset::TYPE)
			out += "models/";

		out += str;

		return out;
	}
	
	AssetProcessor::AssetProcessor(AssetStorage* storage, Asset::LoadFlag type) 
		: m_storage(storage), m_processType(type) {
	}
	AssetProcessor::~AssetProcessor() {
		cleanup();
	}
	void AssetProcessor::cleanup() {
		
		if (m_thread.joinable()) {
			stop();
			m_thread.join();
		} else {
			// this is fine if it is gl processor, gl processor doesn't create a thread
			//log::out << log::RED << "AssetProcessor::stop - thread isn't joinable!\n";
		}
	}
	void AssetProcessor::start() {
		if (!m_running) {
			m_running = true;
			m_thread = std::thread([this]() {
				std::string name = "Proc. ";
				name += ToString(m_processType);
				engone::SetThreadName(-1, name.c_str());

				//ENGONE_DEBUG(log::out << "AssetProcessor::start - started thread\n", ASSET_LEVEL, 1)
				while (m_running) {

					process();

					std::unique_lock<std::mutex> lock(m_waitMutex);

					m_wait.wait(lock, [this]() {return m_queue.size() != 0||!m_running; });
					//ENGONE_DEBUG(log::out << "AssetProcessor::start - stopped waiting\n", ASSET_LEVEL, 1)
				}
			});
		}
	}
	void AssetProcessor::process() {
		m_queueMutex.lock();
		std::vector<AssetTask> tasks = std::move(m_queue); // moving will clear m_queue
		m_queueMutex.unlock();

		//for (AssetTask& task : tasks) {
			//task.finalAsset->m_state = Asset::Loading;
		//}
		for (AssetTask& task : tasks) {
			// process it
			//log::out << "Process("<<m_processType<<"): " << task.asset->getPath() << "\n";
			// 
			//if (this->m_processType == Asset::LoadGraphic) {
			//	DebugBreak();
			//}
			
			// NOTE: loading and asset twice with the same type would cause errors.
			//   loading mesh twice with type LoadData for example.
			Asset::LoadFlags flags = task.asset->load(m_processType);
			task.asset->m_flags = flags;
			if (task.asset->m_error != Error::NoError) {
				task.asset->m_flags = Asset::LoadNone;
				log::out << log::RED<<"Failed loading: " << task.asset->getPath() << "\n";
			}
			if (task.asset->m_state & Asset::Loaded) {
				if (task.asset->getPath().size() != 0) {
					//ENGONE_DEBUG(log::out << "Loaded " << task.asset->getPath()<<"\n", ASSET_LEVEL, 2)
				}
			} else {
				//ENGONE_DEBUG(log::out << "Loaded " << task.asset->getPath() << " next: " << flags << "\n", ASSET_LEVEL, 1)
			}
			//task.nextFlags = flags;
			// depending on asset type
			//task.finalAsset->m_state = Asset::Loaded;
		}
		// do more processing if necessary.
		for (AssetTask& t : tasks) {
			m_storage->processTask(t);
		}
	}
	void AssetProcessor::stop() {
		m_running = false;
		m_wait.notify_one();
	}
	void AssetProcessor::queue(AssetTask task) {
		//ENGONE_DEBUG(log::out << "Queue Task: " << task.asset->getPath() << " " << (Asset::LoadFlag)task.asset->m_flags << "\n", ASSET_LEVEL, 1)
		//std::lock_guard lk(m_waitMutex);
		m_waitMutex.lock();
		// Todo: Do mutex
		m_queueMutex.lock();
		m_queue.push_back(task);
		m_queueMutex.unlock();
		m_wait.notify_one();
		m_waitMutex.unlock();
	}
	int AssetProcessor::numQueuedTasks() {
		m_queueMutex.lock();
		int num = m_queue.size();
		m_queueMutex.unlock();
		return num;
	}
}