
#include "UniSync/SyncApp.h"

#include "Engone/EventModule.h"
#include "Engone/Engone.h"

#include "UniSync/../../resource.h"

#define SYNCAPP_DEBUG(x) x
//#define SYNCAPP_DEBUG()

namespace unisync {
	static const engone::ui::Color linkedColor = { 0.2,0.95,0.1 };
	static const engone::ui::Color failedColor = { 0.95,0.2,0.2 };
	static const engone::ui::Color connectingColor = { 0.96,0.78,0.3 };
	static const engone::ui::Color frozenColor = { 0.1,0.7,0.8 };
	static const engone::ui::Color newColor = { 0.3,0.88,0.3 };
	static const engone::ui::Color overrideColor = { 0.2,0.1,0.9 };

	static const engone::ui::Color defaultColor = { 0.75,0.85,0.75 };
	static const engone::ui::Color selectedColor = { 0.3,0.94,0.4 };
	static const engone::ui::Color editColor = { 1,0.4,0.1 };

	// for convenience
	static engone::Logger& operator<<(engone::Logger& logger, engone::Sender* sender) {
		if (sender->isServer()) logger << "Server: ";
		else logger << "Client: ";
		return logger;
	}

	SyncApp::SyncApp(engone::Engone* engone, const std::string& cachePath) : Application(engone) {
		using namespace engone;
		
		options = options | AutoRefresh;

		CreateDefaultKeybindings();

		m_window = createWindow({ModeWindowed,600,500});
		m_window->setTitle("UniSync");
		consolas = getStorage()->set<FontAsset>("consolas", new FontAsset(IDB_PNG1,"4\n35"));

		userCache.load(cachePath);
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

		if (!consolas)
			log::out << log::RED << "Consolas is nullptr\n";
	}
	SyncApp::~SyncApp() {
		cleanup();
	}
	void SyncApp::cleanup() {
		engone::GetTracker().untrack(server);
		engone::GetTracker().untrack(client);
		delete server;
		server = nullptr;
		delete client;
		client = nullptr;
	}
	void SyncApp::renderText(const std::string& text) {
		if (!consolas) return;
		engone::ui::TextBox root = { text, indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas, selectedLine == line ? selectedColor : defaultColor };
		if (editingText && selectedLine == line) {
			root.at = editingIndex;
			root.editing = true;
			root.rgba = editColor;
		}
		engone::ui::Draw(root);
		line++;
	}
	void SyncApp::renderText(const std::string& first, const std::string& second) {
		if (!consolas) return;
		engone::ui::TextBox root = { first + second, indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas, selectedLine == line ? selectedColor : defaultColor };
		if (editingText && selectedLine == line) {
			root.at = first.length() + editingIndex;
			root.editing = true;
			root.rgba = editColor;
		}
		engone::ui::Draw(root);
		line++;
	}
	void SyncApp::openAction(SyncPart* part) {
		if (editingText)
			return;
		if (engone::IsKeybindingPressed(KeyOpen) && selectedLine == line) {
			part->m_status = part->m_status | StateOpen;
		}
		if (engone::IsKeybindingPressed(KeyClose) && selectedLine == line) {
			part->m_status = part->m_status & (~StateOpen);
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
			if (engone::IsKeybindingPressed(KeyEdit)) {
				//part->m_status = part->m_status | SyncState::Open;
				if (editingText) {
					editingText = false;
				} else {
					editingText = true;
					editingIndex = str.length();
				}
				return;
			}
			if (editingText) {
				engone::ui::Edit(str, editingIndex,editingText,false);
			}
		}
	}
	void SyncApp::renderPart(SyncPart* part) {
		using namespace engone;

		renderText(part->m_name);
		renderState(part->m_name, part->m_status);
	}
	void SyncApp::renderState(const std::string& text, SyncStates state) {
		using namespace engone;
		if (!consolas) return;
		line--;
		int offset = consolas->getWidth(text, lineHeight);
		if (state & StateConnLinked) {
			ui::TextBox textBox = { " [Linked]",offset + indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas,linkedColor };
			ui::Draw(textBox);
			offset += consolas->getWidth(textBox.text, lineHeight);
		}
		if (state & StateConnFailed) {
			ui::TextBox textBox = { " [Failed]",offset + indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas,failedColor };
			ui::Draw(textBox);
			offset += consolas->getWidth(textBox.text, lineHeight);
		}
		if (state & StateConnConnecting) {
			ui::TextBox textBox = { " [Connecting]",offset + indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas,connectingColor };
			ui::Draw(textBox);
			offset += consolas->getWidth(textBox.text, lineHeight);
		}
		if (state & StateConnReconnect) {
			ui::TextBox textBox = { " [Reconnect]",offset + indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas,connectingColor };
			ui::Draw(textBox);
			offset += consolas->getWidth(textBox.text, lineHeight);
		}
		//log::out << (state & StateUnitFrozen) << " frozen?\n";
		if ((state & StateUnitFrozen)||(state&StateFileFrozen)) {
			ui::TextBox textBox = { " [Frozen]",offset + indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas,frozenColor };
			ui::Draw(textBox);
			offset += consolas->getWidth(textBox.text, lineHeight);
		}
		if (state & StateFileCollision) {
			ui::TextBox textBox = { " [Collision]",offset + indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas,connectingColor };
			ui::Draw(textBox);
			offset += consolas->getWidth(textBox.text, lineHeight);
		}
		if (state & StateFileNew) {
			ui::TextBox textBox = { " [New]",offset + indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas,newColor };
			ui::Draw(textBox);
			offset += consolas->getWidth(textBox.text, lineHeight);
		}
		if (state & StateFileOverride) {
			ui::TextBox textBox = { " [Override]",offset + indentSize * indent,lineHeight * (line + scrollY),lineHeight,consolas,overrideColor };
			ui::Draw(textBox);
			offset += consolas->getWidth(textBox.text, lineHeight);
		}
		line++;
	}

