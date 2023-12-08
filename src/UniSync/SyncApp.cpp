
#include "UniSync/SyncApp.h"

// #include "Engone/EventModule.h"
// #include "Engone/Engone.h"

#include "UniSync/resource.h"
#include "Engone/Util/ImageUtility.h"

#define SYNCAPP_DEBUG(x) x
//#define SYNCAPP_DEBUG()

namespace unisync {
    
    int RenderThread(SyncApp* app);
    void StartApp(const AppOptions& options) {
        SyncApp* app = new SyncApp();
        app->appOptions = options;
        
        RenderThread(app);
        
        delete app;
    }
    int RenderThread(SyncApp* app) {
        using namespace engone;
    
        Assert(glfwInit());
    
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    
        GLFWwindow* window = glfwCreateWindow(800, 600, "Convenient syncrhonization", NULL, NULL);
        if (!window) {
            glfwTerminate();
            return 0;
        }
        
        // TODO: Fix memory leak
        // ICO* ico = ICO::Load(IDI_ICON1); // this fails, not sure why
        // if(ico){
        //     RawImage* raw = ICOToRaw(ico);
        //     GLFWimage tmp = {raw->width,raw->height,(uint8_t*)raw->data()};
        //     glfwSetWindowIcon(window,1,&tmp);
        // }
    
        glfwMakeContextCurrent(window);
        glewInit();

        // if(gameState->locked_fps) 
            glfwSwapInterval(1); // limit fps to your monitors frequency?
        // else
        //     glfwSwapInterval(0);
            
        app->window = window;

        // SetupRendering(gameState);

        #define HOT_RELOAD_ORIGIN "bin/app.dll"
        #define HOT_RELOAD_IN_USE "bin/hotloaded.dll"
        #define HOT_RELOAD_ORIGIN_PDB "bin/app.pdb"
        #define HOT_RELOAD_IN_USE_PDB "bin/hotloaded.pdb"
        #define HOT_RELOAD_TIME 2

        app->activeUpdateProc = UpdateApp;
        app->activeRenderProc = RenderApp;
        app->inactiveUpdateProc = UpdateApp;
        app->inactiveRenderProc = RenderApp;

        void* prev_hot_reload_dll = nullptr;
        void* hot_reload_dll = nullptr;
        double last_dll_write = 0;
    
        auto lastTime = engone::StartMeasure();

        // glEnable(GL_BLEND);
        // glEnable(GL_CULL_FACE);
        // glEnable(GL_DEPTH_TEST);
        // glCullFace(GL_FRONT);
        
        PNG* consolas_png = PNG::Load(IDB_PNG1);
        // PNG* colorMarker_png = PNG::Load(IDB_PNG2);
        
        RawImage* consolas_raw = PNGToRawImage(consolas_png);
        // RawImage* colorMarker_raw = PNGToRawImage(colorMarker_png);
        
        int consolasId = app->uiModule.readRawFont((u8*)consolas_raw->data(), consolas_raw->width, consolas_raw->height, consolas_raw->channels);
        Assert(consolasId == 0);
        // app->colorMarker_textureId = app->uiModule.readRawTexture((u8*)colorMarker_raw->data(), colorMarker_raw->width, colorMarker_raw->height, colorMarker_raw->channels);
        // app->uiModule.readRawTexture((u8*)colorMarker_raw->data(), colorMarker_raw->width, colorMarker_raw->height, colorMarker_raw->channels);
        app->inputModule.init(app->window);
        app->uiModule.init(nullptr);
        app->uiModule.enableInput(&app->inputModule);
        
        app->init(app->appOptions);
        
        // Loop
        auto gameStartTime = engone::StartMeasure();
        double updateAccumulation = 0;
        double sec_timer = 0;
        double reloadTime = 0;
        while (!glfwWindowShouldClose(window)) {    
            int width, height;
            glfwGetFramebufferSize(window, &width, &height);
            app->winWidth = width;
            app->winHeight = height;
        
            auto now = engone::StartMeasure();
            double frame_deltaTime = DiffMeasure(now - lastTime);
            lastTime = now;
            app->current_frameTime = frame_deltaTime;
            
            app->avg_frameTime.addSample(frame_deltaTime);

            app->game_runtime = DiffMeasure(now - gameStartTime);

            glViewport(0, 0, width, height);
            glClearColor(0.1,0.1,0.1,1);
            // glClearColor(0.7,0.7,0.33,1);
            glClear(GL_COLOR_BUFFER_BIT);

            #ifdef USE_HOT_RELOAD
            reloadTime -= frame_deltaTime;
            if(reloadTime <= 0) {
                reloadTime += HOT_RELOAD_TIME;
                // log::out << "Try reload\n";
                double new_dll_write = 0;
                bool yes = engone::FileLastWriteSeconds(HOT_RELOAD_ORIGIN, &new_dll_write);
                if(yes && new_dll_write > last_dll_write) {
                    log::out << "Reloaded\n";
                    last_dll_write = new_dll_write;
                    if(hot_reload_dll) {
                        Assert(!prev_hot_reload_dll);
                        prev_hot_reload_dll = hot_reload_dll;
                        hot_reload_dll = nullptr;
                    }
                    // TODO: Is 256 enough?
                    char dll_path[256]{0};
                    char pdb_path[256]{0};
                    snprintf(dll_path,sizeof(dll_path),"bin/hotloaded-%d.dll", (int)rand());
                    snprintf(pdb_path,sizeof(pdb_path),"bin/hotloaded-%d.pdb", (int)rand());
                    engone::FileCopy(HOT_RELOAD_ORIGIN, dll_path);
                    engone::FileCopy(HOT_RELOAD_ORIGIN_PDB, pdb_path);
                    // engone::FileCopy(HOT_RELOAD_ORIGIN, HOT_RELOAD_IN_USE);
                    // engone::FileCopy(HOT_RELOAD_ORIGIN_PDB, HOT_RELOAD_IN_USE_PDB);
                    hot_reload_dll = engone::LoadDynamicLibrary(dll_path);
                    // hot_reload_dll = engone::LoadDynamicLibrary(HOT_RELOAD_IN_USE);

                    gameState->inactiveUpdateProc = (GameProcedure)engone::GetFunctionPointer(hot_reload_dll, "UpdateGame");
                    gameState->inactiveRenderProc = (GameProcedure)engone::GetFunctionPointer(hot_reload_dll, "RenderGame");
                    gameState->activeRenderProc = gameState->inactiveRenderProc;
                }
            }
            // TODO: Mutex on game proc
            if(gameState->activeUpdateProc == gameState->inactiveUpdateProc && prev_hot_reload_dll) {
                engone::UnloadDynamicLibrary(prev_hot_reload_dll);
                prev_hot_reload_dll = nullptr;
            }
            #endif
            if(app->activeUpdateProc != app->inactiveUpdateProc) {
                app->activeUpdateProc = app->inactiveUpdateProc;
            }

            // for(auto& proc : app->assetStorage.getIOProcessors()) {
            //     proc->process();
            // }
            // for(auto& proc : app->assetStorage.getDataProcessors()) {
            //     proc->process();
            // }

            updateAccumulation += frame_deltaTime;
            if(app->activeUpdateProc){
                while(updateAccumulation>app->update_deltaTime){
                    updateAccumulation-=app->update_deltaTime;
                    app->activeUpdateProc(app);
                }
            }

            // for(auto& proc : app->assetStorage.getGraphicProcessors()) {
            //     proc->process();
            // }
            if(app->activeRenderProc) {
                app->activeRenderProc(app);
            }

            // glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            app->uiModule.render(app->winWidth, app->winHeight);

            app->inputModule.resetEvents(true);
            app->inputModule.resetPollChar();
            app->inputModule.m_lastMouseX = app->inputModule.m_mouseX;
            app->inputModule.m_lastMouseY = app->inputModule.m_mouseY;
    
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
        app->isRunning = false;
        
        // TODO: Move into it's own function?
        app->userCache.save();
		if (app->client) {
			app->client->stop();
		}
		if (app->server) {
			app->server->stop();
		}
		app->window = nullptr;
    
        glfwDestroyWindow(window);
        glfwTerminate();
        
        return 0;
    }
    
	static const engone::UIColor linkedColor = { 0.2,0.95,0.1 };
	static const engone::UIColor failedColor = { 0.95,0.2,0.2 };
	static const engone::UIColor connectingColor = { 0.96,0.78,0.3 };
	static const engone::UIColor collisionColor = { 0.96,0.78,0.3 };
	static const engone::UIColor frozenColor = { 0.1,0.7,0.8 };
	static const engone::UIColor newColor = { 0.3,0.88,0.3 };
	static const engone::UIColor overrideColor = { 0.2,0.1,0.9 };

	static const engone::UIColor defaultColor = { 0.75,0.85,0.75 };
	static const engone::UIColor selectedColor = { 0.3,0.94,0.4 };
	static const engone::UIColor editColor = { 1,0.4,0.1 };

	// for convenience
	static engone::Logger& operator<<(engone::Logger& logger, engone::Sender* sender) {
		if (sender->isServer()) logger << "Server: ";
		else logger << "Client: ";
		return logger;
	}
    void SyncApp::cleanup(){
		delete server;
		server = nullptr;
		delete client;
		client = nullptr;
        for(auto& pair : fileDownloads) {
            delete pair.second.writer;
        }
        
        if(window) {
            glfwDestroyWindow(window);
            glfwTerminate();
	    	window = nullptr;
        }
        // engone::log::out << engone::log::RED << "SyncApp CLEANUP INCOMPLETE\n";
    }
	void SyncApp::init(const AppOptions& appOptions) {
		using namespace engone;
		
		options = options | AutoRefresh;

		// CreateDefaultKeybindings();

		// m_window = createWindow({ModeWindowed,600,500});
		// m_window->setTitle("UniSync");
		// consolas = getStorage()->set<FontAsset>("consolas", new FontAsset(IDB_PNG1,"4\n35"));
        Assert(!appOptions.cachePath.empty());
		userCache.load(appOptions.cachePath);
		if (options & AutoRefresh) {
			for (SyncUnit* unit : userCache.getUnits()) {
				unit->startMonitor();
				//unit->autoRefresh();
			}
		}
		// Importent to refresh before connecting. Connection could otherwise overwrite files because of old cache.
		refreshUnits();
		// FEATURE: flag which auto reconnects
		reconnect();

		// if (!consolas)
		// 	log::out << log::RED << "Consolas is nullptr\n";
	}
    void UpdateApp(SyncApp* app) {
        using namespace engone;
		if (app->upCooldown > 0) app->upCooldown -= app->update_deltaTime;
		if (app->downCooldown > 0) app->downCooldown -= app->update_deltaTime;
		if ((app->inputModule.isKeyDown(KeyUp) && app->upCooldown <= 0) || app->inputModule.isKeyPressed(KeyUp)) {
			app->upCooldown = 0.2;
			app->editingText = false;
			app->selectedLine--;
		}
		if ((app->inputModule.isKeyDown(KeyDown) && app->downCooldown <= 0) || app->inputModule.isKeyPressed(KeyDown)) {
			app->downCooldown = 0.2;
			app->editingText = false;
			app->selectedLine++;
		}

		if (app->options & SyncApp::AutoRefresh) {
			for (int i = 0; i < app->userCache.getUnits().size(); i++) {
				if (app->userCache.getUnits()[i]->m_wasRefreshed) {
					app->m_shouldSynchronize = true;
					app->userCache.getUnits()[i]->m_wasRefreshed = false;
				}
			}
		}
		if (app->m_shouldSynchronize) {
			app->synchronizeUnits();
			app->m_shouldSynchronize = false;
		}
    }
	void SyncApp::renderText(const std::string& text) {
		// if (!consolas) return;
        auto ui = &uiModule;
        
        auto uitext = ui->makeText();
        ui->setString(uitext, text.c_str());
        uitext->x = indentSize * indent;
        uitext->y = lineHeight * (line + scrollY); 
        uitext->h = lineHeight;
        uitext->color = selectedLine == line ? selectedColor : defaultColor;
        
        if (editingText && selectedLine == line) {
			uitext->cursorIndex = editingIndex;
			uitext->editing = true;
			uitext->color = editColor;
		}
        
		// engone::ui::TextBox root = { text, indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas, selectedLine == line ? selectedColor : defaultColor };
		// if (editingText && selectedLine == line) {
		// 	root.at = editingIndex;
		// 	root.editing = true;
		// 	root.rgba = editColor;
		// }
		// engone::ui::Draw(root);
		line++;
	}
	void SyncApp::renderText(const std::string& first, const std::string& second) {
		// if (!consolas) return;
        auto ui = &uiModule;
        
        auto uitext = ui->makeText();
        std::string comb = first + second; // TODO: Optimize
        ui->setString(uitext, comb.c_str());
        uitext->x = indentSize * indent;
        uitext->y = lineHeight * (line + scrollY); 
        uitext->h = lineHeight;
        uitext->color = selectedLine == line ? selectedColor : defaultColor;
        
        if (editingText && selectedLine == line) {
			uitext->cursorIndex = first.length() + editingIndex;
			uitext->editing = true;
			uitext->color = editColor;
		}
        
		// engone::ui::TextBox root = { first + second, indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas, selectedLine == line ? selectedColor : defaultColor };
		// if (editingText && selectedLine == line) {
		// 	root.at = first.length() + editingIndex;
		// 	root.editing = true;
		// 	root.rgba = editColor;
		// }
		// engone::ui::Draw(root);
		line++;
	}
	void SyncApp::openAction(SyncPart* part) {
		if (editingText)
			return;
		if (inputModule.isKeyPressed(KeyOpen) && selectedLine == line) {
            part->set(STATUS_OPEN);
			// part->m_status = part->m_status | STATUS_OPEN;
		}
		if (inputModule.isKeyPressed(KeyClose) && selectedLine == line) {
            part->set(STATUS_OPEN, false);
			// part->m_status = part->m_status & (~STATUS_OPEN);
		}
	}
	// check if path contains .. if it does, the program could alter files above the root directory. That would be a security issue.
	// This only need to be done before writing file
	bool IsPathSafe(const std::string& path) {
		int n = path.find("..");
		return n == path.npos;
	}
	void SyncApp::editAction(std::string& str) {
		if (selectedLine == line) {
			if (inputModule.isKeyPressed(KeyEdit)) {
				//part->m_status = part->m_status | Status::Open;
				if (editingText) {
					editingText = false;
				} else {
					editingText = true;
					editingIndex = str.length();
				}
				return;
			}
			if (editingText) {
                uiModule.edit(str, editingIndex, editingText, false);
				// engone::ui::Edit(str, editingIndex,editingText,false);
			}
		}
	}
	void SyncApp::renderPart(SyncPart* part) {
		using namespace engone;

		renderText(part->m_name);
		renderState(part->m_name, part->m_status);
	}
	void SyncApp::renderState(const std::string& text, Status state) {
		using namespace engone;
		// if (!consolas) return;
		line--;
        auto ui = &uiModule;
        auto layout = ui->makeLayout();
        layout.flow = UILayout::FLOW_RIGHT;
        layout.textSize = lineHeight;
        layout.x = indentSize * indent + ui->getWidthOfText(nullptr, lineHeight, text.length());
        layout.y = lineHeight * (line + scrollY);
        // TODO: Use UILayout
		// int offset =  ui->getWidthOfText(nullptr, lineHeight, text.length());
		if (state & STATUS_CON_LINKED) {
            auto textBox = layout.makeText(" [Linked]");
            textBox->color = linkedColor;
		}
		if (state & STATUS_CON_FAILED) {
            auto textBox = layout.makeText(" [Failed]");
            textBox->color = failedColor;
		}
		if (state & STATUS_CON_CONNECTING) {
            auto textBox = layout.makeText(" [Connecting]");
            textBox->color = connectingColor;
		}
		if (state & STATUS_CON_RECONNECT) {
            auto textBox = layout.makeText(" [Reconnect]");
            textBox->color = connectingColor;
		}
		//log::out << (state & STATUS_UNIT_FROZEN) << " frozen?\n";
		if ((state & STATUS_UNIT_FROZEN)||(state&STATUS_FILE_FROZEN)) {
            auto textBox = layout.makeText(" [Frozen]");
            textBox->color = frozenColor;
		}
		if (state & STATUS_FILE_COLLISION) {
            auto textBox = layout.makeText(" [Collision]");
            textBox->color = collisionColor;
		}
		if (state & STATUS_FILE_NEW) {
            auto textBox = layout.makeText(" [New]");
            textBox->color = newColor;
		}
		if (state & STATUS_FILE_OVERRIDE) {
            auto textBox = layout.makeText(" [Override]");
            textBox->color = overrideColor;
		}
		line++;
	}

	void SyncApp::recSyncUnits(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid) {
		using namespace engone;

		// should files relocate/update if someone send this message? what if they spam?
		if((options&AutoRefresh)==0)
			refreshUnits();

		u32 unitCount;
		buf.pull(&unitCount);

		for (int i = 0; i < unitCount; i++) {
			std::string unitName;
			buf.pull(unitName,256);
			std::string password;
			buf.pull(password, 256);
			Status state;
			buf.pull(&state);
			u64 timestamp;
			buf.pull(&timestamp);

			SyncUnit* unit = userCache.getUnit(unitName);
			if (!unit) {
				// do nothing, unit does not exist
			} else{
				unit->lock();

				// check password
				if (password != unit->m_password) {
					// password is wrong
					log::out << log::YELLOW << "Received wrong password\n";
				}else{
					MessageBuffer send;
					send.push(NET_SYNC_FILES);
					send.push(unitName);
					send.push(password);
					u32 fileSize = 0;
					for (int k = 0; k < unit->m_files.size(); k++) {
						SyncFile& file = unit->m_files[k];
						//if (file.m_status & STATUS_FILE_FROZEN)
						//	continue;
						fileSize++;
					}
					send.push(fileSize);
					//log::out << "NetSync " << unit->m_files.size() << "\n";
					for (int k = 0; k < unit->m_files.size(); k++) {
						SyncFile& file = unit->m_files[k];
						//if (file.m_status & STATUS_FILE_FROZEN)
						//	continue;
						send.push(file.m_name);
						send.push(file.m_status);
						send.push(file.m_timestamp);
						//log::out << "SyncFiles send " <<file.m_name << " "<< file.m_status << "\n";
					}
					//log::out << "recSyncUnits - send\n";
					sender->send(send, uuid,false);
				}
				unit->unlock();
			} 
		}
	}
	void SyncApp::recSyncFiles(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid) {
		using namespace engone;
		std::string unitName;
		buf.pull(unitName, 256);
		std::string password;
		buf.pull(password, 256);

		SyncUnit* unit = userCache.getUnit(unitName);
		//log::out << sender << " Sync files\n";
		if (!unit) {
			return;
		}
        unit->refresh(); // ensure that the files we have receive the NEW status (if they should have it)
        // TODO: Files may be modified in between here. That's no good.
		unit->lock();
		if (password != unit->m_password) {
			log::out << log::YELLOW << "Received wrong password\n";
			unit->unlock();
			return;
		}
		std::vector<std::string> fileRequests; // files to request, old version or not have
		std::vector<std::string> fileNotifications; // collision notifications to send
		std::vector<bool> sendFiles(unit->m_files.size(), true); // files to send, newest version, or you dont have

		u32 fileCount;
		buf.pull(&fileCount);
		//log::out << "fileCount "<<fileCount<<"\n";
		for (int i = 0; i < fileCount; i++) {
			std::string fileName;
			buf.pull(fileName, 256);
			Status state;
			buf.pull(&state);
			u64 time;
			buf.pull(&time);
					
			//if (state & STATUS_FILE_FROZEN)
			//	continue;

			bool found = false;
			for (int j = 0; j < unit->m_files.size(); j++) {
				SyncFile& file = unit->m_files[j];
				//log::out << fileName << " = " << file.m_name << "?\n";
				if (fileName == file.m_name) {
					sendFiles[j] = false;
					found = true;

					if (unit->m_status & STATUS_UNIT_FROZEN) {
						if (!(unit->m_status & STATUS_FILE_MELTED))
							break;
					} else if (file.m_status & STATUS_FILE_FROZEN)
						break;

					//log::out << file.m_status << " - "<<state<<"\n";
					if (file.check(STATUS_FILE_NEW)) {
						if (state & STATUS_FILE_NEW) {
							//log::out << "both new\n";
							if (file.check(STATUS_FILE_OVERRIDE)) { // I have priority of overriding
								sendFiles[j] = true;
								//log::out << "override1\n";
							} else if (state & STATUS_FILE_OVERRIDE) {
								fileRequests.push_back(fileName);
								//log::out << "override2\n";
							} else {
                                log::out << "collision "<<fileName<<"\n";
								file.set(STATUS_FILE_COLLISION);
								if (!(state & STATUS_FILE_COLLISION))
									fileNotifications.push_back(fileName);
							}
						} else {
							sendFiles[j] = true;
						}
					} else {
						if (state & STATUS_FILE_NEW) {
							fileRequests.push_back(fileName);
						} else {
							// neither is new do nothing
						}
					}
					break;
				}
			}
			if (!found) {
				// received non-existent file
				fileRequests.push_back(fileName);
			}
		}
		if (fileNotifications.size() > 0) {
			MessageBuffer notfifs;
			notfifs.push(NET_NOTIFY_FILES);
			notfifs.push(unit->m_name);
			notfifs.push(unit->m_password);
			notfifs.push((u32)fileNotifications.size());
			for (int i = 0; i < fileNotifications.size(); i++) {
				//log::out << sender << "Notify " << fileNotifications[i] << "\n";
				notfifs.push(fileNotifications[i]);
			}
			sender->send(notfifs, uuid,false);
		}
		if (fileRequests.size() > 0) {
			MessageBuffer req;
			req.push(NET_REQUEST_FILE);
			req.push(unit->m_name);
			req.push(unit->m_password);
			req.push((u32)fileRequests.size());
			for (int i = 0; i < fileRequests.size(); i++) {
				log::out << sender << "Req " << fileRequests[i] << "\n";
				req.push(fileRequests[i]);
			}
			sender->send(req, uuid,false);
		}
		for (int i = 0; i < unit->m_files.size(); i++) {
			if (!sendFiles[i]) continue;

			SyncFile& file = unit->m_files[i];

			//log::out << "Send " << file.m_name << "\n";

			sendFile(unit, file, sender, uuid);
		}
		unit->unlock();
	}
	void SyncApp::recNotifyFiles(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid) {
		using namespace engone;
		std::string unitName;
		buf.pull(unitName, 256);
		std::string password;
		buf.pull(password, 256);

		SyncUnit* unit = userCache.getUnit(unitName);

		if (unit) {
			u32 fileCount;
			buf.pull(&fileCount);
			unit->lock();

			if (password != unit->m_password) {
				log::out << log::YELLOW << "Received wrong password\n";
			} else {
				for (int i = 0; i < fileCount; i++) {
					std::string path;
					buf.pull(path);
					for (int j = 0; j < unit->m_files.size(); j++) {
						SyncFile& file = unit->m_files[j];
						if (unit->m_status & STATUS_UNIT_FROZEN) {
							if (!(unit->m_status & STATUS_FILE_MELTED))
								break;
						} else if (file.m_status & STATUS_FILE_FROZEN)
							break;
						if (file.m_name == path) {
							file.set(STATUS_FILE_COLLISION, true);
						}
					}
				}
			}
			unit->unlock();
		}
	}
	void SyncApp::recReqFiles(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid) {
		using namespace engone;
		std::string unitName;
		buf.pull(unitName, 256);
		std::string password;
		buf.pull(password, 256);

		SyncUnit* unit = userCache.getUnit(unitName);

		if (unit) {
			unit->lock();
			if (password != unit->m_password) {
				log::out << log::YELLOW << "Received wrong password\n";
			} else {
				u32 fileCount;
				buf.pull(&fileCount);

				for (int i = 0; i < fileCount; i++) {
					std::string path;
					buf.pull(path);
					for (int j = 0; j < unit->m_files.size(); j++) {
						SyncFile& file = unit->m_files[j];
						if (file.m_name == path) {
							sendFile(unit, file, sender, uuid);
						}
					}
				}
			}
			unit->unlock();
		}
	}
	enum StreamState : u8 {
		StreamNormal,
		StreamStart=1,
		StreamEnd=2,
	};
	typedef u8 StreamStates;
	void SyncApp::sendFile(SyncUnit* unit, SyncFile& file, engone::Sender* sender, engone::UUID uuid) {
		using namespace engone;

		// unit should already be locked

		if (unit->m_status & STATUS_UNIT_FROZEN) {
			if (!(unit->m_status & STATUS_FILE_MELTED))
				return;
		} else if (file.m_status & STATUS_FILE_FROZEN)
			return;

		// ISSUE: Here I assume the file will be received. Because if it isn't, the file on my side will not be new, and the file on the other side is still new.
		//   The file on the other side will then be assumed to be the newest which is not the case.
		file.set(STATUS_FILE_NEW, false);
		file.set(STATUS_FILE_OVERRIDE, false);
		file.set(STATUS_FILE_COLLISION, false);
		file.m_timestamp = unit->getTime(unit->m_root + file.m_name);
		//file.m_lastModified = unit->getTime(unit->m_root + file.m_name);

		if (file.isDir) {
			MessageBuffer send;
			send.push(NET_SEND_FILE);
			send.push(&file.isDir);
			send.push(unit->m_name);
			send.push(unit->m_password);
			send.push(file.m_name);
			sender->send(send, uuid,false);
			return;
		}

		FileReader reader(unit->m_root + file.m_name, true);
		if (!reader.isOpen()) {
			log::out << log::RED << "SyncApp::sendFile - Failed reading " << reader.getPath() << "\n";
			return;
		}
		u32 bytesLeft = reader.size();
		bool fileEmpty = bytesLeft == 0;


		UUID fileUuid = UUID::New();

		//log::out << sender << "Send " << file.m_name << "\n";

		// THIS NEEDS TO CHANGE IF YOU CHANGE THE STRUCTURE OF THE MESSAGE
		u32 maxHeaderSize = sizeof(SyncNet) + sizeof(file.isDir) + sizeof(fileUuid) +
			sizeof(StreamStates) + sizeof(u32)+8; // 8 for some extra bytes, why not

		maxHeaderSize += unit->m_name.length() + 5; // +5 because 4 is integer for length of string, 1 is for null char.
		maxHeaderSize += unit->m_password.length() + 5;
		maxHeaderSize += file.m_name.length() + 5;

		while (bytesLeft != 0 || fileEmpty) {
			fileEmpty = false;
			MessageBuffer send;
			send.push(NET_SEND_FILE);
			send.push(&file.isDir); // always false
			send.push(fileUuid);

			StreamStates streamState = 0;
			if (bytesLeft == reader.size()) streamState = streamState | StreamStart;
			if (bytesLeft <= sender->getTransferLimit() - maxHeaderSize) streamState = streamState | StreamEnd;

			send.push(&streamState);
			if (streamState & StreamStart) {
				send.push(unit->m_name);
				send.push(unit->m_password);
				send.push(file.m_name);
			}

			u32 transferSize = sender->getTransferLimit() - maxHeaderSize;
            if(bytesLeft < transferSize)
                transferSize = bytesLeft;
			//log::out << "Transfer "<<transferSize << "\n";
			char* data = send.pushBuffer(transferSize);

			reader.read(data, transferSize);
			//log::out << "Buf size " << send.size() << "\n";
			sender->send(send,uuid,false);
			bytesLeft -= transferSize;
		}
	}
	void SyncApp::recSendFile(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid) {
		using namespace engone;

		bool isDir;
		buf.pull(&isDir);
		if (isDir) {
			std::string unitName;
			buf.pull(unitName, 256);
			std::string password;
			buf.pull(password, 256);
			std::string path;
			buf.pull(path, 256);

			SyncUnit* unit = userCache.getUnit(unitName);
			if (!unit){
				log::out << sender << "Unit doesn't exist? " << unitName << "\n";
				return;
			}
			unit->lock();

			if (password != unit->m_password) {
				log::out << log::YELLOW << "Received wrong password\n";
				unit->unlock();
				return;
			}
			std::string fullPath = unit->m_root + path;

			if (!IsPathSafe(path)) {
				log::out << sender << "Dangerous path! " << path << "\n";
				unit->unlock();
				return;
			}
			bool b = std::filesystem::create_directories(fullPath);
			//if (!b) log::out << log::RED<<"Could not create directories\n";

			SyncFile* file = unit->getFile(path, isDir);

			u64 time = unit->getTime(fullPath);
			if (file) {
				bool yes = !file->check(STATUS_FILE_FROZEN);
				if (unit->check(STATUS_UNIT_FROZEN))
					yes = file->check(STATUS_FILE_MELTED);

				if(yes){
					file->m_timestamp = time;
					file->set(STATUS_FILE_NEW, false);
					file->set(STATUS_FILE_OVERRIDE, false);
					file->set(STATUS_FILE_COLLISION, false);
				}
			} else {
				unit->insertFile({ path,isDir,time });
			}
			unit->unlock();
		} else {
			UUID fileUuid;
			buf.pull(&fileUuid);
			StreamStates streamState;
			buf.pull(&streamState);

			FileDownload* download = nullptr;

			if (streamState & StreamStart) {
				std::string unitName;
				buf.pull(unitName, 256);
				std::string password;
				buf.pull(password, 256);
				std::string path;
				buf.pull(path, 256);

				SyncUnit* unit = userCache.getUnit(unitName);

				if (!unit) {
					return;
				}

				unit->lock();

				if (password != unit->m_password) {
					log::out <<log::YELLOW<< "Received wrong password\n";
					unit->unlock();
					return;
				}
				std::string fullPath = unit->m_root + path;

				if (!IsPathSafe(fullPath)) {
					log::out << log::RED<<sender << "Dangerous path! " << fullPath << "\n";
					unit->unlock();
					return;
				}
				//unit->preventRefresh(true);
				FileWriter* writer = new FileWriter(fullPath, true);
				if (writer->isOpen()) {
					// engone::GetTracker().track(writer);
					fileDownloads[fileUuid] = { unit,path,writer };
					download = &fileDownloads[fileUuid]; // the memory shouldn't change until this function is done, meaning download will not be invalidated by unordered_map
				} else {
					log::out << "SyncApp::recSendFile (" << sender << ") " << writer->getError() << " " << fullPath << "\n";
					delete writer; // no need to track file writer
					writer = nullptr;
					//unit->preventRefresh(false);
					//fileDownloads[fileUuid] = { unit,path,nullptr };
				}
				unit->unlock();
			} else {
				auto find = fileDownloads.find(fileUuid);
				if (find != fileDownloads.end()) {
					download = &find->second;
				} else {
					log::out << log::RED <<sender<< "UUID does not have a FileWriter\n";
				}
			}

			if (!download) {
				//log::out << log::RED << "FileWriter for UUID not found\n";
				return;
			}
			u32 transferSize;
			char* data = buf.pullBuffer(&transferSize);
			if (download->writer) {
				try {
					download->writer->write(data, transferSize);
				} catch (Error err) {
					log::out << sender << " Error " << err << " writing " << download->fileName << "\n";
					// could delete writer here, but it is done below. If streamState isn't ending this time, it should next message.
				}
			}
			//log::out << "TransferSize "<<transferSize << "\n";
			if (streamState & StreamEnd) {
				if (download->writer) {
					download->unit->lock();
					download->writer->close();
					//download->unit->preventRefresh(false);

					u64 time = download->unit->getTime(download->writer->getPath());

					SyncFile* file = download->unit->getFile(download->fileName, isDir);
					if (file) {
						bool yes = !file->check(STATUS_FILE_FROZEN);
						if (download->unit->check(STATUS_UNIT_FROZEN))
							yes = file->check(STATUS_FILE_MELTED);

						if (yes) {
							file->set(STATUS_FILE_NEW, false);
							file->set(STATUS_FILE_OVERRIDE, false);
							file->set(STATUS_FILE_COLLISION, false);
						}
					} else {
						download->unit->insertFile({ download->fileName,isDir,time });
					}

					download->unit->unlock();
					delete download->writer;
					download->writer = nullptr;
					// engone::GetTracker().untrack(download->writer);
				}
				fileDownloads.erase(fileUuid);
				download = nullptr;
				// ISSUE: fileDownloads may get filled up. You may want to track how many remain now and then because there shouldn't be any left after the synchronization is done
				//	  Even if something fails.
			}
		}
	}
	void SyncApp::linkConnection(SyncConnection* conn) {
		using namespace engone;
		// Default port
		if (conn->m_port.length() == 0) {
			conn->m_port = DEFAULT_PORT;
		}

		if (conn->m_status & STATUS_CON_IS_SERVER) {
			if (!serverConn) {
				if (!server) {
					server = new Server();
					// GetTracker().track(server);
					server->setOnEvent([this](NetEvent type, UUID uuid) {
						//log::out << "serv event " << uuid <<" "<< type << "\n";
						if (serverConn) {
							serverConn->set(STATUS_CON_CONNECTING, false);
							if (type == NetEvent::Connect) {
								serverConn->set(STATUS_CON_LINKED, true);
								refreshUnits();
								synchronizeUnits();
							} else if (type == NetEvent::Disconnect) {
								// A client disconnected
							} else if (type == NetEvent::Stopped) {
								serverConn->set(STATUS_CON_LINKED, false);
								serverConn = nullptr;
								if (waitingServer) {
									linkConnection(waitingServer);
									waitingServer = nullptr;
								}
							}
						}
						return true;
					});
					server->setOnReceive([this](MessageBuffer buf, UUID uuid) {
						SyncNet type;
						buf.pull(&type);
						//log::out << "Server received " << type << "\n";
						if (type == NET_SYNC_UNITS) {
							recSyncUnits(buf, server,uuid);
						} else if (type == NET_SYNC_FILES) {
							recSyncFiles(buf, server, uuid);
						} else if (type == NET_REQUEST_FILE) {
							recReqFiles(buf, server, uuid);
						} else if (type == NET_SEND_FILE) {
							recSendFile(buf, server, uuid);
						} else if (type == NET_NOTIFY_FILES) {
							recNotifyFiles(buf,server,uuid);
						} else {
							log::out << log::RED << "Server Received Unknown Message\n";
						}
						return true;
					});
				}
				conn->m_ip = "127.0.0.1";

				// Only one connection is allowed.
				if (clientConn) {
					if (client) {
						client->stop();
						clientConn->set(STATUS_CON_CONNECTING, false);
						clientConn->set(STATUS_CON_FAILED, false);
						clientConn->set(STATUS_CON_LINKED, false);
					}
				}

				bool yes = server->start(conn->m_port);
				conn->set(STATUS_CON_RECONNECT, false);
				conn->set(STATUS_CON_FAILED, false);
				if (yes) {
					// check if successful
					serverConn = conn;
					conn->set(STATUS_CON_LINKED,true);
					//conn->set(STATUS_CON_CONNECTING,true);
				} else {
					conn->set(STATUS_CON_FAILED, true);
					log::out << "Port was invalid '" << conn->m_port << "'\n";
				}
			} else if (server) {
				if (serverConn != conn)
					waitingServer = conn;
				server->stop();
			}
		} else {
			if (!clientConn) {
				if (!client) {
					client = new Client();
					// GetTracker().track(client);
					client->setOnEvent([this](NetEvent type, UUID uuid) {
						//log::out << "client event " << type<< "\n";
						if (clientConn) {
							clientConn->set(STATUS_CON_CONNECTING, false);
							if (type == NetEvent::Connect) {
								clientConn->set(STATUS_CON_LINKED,true);
								refreshUnits();
								synchronizeUnits();
							} else if (type == NetEvent::Disconnect) {
								clientConn->set(STATUS_CON_LINKED, false);
								clientConn = nullptr;
								if (waitingClient) {
									linkConnection(waitingClient);
									waitingClient = nullptr;
								}
							} else if (type == NetEvent::Failed) {
								clientConn->set(STATUS_CON_FAILED,true);
								clientConn = nullptr;
								if (waitingClient) {
									linkConnection(waitingClient);
									waitingClient = nullptr;
								}
							}
						}
						return true;
					});
					client->setOnReceive([this](MessageBuffer buf, UUID uuid) {

						SyncNet type;
						buf.pull(&type);
						//log::out << "Client received " <<type<< "\n";
						if (type == NET_SYNC_UNITS) {
							recSyncUnits(buf, client, 0);
						} else if (type == NET_SYNC_FILES) {
							recSyncFiles(buf, client, 0);
						} else if (type == NET_REQUEST_FILE) {
							recReqFiles(buf, client, 0);
						} else if (type == NET_SEND_FILE) {
							recSendFile(buf, client, 0);
						} else if (type == NET_NOTIFY_FILES) {
							recNotifyFiles(buf, server, uuid);
						} else {
							log::out << log::RED << "Client Received Unknown Message\n";
						}
						return true;
					});
				}
				
				// Only one connection is allowed. mostly because you shouldn't be able to connect with yourself.
				// multiple connections can also mess up the synchronization.
				if (serverConn) {
					if (server) {
						server->stop();
						serverConn->set(STATUS_CON_CONNECTING, false);
						serverConn->set(STATUS_CON_LINKED, false);
						serverConn->set(STATUS_CON_FAILED, false);
					}
				}
				
				if (conn->m_ip.empty())
					conn->m_ip = "127.0.0.1";
				bool yes = client->start(conn->m_ip, conn->m_port);
				conn->set(STATUS_CON_RECONNECT, false);
				conn->set(STATUS_CON_FAILED, false);
				if (yes) {
					clientConn = conn;
					conn->set(STATUS_CON_CONNECTING,true);
				} else {
					conn->set(STATUS_CON_FAILED, true);
					log::out << "IP/Port was invalid '" <<conn->m_ip<<":" <<conn->m_port << "'\n";
				}
			} else if (client) {
				if (clientConn != conn)
					waitingClient = conn;
				client->stop();
			}
		}
	}
	bool SyncApp::mainAction() {
		return inputModule.isKeyPressed(KeyMainAction) && line == selectedLine && !editingText;
	}
	void SyncApp::synchronizeUnits() {
		using namespace engone;
		if (client||server) {
			MessageBuffer buf;
			buf.push(NET_SYNC_UNITS);

			//-- find valid and non-frozen units
			u32 unitSize = 0;
			for (int i = 0; i < userCache.getUnits().size(); i++) {
				SyncUnit* unit = userCache.getUnits()[i];
				if (!unit->check(STATUS_UNIT_FROZEN) && unit->validRoot())
					unitSize++;
			}
			//-- send units
			buf.push(unitSize);
			//log::out << "UNITSIZE " << unitSize << "\n";
			for (int i = 0; i < userCache.getUnits().size(); i++) {
				SyncUnit* unit = userCache.getUnits()[i];
				if (unit->check(STATUS_UNIT_FROZEN) || !unit->validRoot()) continue;
				buf.push(unit->m_name);
				buf.push(unit->m_password);
				buf.push(unit->m_status);
				buf.push(unit->m_timestamp);
			}
			if(client)
				client->send(buf);
			if(server)
				server->send(buf,0,true);
		} else {
			engone::log::out << "Synchronization requires a connection\n";
		}
	}
	void SyncApp::reconnect() {
		for (int i = 0; i < userCache.getConnections().size(); i++) {
			SyncConnection* conn = userCache.getConnections()[i];
			if (conn->check(STATUS_CON_RECONNECT)) {
				linkConnection(conn);
			}
		}
	}
	void SyncApp::refreshUnits() {
		for (SyncUnit* unit : userCache.getUnits()) {
			unit->refresh();
		}
	}
	// void SyncApp::onClose(engone::Window* window){
	// 	userCache.save();
	// 	if (client) {
	// 		client->stop();
	// 	}
	// 	if (server) {
	// 		server->stop();
	// 	}
	// 	m_window = nullptr;
	// 	stop();
	// };
	void SyncApp::pendSynchronize() {
		m_shouldSynchronize = true;
	}
	void RenderApp(SyncApp* app) {
		using namespace engone;

		//chickenY += 1;
		//if (chickenY > 500) {
		//	chickenY = 0;
		//}
		//ui::Box box = {200, chickenY, 50, 50, { 1,1,1,1 }};
		//ui::Draw(box);

		int indentDepth = 2;

		// FEATURE: flag which relocates and syncs units when window loses focus.
        int hasFocus = glfwGetWindowAttrib(app->window, GLFW_FOCUSED);
		if (hasFocus && !app->hadFocus) {
			log::out << "SyncApp::render - gained focus\n"; // debug stuff?
			//if (autoFeatures) {
			app->refreshUnits();
			app->synchronizeUnits();
			//}
		}
		app->hadFocus = hasFocus;

		app->selectedLine -= app->inputModule.getScrollY();

		if (app->selectedLine < 0)
			app->selectedLine = 0;
		if (app->selectedLine > app->line - 1)
			app->selectedLine = app->line - 1;

		std::string fileDrop = "";
        //TODO: PollPathDrop();

		app->indent = 0;
		app->line = 0;

		int top = (app->selectedLine + app->scrollY);
		int maxLine = app->winHeight / app->lineHeight;
		if (top < 1) {
			app->scrollY++;
		}
		if (top > maxLine - 3) {
			app->scrollY--;
		}
		app->renderText("Connections");
		for (int i = 0; i < app->userCache.getConnections().size(); i++) {
			SyncConnection* conn = app->userCache.getConnections()[i];
			
			app->indent = indentDepth;
			app->openAction(conn);
			if (app->mainAction()) {
				app->linkConnection(conn);
			}
			app->editAction(conn->m_name);
			if (app->inputModule.isKeyPressed(KeyDelete) && app->line == app->selectedLine && !app->editingText) {
				delete conn;
				// GetTracker().untrack(conn);
				app->userCache.getConnections().erase(app->userCache.getConnections().begin() + i);
				i--;
				continue;
			}
			app->renderPart(conn);

			if (conn->m_status & STATUS_OPEN) {
				app->indent = 2* indentDepth;
				if (app->mainAction()) {
					// prevent changing type during a connection
					if (conn != app->serverConn && conn != app->clientConn && conn != app->waitingServer && conn != app->waitingClient) {
						if (conn->m_status & STATUS_CON_IS_SERVER) {
                            conn->set(STATUS_CON_IS_SERVER, false);
							// conn->m_status = conn->m_status & (~STATUS_CON_IS_SERVER);
						} else {
                            conn->set(STATUS_CON_IS_SERVER);
							// conn->m_status = conn->m_status | (STATUS_CON_IS_SERVER);
						}
					}
				}
				if (conn->m_status & STATUS_CON_IS_SERVER) {
					app->renderText("Type: Server");
				} else {
					app->renderText("Type: Client");
				}
				app->editAction(conn->m_ip);
				app->renderText("IP: ", conn->m_ip);
				app->editAction(conn->m_port);
				app->renderText("Port: ", conn->m_port);
			}
		}
		app->indent =  indentDepth;
		if (app->mainAction()) {
			SyncConnection* conn = new SyncConnection();
			app->userCache.getConnections().push_back(conn);
			// GetTracker().track(conn);
		}
		app->renderText("+");

		app->indent = 0;

		app->renderText("Units");

		for (int i = 0; i < app->userCache.getUnits().size(); i++) {
			SyncUnit* unit = app->userCache.getUnits()[i];
		
			app->indent = indentDepth;
			app->openAction(unit);
			app->editAction(unit->m_name);
			if (app->inputModule.isKeyPressed(KeyDelete) && app->line == app->selectedLine && !app->editingText) {
				delete unit;
				// GetTracker().untrack(unit);
				app->userCache.getUnits().erase(app->userCache.getUnits().begin() + i);
				i--;
				continue;
			}
			if (app->inputModule.isKeyPressed(KeyFreeze) && app->line == app->selectedLine && !app->editingText) {
				unit->set(STATUS_UNIT_FROZEN, !unit->check(STATUS_UNIT_FROZEN));
			}
			app->renderPart(unit);

			if (unit->m_status & STATUS_OPEN) {
				app->indent = 2 * indentDepth;
				app->editAction(unit->m_password);
				app->renderText("Password: ", unit->m_password);

				bool wasEditing = app->editingText;
				unit->lock(); // lock when using root
				app->editAction(unit->m_root);
				if (app->editingText && app->line == app->selectedLine) {
					if (!fileDrop.empty()) {
						unit->m_root = fileDrop;
						app->editingIndex = unit->m_root.length();
					}
				}
				unit->unlock();
				if (wasEditing && !app->editingText) {
					// stopped editing, relocate unit
					unit->refresh();
					unit->startMonitor(); // update monitor if root changed
				}
				unit->lock(); // lock when changing or using files
				app->renderText("Root: ", unit->m_root);

				if (unit->m_files.size() == 0) {
					app->renderText("No Files");
				} else {
					app->renderText("Files " + std::to_string(unit->m_files.size()));

					app->indent = 3 * indentDepth;
					std::string closed = "";
					bool flagOverride = 0;
					u32 minFlagIndent = -1; // -1 ignore
					for (int j = 0; j < unit->m_files.size(); j++) {
						SyncFile* file = &unit->m_files[j];
						
						// ISSUE: if file is below window break

						app->indent = 3 * indentDepth;
						bool subOfClosed = false;
						if (closed.length() < file->m_name.length() && !closed.empty()) {
							subOfClosed = true;
							for (int k = 0; k < closed.length(); k++) {
								if (closed[k] != file->m_name[k]) {
									subOfClosed = false;
									break;
								}
							}
							if (file->m_name[closed.length()]!='\\') {// should not be out of bounds
								subOfClosed = false;
							}
						}
						int lastIndex = 0;
						int _indent = app->indent - indentDepth; // all files have \\ in the beginning so -indentDepth to skip it.
						for (int k = 0; k < file->m_name.length(); k++) {
							if (file->m_name[k] == '\\') {
								_indent+= indentDepth;
								lastIndex = k;
							}
						}
						app->indent = _indent;

						if (app->indent > minFlagIndent) { // note that -1 as unsigned will be larger than any number
							file->set(STATUS_FILE_OVERRIDE, flagOverride);
							app->pendSynchronize();
						} else {
							minFlagIndent = -1;
						}
						if (subOfClosed)
							continue;

						if ((!file->check(STATUS_OPEN) || file->check(STATUS_FILE_DELETED)) && file->isDir) {
							closed = file->m_name;
						}
						if (file->check(STATUS_FILE_DELETED))
							continue;
						if (file->isDir) {
							app->openAction(file);
						}

						std::string path = file->m_name.substr(lastIndex + 1);

						if (app->inputModule.isKeyPressed(KeyMainAction) && app->line == app->selectedLine && !app->editingText) {
							if (file->check(STATUS_FILE_NEW)) {
								bool yes = !file->check(STATUS_FILE_OVERRIDE);
								file->set(STATUS_FILE_OVERRIDE, yes);
								if(yes)
									app->pendSynchronize();
								if (file->isDir) {
									flagOverride = yes;
									minFlagIndent = app->indent;
								}
							}
						}
						if (app->inputModule.isKeyPressed(KeyFreeze) && app->line == app->selectedLine && !app->editingText) {
							file->set(STATUS_FILE_FROZEN, !file->check(STATUS_FILE_FROZEN));
							if (!file->check(STATUS_FILE_FROZEN)&&file->check(STATUS_FILE_OVERRIDE))
								app->pendSynchronize();
						}
						if (app->inputModule.isKeyPressed(KeyMelt) && app->line == app->selectedLine && !app->editingText) {
							file->set(STATUS_FILE_MELTED, !file->check(STATUS_FILE_MELTED));
						}
						if (file->isDir) {
							app->indent--;
							if (file->check(STATUS_OPEN))
								app->renderText("+");
							else
								app->renderText("-");
							app->indent++;
							app->line--;
							app->renderText(path);
							app->renderState(path, file->m_status);
						} else {
							app->renderText(path);
							app->renderState(path, file->m_status);
						}
					}
					app->indent = 3 * indentDepth;
				}
				unit->unlock();
			}
		}
		app->indent = 1 * indentDepth;
		if (app->mainAction()) {
			SyncUnit* unit = new SyncUnit();
			app->userCache.getUnits().push_back(unit);
			// GetTracker().track(unit);
		}
		app->renderText("+");

		app->indent = 0;
	}
}