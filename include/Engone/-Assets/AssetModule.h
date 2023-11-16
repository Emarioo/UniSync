#pragma once

#include "Engone/Assets/AnimationAsset.h"
#include "Engone/Assets/ArmatureAsset.h"
#include "Engone/Assets/ColliderAsset.h"
#include "Engone/Assets/FontAsset.h"
#include "Engone/Assets/MaterialAsset.h"
#include "Engone/Assets/MeshAsset.h"
#include "Engone/Assets/ModelAsset.h"
#include "Engone/Assets/TextureAsset.h"
#include "Engone/Assets/ShaderAsset.h"

#include "Engone/Util/Utilities.h"
#include "Engone/Util/Alloc.h"
#include "Engone/Asserts.h"

/* -- AssetModule --

AssetStorage - Assets and processers are storeds here. (accessed from multiple threads)
Loader - one thread for reading and writing files (I/O)
Processer - Processes tasks from the loader into usable assets. (multiple threads)

OPENGL is not multithreaded which means only one thread is allowed
for OpenGL related things(this would be on the render thread).

AssetStorage has disc and non-disc storage.
Files can be loaded to disc storage.
Asses which you make can be added to non-disc storage. (shaders for example)

how does sync work with renderer thread

ask storage for an asset. It gives you a pointer to asset
You can specify flags. Sync or Async.
Sync will ensure that the returned asset isn't empty(it has been loaded)
Async will give you an asset which probably is empty but is loaded whenever the
processor is done

ask storage to reload an asset from disc if it has been updated on disc.
ask storage to reload all assets from disc.

purpose of unload is to clean assets and memory so that is what it should do. delete asset

Any loaded asset is assumed to be used by storage

MaterialAsset::load - requires opengl context when without storage because of diffuse_map which is a texture asset

cleanup on some assets require opengl context. Have a delete queue process? it would need to out live AssetStorage
could be destroyed before delete queue has finished. Waiting for delete queue and then destroying storage could be an option
unless storage is being destroyed on render thread in which case it might lock itself depending on how things are coded.

*/

#define ASSET_LEVEL_ALL 0xffffffff
#define ASSET_LEVEL_INFO 0x01
#define ASSET_LEVEL_WARNING 0x02
#define ASSET_LEVEL_ERROR 0x04

namespace engone {
	struct AssetTask {
		//AssetTask(Asset* asset, Asset::LoadFlags flags) : asset(asset) , nextFlags(flags) {}
		AssetTask(Asset* asset) : asset(asset) {}
		Asset* asset;
		//Asset::LoadFlags nextFlags;
		//enum State : uint8_t {
		//	None = 0,
		//	BaseProcessing = 1,
		//	GLProcessing = 2,
		//	Finished = 4,
		//};
		//State state=None; // finished, phase1, phase2
	};
	class AssetProcessor;
	// An instance of this class exists in each instance of Window.
	// Allow auto refresh
	class AssetStorage {
	public:
		AssetStorage();
		~AssetStorage();

		//-- Flags for methods
		static const uint8_t SYNC = 1; // Perform function synchronously. Applies to AssetStorage::load and set
		//static const uint8_t FORCE_RELOAD = 2;// Load were supposed to always load asset even if it had been loaded. BUT, it will now reload automatically.
		static const uint8_t DEFAULT_FLAGS = 0;
		//static const uint8_t DEFAULT_FLAGS = SYNC;

		// load asset from path
		template<class T>
		T* load(const std::string& path, uint8_t flags = DEFAULT_FLAGS) {
			return (T*)load(T::TYPE, path, flags);
		}
		// deletes an asset which was loaded from path
		// note that the pointer of the asset is invalid memory after it is unloaded
		template<typename T>
		inline void unload(const std::string& path, uint8_t flags = DEFAULT_FLAGS) {
			unload(T::TYPE, path, flags);
		}

		// returns nullptr if not found. str could be path or name
		template<typename T>
		inline T* get(const std::string& str, uint8_t flags = DEFAULT_FLAGS) {
			return (T*)get(T::TYPE, str, flags);
		}

		// returns nullptr if name already exists.
		// by using this function you relieve your responsibility of deleting asset. It is now done by storage.
		template<typename T>
		inline T* set(const std::string& name, T* asset, uint8_t flags = DEFAULT_FLAGS) {
			return (T*)set(T::TYPE, name, asset, flags);
		}

		void addIOProcessor();
		void addDataProcessor();
		void addGraphicProcessor();

		void cleanup();

		void printTasks();
		
		void setRoot(const std::string& path){ Assert(path.length() == 0 || path.back() == '/'); m_storagePath=path; }

		std::vector<AssetProcessor*>& getIOProcessors() { return m_ioProcessors; }
		std::vector<AssetProcessor*>& getDataProcessors() { return m_dataProcessors; }
		std::vector<AssetProcessor*>& getGraphicProcessors() { return m_graphicProcessors; }

	private:
		bool m_running = false;

		// will queue task into appropriate processor
		// path needs to be set for the asset before hand.
		void processTask(AssetTask task);

		Asset* load(AssetType type, const std::string& path, uint8_t flags = DEFAULT_FLAGS);
		void unload(AssetType type, const std::string& path, uint8_t flags = DEFAULT_FLAGS);
		Asset* get(AssetType type, const std::string& str, uint8_t flags = DEFAULT_FLAGS);
		Asset* set(AssetType type, const std::string& name, Asset* asset, uint8_t flags = DEFAULT_FLAGS);
		
		//-- distrubtion when queueing tasks
		int m_ioNext=0;
		int m_dataNext=0;
		int m_graphicNext=0;

		//-- Processors
		std::vector<AssetProcessor*> m_ioProcessors;
		std::vector<AssetProcessor*> m_dataProcessors;
		std::vector<AssetProcessor*> m_graphicProcessors; // OpenGL is single threaded. Can only have one processor.
		std::mutex m_ioProcMutex;
		std::mutex m_dataProcMutex;
		std::mutex m_graphicProcMutex;

		//-- Assets in storage
		std::unordered_map<AssetType, std::unordered_map<std::string,Asset*>> m_assets;
		std::mutex m_assetMutex;
		
		// loading assets and then changing this will not allow
		// you to access those assets again since the path is different
		// std::string m_storagePath="assets/";
		std::string m_storagePath="assets/";

		template<typename T>
		inline std::string modifyPath(const std::string& str) {
			return modifyPath(T::TYPE, str);
		}
		std::string modifyPath(AssetType type, const std::string& str);

		Asset* allocateAsset(AssetType type);

		friend class AssetLoader;
		friend class AssetProcessor;
	};
	// call start to run the processor on a thread.
	// you can call process to update tasks incase you don't want it on a thread. (processing OpenGL stuff for example)
	class AssetProcessor {
	public:
		AssetProcessor(AssetStorage* storage, Asset::LoadFlag type);
		~AssetProcessor();
		void cleanup();

		// creates a thread which runs process in a while loop with a bit of sleep.
		void start();
		void stop();

		// works through the queue and clears it. Finished tasks are given to AssetStorage for more processing.
		void process();

		void queue(AssetTask task);

		int numQueuedTasks();
		
	private:
		AssetStorage* m_storage = nullptr;
		bool m_running=false;

		std::thread m_thread;

		Asset::LoadFlag m_processType;
		std::vector<AssetTask> m_queue; // tasks to do
		std::mutex m_queueMutex;

		std::mutex m_waitMutex;
		std::condition_variable m_wait;
	};
}