	void SyncApp::recSyncUnits(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid) {
		using namespace engone;

		// should files relocate/update if someone send this message? what if they spam?
		if((options&AutoRefresh)==0)
			refreshUnits();

		uint32_t unitCount;
		buf.pull(&unitCount);

		for (int i = 0; i < unitCount; i++) {
			std::string unitName;
			buf.pull(unitName,256);
			std::string password;
			buf.pull(password, 256);
			SyncState state;
			buf.pull(&state);
			uint64_t timestamp;
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
					send.push(NetSynchronizeFiles);
					send.push(unitName);
					send.push(password);
					uint32_t fileSize = 0;
					for (int k = 0; k < unit->m_files.size(); k++) {
						SyncFile& file = unit->m_files[k];
						//if (file.m_status & StateFileFrozen)
						//	continue;
						fileSize++;
					}
					send.push(fileSize);
					//log::out << "NetSync " << unit->m_files.size() << "\n";
					for (int k = 0; k < unit->m_files.size(); k++) {
						SyncFile& file = unit->m_files[k];
						//if (file.m_status & StateFileFrozen)
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
		unit->lock();
		if (password != unit->m_password) {
			log::out << log::YELLOW << "Received wrong password\n";
			unit->unlock();
			return;
		}
		std::vector<std::string> fileRequests; // files to request, old version or not have
		std::vector<std::string> fileNotifications; // collision notifications to send
		std::vector<bool> sendFiles(unit->m_files.size(), true); // files to send, newest version, or you dont have

		uint32_t fileCount;
		buf.pull(&fileCount);
		//log::out << "fileCount "<<fileCount<<"\n";
		for (int i = 0; i < fileCount; i++) {
			std::string fileName;
			buf.pull(fileName, 256);
			SyncState state;
			buf.pull(&state);
			uint64_t time;
			buf.pull(&time);
					
			//if (state & StateFileFrozen)
			//	continue;

			bool found = false;
			for (int j = 0; j < unit->m_files.size(); j++) {
				SyncFile& file = unit->m_files[j];
				//log::out << fileName << " = " << file.m_name << "?\n";
				if (fileName == file.m_name) {
					sendFiles[j] = false;
					found = true;

					if (unit->m_status & StateUnitFrozen) {
						if (!(unit->m_status & StateFileMelted))
							break;
					}else if (file.m_status & StateFileFrozen)
						break;

					//log::out << file.m_status << " - "<<state<<"\n";
					if (file.check(StateFileNew)) {
						if (state & StateFileNew) {
							//log::out << "both new\n";
							if (file.check(StateFileOverride)) { // I have priority of overriding
								sendFiles[j] = true;
								//log::out << "override1\n";
							} else if (state & StateFileOverride) {
								fileRequests.push_back(fileName);
								//log::out << "override2\n";
							} else {
								file.set(StateFileCollision);
								if (!(state & StateFileCollision))
									fileNotifications.push_back(fileName);
							}
						} else {
							sendFiles[j] = true;
						}
					} else {
						if (state & StateFileNew) {
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
			notfifs.push(NetNotifyFiles);
			notfifs.push(unit->m_name);
			notfifs.push(unit->m_password);
			notfifs.push((uint32_t)fileNotifications.size());
			for (int i = 0; i < fileNotifications.size(); i++) {
				//log::out << sender << "Notify " << fileNotifications[i] << "\n";
				notfifs.push(fileNotifications[i]);
			}
			sender->send(notfifs, uuid,false);
		}
		if (fileRequests.size() > 0) {
			MessageBuffer req;
			req.push(NetRequestFiles);
			req.push(unit->m_name);
			req.push(unit->m_password);
			req.push((uint32_t)fileRequests.size());
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
			uint32_t fileCount;
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
						if (unit->m_status & StateUnitFrozen) {
							if (!(unit->m_status & StateFileMelted))
								break;
						} else if (file.m_status & StateFileFrozen)
							break;
						if (file.m_name == path) {
							file.set(StateFileCollision, true);
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
				uint32_t fileCount;
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
	enum StreamState : uint8_t {
		StreamNormal,
		StreamStart=1,
		StreamEnd=2,
	};
	typedef uint8_t StreamStates;
	void SyncApp::sendFile(SyncUnit* unit, SyncFile& file, engone::Sender* sender, engone::UUID uuid) {
		using namespace engone;

		// unit should already be locked

		if (unit->m_status & StateUnitFrozen) {
			if (!(unit->m_status & StateFileMelted))
				return;
		} else if (file.m_status & StateFileFrozen)
			return;

		// ISSUE: Here I assume the file will be received. Because if it isn't, the file on my side will not be new, and the file on the other side is still new.
		//   The file on the other side will then be assumed to be the newest which is not the case.
		file.set(StateFileNew, false);
		file.set(StateFileOverride, false);
		file.set(StateFileCollision, false);
		file.m_timestamp = unit->getTime(unit->m_root + file.m_name);
		//file.m_lastModified = unit->getTime(unit->m_root + file.m_name);

		if (file.isDir) {
			MessageBuffer send;
			send.push(NetSendFile);
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
		uint32_t bytesLeft = reader.size();
		bool fileEmpty = bytesLeft == 0;


		UUID fileUuid = UUID::New();

		//log::out << sender << "Send " << file.m_name << "\n";

		// THIS NEEDS TO CHANGE IF YOU CHANGE THE STRUCTURE OF THE MESSAGE
		uint32_t maxHeaderSize = sizeof(SyncNet) + sizeof(file.isDir) + sizeof(fileUuid) +
			sizeof(StreamStates) + sizeof(uint32_t)+8; // 8 for some extra bytes, why not

		maxHeaderSize += unit->m_name.length() + 5; // +5 because 4 is integer for length of string, 1 is for null char.
		maxHeaderSize += unit->m_password.length() + 5;
		maxHeaderSize += file.m_name.length() + 5;

		while (bytesLeft != 0 || fileEmpty) {
			fileEmpty = false;
			MessageBuffer send;
			send.push(NetSendFile);
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

			uint32_t transferSize = std::min(bytesLeft, sender->getTransferLimit() - maxHeaderSize);
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

			uint64_t time = unit->getTime(fullPath);
			if (file) {
				bool yes = !file->check(StateFileFrozen);
				if (unit->check(StateUnitFrozen))
					yes = file->check(StateFileMelted);

				if(yes){
					file->m_timestamp = time;
					file->set(StateFileNew, false);
					file->set(StateFileOverride, false);
					file->set(StateFileCollision, false);
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
					engone::GetTracker().track(writer);
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
			uint32_t transferSize;
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

					uint64_t time = download->unit->getTime(download->writer->getPath());

					SyncFile* file = download->unit->getFile(download->fileName, isDir);
					if (file) {
						bool yes = !file->check(StateFileFrozen);
						if (download->unit->check(StateUnitFrozen))
							yes = file->check(StateFileMelted);

						if (yes) {
							file->set(StateFileNew, false);
							file->set(StateFileOverride, false);
							file->set(StateFileCollision, false);
						}
					} else {
						download->unit->insertFile({ download->fileName,isDir,time });
					}

					download->unit->unlock();
					delete download->writer;
					download->writer = nullptr;
					engone::GetTracker().untrack(download->writer);
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
			conn->m_port = "492";
		}

		if (conn->m_status & StateServerType) {
			if (!serverConn) {
				if (!server) {
					server = new Server();
					GetTracker().track(server);
					server->setOnEvent([this](NetEvent type, UUID uuid) {
						//log::out << "serv event " << uuid <<" "<< type << "\n";
						if (serverConn) {
							serverConn->set(StateConnConnecting, false);
							if (type == NetEvent::Connect) {
								serverConn->set(StateConnLinked, true);
								refreshUnits();
								synchronizeUnits();
							} else if (type == NetEvent::Disconnect) {
								// A client disconnected
							} else if (type == NetEvent::Stopped) {
								serverConn->set(StateConnLinked, false);
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
						if (type == NetSynchronizeUnits) {
							recSyncUnits(buf, server,uuid);
						} else if (type == NetSynchronizeFiles) {
							recSyncFiles(buf, server, uuid);
						} else if (type == NetRequestFiles) {
							recReqFiles(buf, server, uuid);
						} else if (type == NetSendFile) {
							recSendFile(buf, server, uuid);
						} else if (type == NetNotifyFiles) {
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
						clientConn->set(StateConnConnecting, false);
						clientConn->set(StateConnFailed, false);
						clientConn->set(StateConnLinked, false);
					}
				}

				bool yes = server->start(conn->m_port);
				conn->set(StateConnReconnect, false);
				conn->set(StateConnFailed, false);
				if (yes) {
					// check if successful
					serverConn = conn;
					conn->set(StateConnLinked,true);
					//conn->set(StateConnConnecting,true);
				} else {
					conn->set(StateConnFailed, true);
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
					GetTracker().track(client);
					client->setOnEvent([this](NetEvent type, UUID uuid) {
						//log::out << "client event " << type<< "\n";
						if (clientConn) {
							clientConn->set(StateConnConnecting, false);
							if (type == NetEvent::Connect) {
								clientConn->set(StateConnLinked,true);
								refreshUnits();
								synchronizeUnits();
							} else if (type == NetEvent::Disconnect) {
								clientConn->set(StateConnLinked, false);
								clientConn = nullptr;
								if (waitingClient) {
									linkConnection(waitingClient);
									waitingClient = nullptr;
								}
							} else if (type == NetEvent::Failed) {
								clientConn->set(StateConnFailed,true);
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
						if (type == NetSynchronizeUnits) {
							recSyncUnits(buf, client, 0);
						} else if (type == NetSynchronizeFiles) {
							recSyncFiles(buf, client, 0);
						} else if (type == NetRequestFiles) {
							recReqFiles(buf, client, 0);
						} else if (type == NetSendFile) {
							recSendFile(buf, client, 0);
						} else if (type == NetNotifyFiles) {
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
						serverConn->set(StateConnConnecting, false);
						serverConn->set(StateConnLinked, false);
						serverConn->set(StateConnFailed, false);
					}
				}
				
				if (conn->m_ip.empty())
					conn->m_ip = "127.0.0.1";
				bool yes = client->start(conn->m_ip, conn->m_port);
				conn->set(StateConnReconnect, false);
				conn->set(StateConnFailed, false);
				if (yes) {
					clientConn = conn;
					conn->set(StateConnConnecting,true);
				} else {
					conn->set(StateConnFailed, true);
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
		return engone::IsKeybindingPressed(KeyMainAction) && line == selectedLine && !editingText;
	}
	void SyncApp::synchronizeUnits() {
		using namespace engone;
		if (client||server) {
			MessageBuffer buf;
			buf.push(NetSynchronizeUnits);

			//-- find valid and non-frozen units
			uint32_t unitSize = 0;
			for (int i = 0; i < userCache.getUnits().size(); i++) {
				SyncUnit* unit = userCache.getUnits()[i];
				if (!unit->check(StateUnitFrozen) && unit->validRoot())
					unitSize++;
			}
			//-- send units
			buf.push(unitSize);
			//log::out << "UNITSIZE " << unitSize << "\n";
			for (int i = 0; i < userCache.getUnits().size(); i++) {
				SyncUnit* unit = userCache.getUnits()[i];
				if (unit->check(StateUnitFrozen) || !unit->validRoot()) continue;
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
			if (conn->check(StateConnReconnect)) {
				linkConnection(conn);
			}
		}
	}
	void SyncApp::refreshUnits() {
		for (SyncUnit* unit : userCache.getUnits()) {
			unit->refresh();
		}
	}
	void SyncApp::onClose(engone::Window* window){
		userCache.save();
		if (client) {
			client->stop();
		}
		if (server) {
			server->stop();
		}
		m_window = nullptr;
		stop();
	};
	void SyncApp::update(engone::LoopInfo& info) {
		using namespace engone;
		if (upCooldown > 0) upCooldown -= info.timeStep;
		if (downCooldown > 0) downCooldown -= info.timeStep;
		if ((IsKeybindingDown(KeyUp) && upCooldown <= 0) || IsKeybindingPressed(KeyUp)) {
			upCooldown = 0.2;
			editingText = false;
			selectedLine--;
		}
		if ((IsKeybindingDown(KeyDown) && downCooldown <= 0) || IsKeybindingPressed(KeyDown)) {
			downCooldown = 0.2;
			editingText = false;
			selectedLine++;
		}

		if (options & AutoRefresh) {
			for (int i = 0; i < userCache.getUnits().size(); i++) {
				if (userCache.getUnits()[i]->m_wasRefreshed) {
					m_shouldSynchronize = true;
					userCache.getUnits()[i]->m_wasRefreshed = false;
				}
			}
		}
		if (m_shouldSynchronize) {
			synchronizeUnits();
			m_shouldSynchronize = false;
		}
	}
	void SyncApp::pendSynchronize() {
		m_shouldSynchronize = true;
	}
	void SyncApp::render(engone::LoopInfo& info) {
		using namespace engone;

		//chickenY += 1;
		//if (chickenY > 500) {
		//	chickenY = 0;
		//}
		//ui::Box box = {200, chickenY, 50, 50, { 1,1,1,1 }};
		//ui::Draw(box);

		int indentDepth = 2;

		// FEATURE: flag which relocates and syncs units when window loses focus.
		if (info.window->hasFocus() && !hadFocus) {
			log::out << "SyncApp::render - gained focus\n"; // debug stuff?
			//if (autoFeatures) {
			refreshUnits();
			synchronizeUnits();
			//}
		}
		hadFocus = info.window->hasFocus();

		selectedLine -= IsScrolledY();

		if (selectedLine < 0)
			selectedLine = 0;
		if (selectedLine > line - 1)
			selectedLine = line - 1;

		std::string fileDrop = PollPathDrop();

		indent = 0;
		line = 0;

		int top = (selectedLine + scrollY);
		int maxLine = GetHeight() / lineHeight;
		if (top < 1) {
			scrollY++;
		}
		if (top > maxLine - 3) {
			scrollY--;
		}
		renderText("Connections");
		for (int i = 0; i < userCache.getConnections().size(); i++) {
			SyncConnection* conn = userCache.getConnections()[i];
			
			indent = indentDepth;
			openAction(conn);
			if (mainAction()) {
				linkConnection(conn);
			}
			editAction(conn->m_name);
			if (IsKeybindingPressed(KeyDelete) && line == selectedLine && !editingText) {
				delete conn;
				GetTracker().untrack(conn);
				userCache.getConnections().erase(userCache.getConnections().begin() + i);
				i--;
				continue;
			}
			renderPart(conn);

			if (conn->m_status & StateOpen) {
				indent = 2* indentDepth;
				if (mainAction()) {
					// prevent changing type during a connection
					if (conn != serverConn && conn != clientConn && conn != waitingServer && conn != waitingClient) {
						if (conn->m_status & StateServerType) {
							conn->m_status = conn->m_status & (~StateServerType);
						} else {
							conn->m_status = conn->m_status | (StateServerType);
						}
					}
				}
				if (conn->m_status & StateServerType) {
					renderText("Type: Server");
				} else {
					renderText("Type: Client");
				}
				editAction(conn->m_ip);
				renderText("IP: ", conn->m_ip);
				editAction(conn->m_port);
				renderText("Port: ", conn->m_port);
			}
		}
		indent =  indentDepth;
		if (mainAction()) {
			SyncConnection* conn = new SyncConnection();
			userCache.getConnections().push_back(conn);
			GetTracker().track(conn);
		}
		renderText("+");

		indent = 0;

		renderText("Units");

		for (int i = 0; i < userCache.getUnits().size(); i++) {
			SyncUnit* unit = userCache.getUnits()[i];
		
			indent = indentDepth;
			openAction(unit);
			editAction(unit->m_name);
			if (IsKeybindingPressed(KeyDelete) && line == selectedLine && !editingText) {
				delete unit;
				GetTracker().untrack(unit);
				userCache.getUnits().erase(userCache.getUnits().begin() + i);
				i--;
				continue;
			}
			if (IsKeybindingPressed(KeyFreeze) && line == selectedLine && !editingText) {
				unit->set(StateUnitFrozen, !unit->check(StateUnitFrozen));
			}
			renderPart(unit);

			if (unit->m_status & StateOpen) {
				indent = 2 * indentDepth;
				editAction(unit->m_password);
				renderText("Password: ", unit->m_password);

				bool wasEditing = editingText;
				unit->lock(); // lock when using root
				editAction(unit->m_root);
				if (editingText && line == selectedLine) {
					if (!fileDrop.empty()) {
						unit->m_root = fileDrop;
						editingIndex = unit->m_root.length();
					}
				}
				unit->unlock();
				if (wasEditing && !editingText) {
					// stopped editing, relocate unit
					unit->refresh();
					unit->startMonitor(); // update monitor if root changed
				}
				unit->lock(); // lock when changing or using files
				renderText("Root: ", unit->m_root);

				if (unit->m_files.size() == 0) {
					renderText("No Files");
				} else {
					renderText("Files " + std::to_string(unit->m_files.size()));

					indent = 3 * indentDepth;
					std::string closed = "";
					bool flagOverride = 0;
					uint32_t minFlagIndent = -1; // -1 ignore
					for (int j = 0; j < unit->m_files.size(); j++) {
						SyncFile* file = &unit->m_files[j];
						
						// ISSUE: if file is below window break

						indent = 3 * indentDepth;
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
						int _indent = indent- indentDepth; // all files have \\ in the beginning so -indentDepth to skip it.
						for (int k = 0; k < file->m_name.length(); k++) {
							if (file->m_name[k] == '\\') {
								_indent+= indentDepth;
								lastIndex = k;
							}
						}
						indent = _indent;

						if (indent > minFlagIndent) { // note that -1 as unsigned will be larger than any number
							file->set(StateFileOverride, flagOverride);
							pendSynchronize();
						} else {
							minFlagIndent = -1;
						}
						if (subOfClosed)
							continue;

						if ((!file->check(StateOpen) || file->check(StateFileDeleted)) && file->isDir) {
							closed = file->m_name;
						}
						if (file->check(StateFileDeleted))
							continue;
						if (file->isDir) {
							openAction(file);
						}

						std::string path = file->m_name.substr(lastIndex + 1);

						if (IsKeybindingPressed(KeyMainAction) && line == selectedLine && !editingText) {
							if (file->check(StateFileNew)) {
								bool yes = !file->check(StateFileOverride);
								file->set(StateFileOverride, yes);
								if(yes)
									pendSynchronize();
								if (file->isDir) {
									flagOverride = yes;
									minFlagIndent = indent;
								}
							}
						}
						if (IsKeybindingPressed(KeyFreeze) && line == selectedLine && !editingText) {
							file->set(StateFileFrozen, !file->check(StateFileFrozen));
							if (!file->check(StateFileFrozen)&&file->check(StateFileOverride))
								pendSynchronize();
						}
						if (IsKeybindingPressed(KeyMelt) && line == selectedLine && !editingText) {
							file->set(StateFileMelted, !file->check(StateFileMelted));
						}
						if (file->isDir) {
							indent--;
							if (file->check(StateOpen))
								renderText("+");
							else
								renderText("-");
							indent++;
							line--;
							renderText(path);
							renderState(path, file->m_status);
						} else {
							renderText(path);
							renderState(path, file->m_status);
						}
					}
					indent = 3 * indentDepth;
				}
				unit->unlock();
			}
		}
		indent = 1 * indentDepth;
		if (mainAction()) {
			SyncUnit* unit = new SyncUnit();
			userCache.getUnits().push_back(unit);
			GetTracker().track(unit);
		}
		renderText("+");

		indent = 0;
	}
}