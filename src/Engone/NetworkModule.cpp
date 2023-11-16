#include "Engone/Networking/NetworkModule.h"

#include "Engone/Logger.h"

#include "Engone/Util/Alloc.h"
#include "Engone/Util/Utilities.h"

#include <condition_variable>

#include "Engone/Win32Includes.h"

//#ifdef _WIN32
//#define _WIN32_WINNT 0x0A00
//#endif

//#define _WIN32_WINNT 0x0501
//#define WIN32_LEAN_AND_MEAN
//#include <winsock2.h>
//#include <ws2tcpip.h>
//#include <windows.h>

//#include <string>
//#include <string_view>
//#include <memory>
//#include <thread>

#define IF_LEVEL(level) if(false)
// #define IF_LEVEL(level) if (HasLogLevel(NETWORK_LEVEL, level))

namespace engone {
// #ifdef ENGONE_TRACKER
// 	TrackerId Server::trackerId = "Server";
// 	TrackerId Client::trackerId = "Client";
// #endif

	//namespace alloc {
	//	void* malloc(uint64_t size) {
	//		if (size == 0) return nullptr;
	//		void* ptr = std::malloc(size);
	//		return ptr;
	//	}
	//	void free(void* ptr, uint64_t size) {
	//		if (!ptr) return;
	//		std::free(ptr);
	//	}
	//	void* realloc(void* ptr, uint64_t oldSize, uint64_t newSize) {
	//		if (newSize == 0) {
	//			free(ptr, oldSize);
	//			return nullptr;
	//		}
	//		void* newPtr = std::realloc(ptr, newSize);
	//		if (newPtr) {
	//			return newPtr;
	//		} else {
	//			return ptr;
	//		}
	//	}
	//}

	static WSADATA wsaData;
	static bool startedWSA = false;
	static std::mutex init_mutex;
	void InitNetworking() {
		init_mutex.lock();
		if (!startedWSA) {
			if (WSAStartup(MAKEWORD(2, 2), &wsaData)) {
				printf("Error with WSAStartup\n");
			} else {
				startedWSA = true;
			}
		}
		init_mutex.unlock();
	}
	void DestroyNetworking() {
		WSACleanup();
		startedWSA = false;
	}
	/*
		NETEVENT
	*/
	std::string toString(NetEvent t) {
		switch (t) {
		case NetEvent::Connect: return "Connect";
		case NetEvent::Disconnect: return "Disconnect";
		case NetEvent::Failed: return "Failed";
		case NetEvent::Stopped: return "Stopped";
		}
		return "";
	}
	Logger& operator<<(Logger& log, NetEvent b) {
		return log << toString(b);
	}
	/*
		MESSAGEBUFFER
	*/
	//MessageBuffer* MessageBuffer::Create(uint32_t size) {
	//	return new MessageBuffer(size);
	//}
	MessageBuffer::MessageBuffer(uint32_t size) {
		if (size != 0)
			resize(size);
	}
	void MessageBuffer::cleanup() {
		if (m_data && !m_sharing) {
			resize(0);
		}
	}

	MessageBuffer::MessageBuffer(const MessageBuffer& msg) {
		if (msg.m_data) {
			resize(msg.size());
			if (m_data) {
				std::memcpy(m_data, msg.m_data, msg.size()+sizeof(uint32_t));
			}
		}
	}
	//MessageBuffer MessageBuffer::operator=(const MessageBuffer& msg) {

	//}
	bool MessageBuffer::resize(uint32_t size) {
		if (m_sharing) return false; // resizing when message doesn't own the data will cause issues
		char* data;
		if (size == 0) {
			if (m_data) {
				Free(m_data, m_dataSize);
				//log::out << "Del MsgBuffer " << m_dataSize << "\n";
				//GetTracker().subMemory(trackerId,m_dataSize);
				m_data = nullptr;
				m_dataSize = 0;
				m_readHead = 0;
			}
			return true;
		} else {
			if (!m_data) {
				data = (char*)Allocate(size + sizeof(uint32_t));
				if (data)
					*((uint32_t*)data) = 0;
			} else {
				data = (char*)Reallocate(m_data, m_dataSize, size + sizeof(uint32_t));
			}
			if (data) {
				m_data = data;
				//GetTracker().addMemory(trackerId,size + sizeof(uint32_t)-m_dataSize);
				m_dataSize = size + sizeof(uint32_t);
				return true;
			} else {
				log::out << log::RED << "MessageBuffer::resize : malloc or realloc failed\n";
				return false;
			}
		}
	}
	char* MessageBuffer::pushBuffer(uint32_t bytes) {
		//if (bytes == 0) {
			// this is fine
			//log::out << log::Warning<<log::YELLOW << "MessageBuffer::pushBuffer - bytes is zero\n";
			//return nullptr;
		//}
		push(&bytes);
		if (m_dataSize < sizeof(uint32_t) + size() + bytes) {
			if (!resize((m_dataSize + bytes) * 2)) {
				// resize will give error
				return nullptr;
			}
		}
		uint32_t head = sizeof(uint32_t) + size();
		*((uint32_t*)m_data) += bytes;
		return m_data + head;
	}
	void MessageBuffer::push(const std::string& in) {
		if (m_dataSize < sizeof(uint32_t) + size() + in.length() + 1) {
			if (!resize((m_dataSize + in.length() + 1) * 2)) {
				// resize will give error
				return;
			}
		}
		uint32_t head = sizeof(uint32_t) + size();
		std::memcpy(m_data + head,  in.data(), in.length() + 1); // +1 for null termination. std::string is has null at end
		*((uint32_t*)m_data) += in.length() + 1;
	}
	void MessageBuffer::push(const char* in) {
		uint32_t len = strlen(in) + 1;
		if (m_dataSize < sizeof(uint32_t) + size() + len) {
			if (!resize((m_dataSize + len) * 2)) {
				// resize will give error
				return;
			}
		}
		uint32_t head = sizeof(uint32_t) + size();
		std::memcpy(m_data + head, in, len);
		*((uint32_t*)m_data) += len;
	}
	char* MessageBuffer::pullBuffer(uint32_t* bytes) {
		//if (*bytes == 0)
		//	return nullptr;
		pull(bytes);
		if (*bytes > size() - m_readHead) {
			// fishy
			log::out << log::RED << "MessageBuffer::pullBuffer : bytes of pulled buffer exceeds message!\n";
			return nullptr;
		}
		char* out = m_data + sizeof(uint32_t) + m_readHead;
		m_readHead += *bytes;
		return out;
	}
	void MessageBuffer::pull(std::string& out, uint32_t max) {
		if (m_data) {
			uint32_t siz = size();
			char* str = m_data + sizeof(uint32_t) + m_readHead;
			int maxlen = siz - m_readHead;
            if(max < maxlen)
                maxlen = max;
			uint32_t length = strnlen_s(str, maxlen) + 1;
			if (length > siz - m_readHead) {
				// fishy
				log::out << log::RED << "MessageBuffer::pull : length of string exceeds message!\n";
				return;
			}
			out.append(str);
			//out.append(std::string_view(m_data +sizeof(uint32_t)+ m_readHead));
			m_readHead += length;
		}
	}
	void MessageBuffer::flush() {
		if (m_data) {
			*((uint32_t*)m_data) = 0;
			m_readHead = 0;
		}
	}
	MessageBuffer* MessageBuffer::copy() {
		MessageBuffer* newMsg = ALLOC_NEW(MessageBuffer)(0);
		if (!newMsg) return nullptr;
		
		if (m_data) {
			bool yes = newMsg->resize(size());
			if (!yes) {
				ALLOC_DELETE(MessageBuffer, newMsg);
				return nullptr;
			}
			if (m_data) {
				std::memcpy(newMsg->m_data, m_data, size() + sizeof(uint32_t));
			}
		}
		return newMsg;
	}
	/*
		CONNECTION
	*/
	class Connection {
	public:
		// does nothing except set socket
		Connection(SOCKET socket) : m_socket(socket) {}
		~Connection() {
			cleanup();
		}
		void close() {
			if (m_socket == INVALID_SOCKET) return;
			SOCKET tmp = m_socket;
			m_socket = INVALID_SOCKET;
			closesocket(tmp);
		}

		void sendMsg(std::shared_ptr<MessageBuffer> msg, bool synchronous=false) {
			if (m_socket == INVALID_SOCKET) return;

			m_writeMutex.lock();
			if (synchronous) {
				int length = msg->size() + sizeof(uint32_t);
				int error = send(m_socket, msg->m_data, length, 0);
				if (error == SOCKET_ERROR || error != length) {
					int err = WSAGetLastError();
					log::out << log::RED << "Connection::sendMsg : send error "<< err<<"\n";
				} else {
					if (m_server) {
						m_server->m_mutex.lock();
						m_server->getStats().m_sentMessages++;
						m_server->getStats().m_sentBytes += length;
						m_server->m_mutex.unlock();
					} else if (m_client) {
						m_client->m_mutex.lock();
						m_client->getStats().m_sentMessages++;
						m_client->getStats().m_sentBytes += length;
						m_client->m_mutex.unlock();
					}
				}
			} else {
				m_outMessages.push_back(msg);
				m_cv.notify_one();
			}
			m_writeMutex.unlock();
			if(!synchronous)
				writeThread();
		}
		UUID m_uuid = 0;
// #ifdef ENGONE_TRACKER
// 		static TrackerId trackerId;
// #endif
	private:
		friend class Client;
		friend class Server;

		SOCKET m_socket = INVALID_SOCKET;

		MessageBuffer* m_buffer = nullptr;

		Server* m_server = nullptr;
		Client* m_client = nullptr;

		std::thread* m_readThread = nullptr;
		std::thread* m_writeThread = nullptr;
		bool m_writing = false;
		std::condition_variable m_cv;
		std::mutex m_uniqueMutex;

		std::mutex m_readMutex; // used when deleting threads
		std::mutex m_writeMutex;

		// will only start thread if it doesn't exist. It is fail safe.
		void readThread();
		// will only start thread if it doesn't exist and if there are messages. You can call this all you want basically. It is fail safe.
		void writeThread();
		uint32_t flushingLeft = 0;
		// will write and send the front message from m_outMessages.
		// will do nothing if m_outMessages is empty.

		void cleanup() {
			close();
			m_readMutex.lock();
			m_writeMutex.lock();
			//ENGONE_DEBUG(log::out << "cleanup notify\n";)
			m_cv.notify_one();
			if (m_readThread) {
				m_readThread->join();
				using namespace std;
				if (m_readThread)
					ALLOC_DELETE(thread, m_readThread);
				//delete m_readThread;
				m_readThread = nullptr;
			}
			if (m_writeThread) {
				m_writeThread->join();
				using namespace std;
				if (m_writeThread)
				ALLOC_DELETE(thread, m_writeThread);
				//delete m_writeThread;
				m_writeThread = nullptr;
			}
			if (m_buffer) {
				if (m_buffer)
					ALLOC_DELETE(MessageBuffer, m_buffer);
				//delete m_buffer;
				m_buffer = nullptr;
			}
			m_outMessages.clear();
			m_readMutex.unlock();
			m_writeMutex.unlock();
		}

		std::vector<std::shared_ptr<MessageBuffer>> m_outMessages;
	};
// #ifdef ENGONE_TRACKER
// 	// TrackerId Connection::trackerId = "Connection";
// #endif
	void Connection::readThread() {
		if (m_socket == INVALID_SOCKET) return;
		if (m_readThread) {
			log::out <<log::RED<< "Connection::readThread : cannot be called twice.\n";
			// you will most likely get stuck here if you call readThread twice.
			// 
			//m_readThread->join();
			//delete m_readThread;
			//m_readThread = nullptr;
			return;
		}
		m_readMutex.lock();
		if (!m_buffer) {
			m_buffer = ALLOC_NEW(MessageBuffer)();
			//GetTracker().track(m_buffer);
			m_buffer->resize(100);
			// prevent the data of this buffer from being deleted out of this scope
		}
		if (!m_buffer->m_data) {
			// no buffer for some reason
			log::out << log::RED << "Connection::readThread : Buffer failed!\n";
			m_readMutex.unlock();
			return;
		}
		// if the client send head and then a smaller size then expected. readBody will freeze until that data has come through.
		// This will only happend if the a programmer tampers with the client code. The same is true for server.
		using namespace std;
		m_readThread = ALLOC_NEW(thread)([this]() {
			if (m_server)
				engone::SetThreadName(-1, "Server recv");
			else if (m_client)
				engone::SetThreadName(-1, "Client recv");
			
			//ENGONE_DEBUG(log::out << m_uuid << " read thread start\n";)
			// not using mutex in thread because once the thread is running. NOTHING and NO ONE will touch the members of the connection.
			enum Next {
				Head,
				Body,
				Flush,
			};
			int expectingSize = 0;
			Next readNext = Head;
			while (true) {
				int readBytes = 0;
				
				int flags = MSG_WAITALL; // This is necessary!
				// NOTE: recv, winsock and networking in general has a network stack buffer. I don't know what size this is but behaviour you don't want may happen.
				///   This probably depends on the OS and other things.
				if (readNext == Head) {
					std::memset(m_buffer->m_data, 0, m_buffer->m_dataSize);
					m_buffer->moveReadHead(0);

					readBytes = recv(m_socket, m_buffer->m_data, sizeof(uint32_t), flags);
				} else if (readNext == Body) {
					readBytes = recv(m_socket, m_buffer->m_data + sizeof(uint32_t), m_buffer->size(), flags);
				} else if (readNext == Flush) {
					uint32_t size = flushingLeft;
					if (size > m_buffer->m_dataSize)
						size = m_buffer->m_dataSize;
					readBytes = recv(m_socket, m_buffer->m_data, size, 0);
				}
				//log::out << "READ " << readBytes << "\n";
				if (readBytes == SOCKET_ERROR || readBytes == 0) {
					int err = WSAGetLastError();
					if (readBytes == 0)
						err = 0;

					//ENGONE_DEBUG(log::out << m_uuid<<" recv error closing "<<err<<"\n";)
					close();
					if (m_server) {
						if (m_server->m_onEvent)
							m_server->m_onEvent(NetEvent::Disconnect, m_uuid);

						m_server->disconnect(m_uuid);
					}
					if (m_client) {
						if(m_client->m_onEvent)
							m_client->m_onEvent(NetEvent::Disconnect, 0);
						// onEvent can't touch readMutex because it already has it.
						
						m_client->stop();
					}

					break;
				}
				//m_readMutex.lock();
				if (readNext == Head&&readBytes==4) {
					uint32_t size = m_buffer->size();
					uint32_t limit = 0;
					if (m_server) limit = m_server->getTransferLimit();
					if (m_client) limit = m_client->getTransferLimit();
					if (size <= limit) {
						bool fine = true;
						if (m_buffer->m_dataSize - sizeof(uint32_t) < size) {
							if (!m_buffer->resize(size)) {
								fine = false;
							}
						}
						if (fine) {
							expectingSize = size;
							readNext = Body;
						} else {
							flushingLeft = size;
							readNext = Flush;
						}
					} else {
						if (m_buffer->m_dataSize - sizeof(uint32_t) < limit) {
							if (!m_buffer->resize(limit)) {
								log::out << log::RED << "Connection::readThread : Failed resizing buffer for flushing! THIS IS REALLY BAD!";
							}
						}
						// FEATURE: warning when message buffer is resized to something big, suggest streaming data in that case? same with 
						// FEATURE: data streaming? with files? uuid for every message, then part 1,2,3... for the split up parts of the whole data?
						// increase read limit or implement data streaming.
						log::out << log::RED << "Connection::readHead : Read limit " << limit << " reached! skipping " << size << " bytes. "<<(m_server?"Server\n":"Client\n");
						// Could you shorten this message?
						flushingLeft = size;
						readNext = Flush;
					}
				} else if (readNext == Body) {
					if (expectingSize != readBytes) {
						log::out << log::YELLOW << "Connection::readThread : readBytes did not match header's size "<<readBytes<<" != "<<expectingSize << "\n";
					}
					IF_LEVEL(NETWORK_LEVEL_SPAM)
						log::out << m_uuid << " recieved " << readBytes << "\n";
					//MessageBuffer copy = *m_buffer;
					//copy.m_sharing = true;
					if (m_server) {
						m_server->m_mutex.lock();
						m_server->getStats().m_receivedMessages++;
						m_server->getStats().m_receivedBytes += readBytes;
						m_server->m_mutex.unlock();
						if (m_server->m_onReceive) {
							bool stay = m_server->m_onReceive(*m_buffer, m_uuid);
							m_buffer->flush();
							if (!stay) {
								close();
							}
						}
					}
					if (m_client) {
						m_client->m_mutex.lock();
						m_client->getStats().m_receivedMessages++;
						m_client->getStats().m_receivedBytes += readBytes;
						m_client->m_mutex.unlock();
						if (m_client->m_onReceive) {
							bool stay = m_client->m_onReceive(*m_buffer, 0);
							m_buffer->flush();
							if (!stay)
								close();
						}
					}
					readNext = Head;
				} else if (readNext == Flush) {
					flushingLeft -= readBytes;
					log::out << "Flushed " << readBytes << " (" << flushingLeft << " left)\n";

					if (flushingLeft == 0)
						readNext = Head;
					else
						readNext = Flush;
				} else {
					log::out << log::RED << "Connection::readThread : some error\n";
				}
			}
			//m_readMutex.unlock();
			//ENGONE_DEBUG(log::out << m_uuid << " read thread stop\n";)
		});
		m_readMutex.unlock();
	}
	void Connection::writeThread() {
		if (m_socket == INVALID_SOCKET||m_writing) {
			return;
		}

		if (m_writeThread)
			m_writeThread->join();

		m_writeMutex.lock();
		using namespace std;
		if(m_writeThread)
			ALLOC_DELETE(thread, m_writeThread);
		//delete m_writeThread;
		m_writeThread = nullptr;
		m_writing = true;
		m_writeThread = ALLOC_NEW(std::thread)([this]() {
			if (m_server)
				engone::SetThreadName(-1, "Server send");
			else if (m_client)
				engone::SetThreadName(-1, "Client send");

			bool shouldUnlock = true; // may not be necessary

			m_writeMutex.lock();
			//ENGONE_DEBUG(log::out << m_uuid << " write thread start\n";)
			while (m_outMessages.size() != 0) {
				auto& msg = m_outMessages.front();
				uint32_t length = msg->size() + sizeof(uint32_t);
				//log::out << "SENDING "<<m_server << " "<<m_client <<" "<< length << "\n";
				//ENGONE_DEBUG(log::out << "send " <<length << "\n";)
				m_writeMutex.unlock();
				int error = send(m_socket, msg->m_data, length, 0);
				if (error == SOCKET_ERROR||error!=length) {
					log::out << log::RED << "Connection::writeThread : send error\n";
					shouldUnlock = false;
					//int err = WSAGetLastError();
					//if (!forcedShutdown(err)) {
					//	log::out << "writeBody error " << err << " " << m_uuid << "\n";
					//	log::out << "Reader broken\n";
					//}
					// write thread can close quitely.
					break;
				}
				IF_LEVEL(NETWORK_LEVEL_SPAM)
					log::out << m_uuid <<" sent "<<error<<"\n";
				if (m_server) {
					//ENGONE_DEBUG(log::out << "wt trylock server\n";)
					m_server->m_mutex.lock();
					//ENGONE_DEBUG(log::out << "wt lock server\n";)
					m_server->getStats().m_sentMessages++;
					m_server->getStats().m_sentBytes += length;
					//ENGONE_DEBUG(log::out << "wt unlock server\n";)
					m_server->m_mutex.unlock();
				} else if (m_client) {
					//ENGONE_DEBUG(log::out << "wt trylock client\n";)
					m_client->m_mutex.lock();
					//ENGONE_DEBUG(log::out << "wt lock client\n";)
					m_client->getStats().m_sentMessages++;
					m_client->getStats().m_sentBytes += length;
					//ENGONE_DEBUG(log::out << "wt unlock client\n";)
					m_client->m_mutex.unlock();
				}
				m_writeMutex.lock();
				m_outMessages.erase(m_outMessages.begin());
				m_writeMutex.unlock();
				// space here to allow new messages flowing in.
				//ENGONE_DEBUG(log::out << "wt Waiting for msg\n";)
				std::unique_lock<std::mutex> lock(m_uniqueMutex);
				if (m_outMessages.size() == 0&& m_socket!=INVALID_SOCKET)
					m_cv.wait(lock);
				//ENGONE_DEBUG(log::out << "wt continuing\n";)
				if (m_outMessages.size() == 0) { // break if we still don't have messages. we still need to lock mutex because we unlock it later
					shouldUnlock = false;
					break;
				}
				m_writeMutex.lock();
			}
			//ENGONE_DEBUG(log::out << m_uuid << " write thread stop\n";)
			if (shouldUnlock) {
				m_writeMutex.unlock();
			}
			m_writing = false;
		});
		m_writeMutex.unlock();
	}
	/*
		SERVER
	*/
	Server::~Server() {
		cleanup();
		IF_LEVEL(NETWORK_LEVEL_INFO)
			log::out << "Server : Destructor finished\n";
	}
	bool Server::start(const std::string& port) {
		if (!hasCallbacks()) {
			log::out << log::RED << "Server::start : has no callbacks\n";
			return false;
		}
		m_port = port;

		m_workMutex.lock();
		m_workQueue.push_back({Action::START, (UUID)0, port});
		m_workMutex.unlock();
		work();

		return true;
	}
	void Server::disconnect(UUID uuid) {
		m_workMutex.lock();
		m_workQueue.push_back({ Action::DISCONNECT, uuid});
		m_workMutex.unlock();
		work();
	}
	void Server::stop() {
		m_workMutex.lock();
		m_workQueue.push_back({ Action::STOP });
		m_workMutex.unlock();
		work();
	}
	void Server::work() {
		m_workMutex.lock();
		if (!m_working) {
			m_working = true;
			if (m_workerThread.joinable())
				m_workerThread.join();
			m_workerThread = std::thread([this]() {
				engone::SetThreadName(-1, "Server worker");

				IF_LEVEL(NETWORK_LEVEL_INFO)
					log::out << "Server : Worker thread started\n";
				while (true) {
					m_workMutex.lock();
					if (m_workQueue.empty()) {
						m_workMutex.unlock();
						break;
					}
					Action action = m_workQueue.front();
					m_workQueue.erase(m_workQueue.begin());
					m_workMutex.unlock();

					if (action.type == Action::START) {
						if (!m_onEvent ||!m_onReceive) {
							log::out << log::RED << "Server : missing callbacks\n";
							
						} else {
							InitNetworking();

							cleanup(); // do not lock this
							lock();

							keepRunning = true;

							//uint16_t intPort = 0;
							//try {
							//	intPort = std::stoi(action.port);
							//} catch (std::invalid_argument err) {
							//	keepRunning = false;
							//}

							struct addrinfo* result = NULL, * ptr = NULL, hints;

							ZeroMemory(&hints, sizeof(hints));
							hints.ai_family = AF_INET;
							hints.ai_socktype = SOCK_STREAM;
							hints.ai_protocol = IPPROTO_TCP;
							hints.ai_flags = AI_PASSIVE;
							// Todo: fix htons and stuff

							int err = getaddrinfo(NULL, action.port.c_str(), &hints, &result);
							if (err != 0) {
								// Error getaddr
								keepRunning = false;
							}

							SOCKET newSocket = 0;
							if (keepRunning) {
								newSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
								if (newSocket == INVALID_SOCKET) {
									freeaddrinfo(result);
									log::out << "Error with server socket\n";
									keepRunning = false;
								}
							}
							if (keepRunning) {
								//m_port = port;
								//m_workerThread = std::thread([this, newSocket, intPort, result]() {
								//	engone::SetThreadName(-1, "Server worker");
									//lock();
									//struct sockaddr_in service;
									//service.sin_family = AF_INET;
									//service.sin_family = AF_INET;
									// localhost as ip does not work
									//service.sin_addr.s_addr = inet_addr("127.0.0.1");
									//int ret = inet_pton(service.sin_family, "127.0.0.1",&service.sin_addr.S_un.S_addr);

									//service.sin_port = htons(intPort);
									//int error = bind(newSocket, (SOCKADDR*)&service, sizeof(service));
									//log::out << "name length " << result->ai_addrlen << "\n";
									int error = bind(newSocket, result->ai_addr, (int)result->ai_addrlen);
									freeaddrinfo(result);
									if (error == SOCKET_ERROR) {
										printf("Error with server bind\n");
										closesocket(newSocket);
										keepRunning = false;
									}
									if (keepRunning) {
										int backlog = 6;
										int error = listen(newSocket, backlog);
										if (error == SOCKET_ERROR) {
											printf("Error with server listen\n");
											closesocket(newSocket);
											keepRunning = false;
										}
									}
									if (keepRunning) {
										m_socket = (void*)newSocket;
										m_port = std::move(action.port);
										// thread joined in cleanup
										m_acceptThread = std::thread([this]() {
											engone::SetThreadName(-1, "Server acceptor");
											//ENGONE_DEBUG(log::out << "accept thread start\n";)
											while (keepRunning) {
												//printf("accepting\n");
												SOCKET newSocket = accept((SOCKET)m_socket, NULL, NULL);

												if (newSocket == INVALID_SOCKET) {
													//  Usually means the server socket was closed
													break;
												}

												//UUID uuid = rand();
												UUID uuid = UUID::New();

												Connection* conn = ALLOC_NEW(Connection)(newSocket);
												conn->m_uuid = uuid;
												conn->m_server = this;
												//GetTracker().track(conn);

												m_connectionsMutex.lock();
												m_connections[uuid] = conn; // needs to be before onEvent because sending messages require the connection 
												m_connectionsMutex.unlock();
												// to be listed in m_connections

												bool keep = m_onEvent(NetEvent::Connect, uuid);

												if (!keep) {
													m_connectionsMutex.lock();
													ALLOC_DELETE(Connection, conn);
													//delete conn;
													m_connections.erase(uuid);
													m_connectionsMutex.unlock();
													continue;
												}
												conn->readThread();
											}
											int err = WSAGetLastError();

											//ENGONE_DEBUG(log::out<<"accept call stop\n";)
											stop();
											//unlock();
											m_onEvent(NetEvent::Stopped, 0);
											//lock();
											//ENGONE_DEBUG(log::out << "accept thread finished\n";)

										});
									}
									//unlock();
								//});
							}
							//bool out = keepRunning;
							unlock();
							//return out;
						}
					} else if (action.type == Action::STOP) {
						cleanup();
					} else if (action.type == Action::DISCONNECT) {
						m_connectionsMutex.lock();
						auto find = m_connections.find(action.uuid);
						if (find != m_connections.end()) {
							ALLOC_DELETE(Connection, find->second);
							m_connections.erase(find);
						}
						m_connectionsMutex.unlock();
					}
				}
				m_workMutex.lock();
				m_working = false;
				m_workMutex.unlock();

				IF_LEVEL(NETWORK_LEVEL_INFO)
					log::out << "Server : Worker thread finished\n";
			});
		}
		m_workMutex.unlock();
	}
	void Server::cleanup() {
		//if (!keepRunning) return; // already cleaned or being cleaned up. workthread will no join if work cleansup.
		IF_LEVEL(NETWORK_LEVEL_INFO)
			log::out << "Server : cleanup started\n";

		keepRunning = false;
		if ((SOCKET)m_socket != INVALID_SOCKET) {
			closesocket((SOCKET)m_socket);
			m_socket = (void*)INVALID_SOCKET;
		}

		lock();
		if (m_acceptThread.joinable() && std::this_thread::get_id() != m_acceptThread.get_id())
			m_acceptThread.join();

		// ISSUE: What if the worker thread is active and tries to delete a connection while connections
		//		are being looped through? use mutex lock for connections?
		m_connectionsMutex.lock();
		for (auto& pair : m_connections) {
			//delete conn;
			ALLOC_DELETE(Connection, pair.second);
		}
		m_connections.clear();
		m_connectionsMutex.unlock();

		m_port.clear();

		unlock();

		// wait for work to finish
		if (m_workerThread.joinable() && std::this_thread::get_id() != m_workerThread.get_id())
			m_workerThread.join();
		IF_LEVEL(NETWORK_LEVEL_INFO)
			log::out << "Server : cleanup finished\n";
	}
	void Server::send(MessageBuffer& msg, UUID uuid, bool ignore, bool synchronous) {
		if (!keepRunning) return;
		int size = msg.size();
		if (size == 0) {
			log::out <<log::YELLOW<< "You tried to send no data?\n";
			return;
		}
		if (size > (int)getTransferLimit()) {
			log::out << log::RED << "Server::send : Message to big ("<<size<<" > "<<getTransferLimit() << ")\n";
			return;
		}
		lock();
		bool quit = false;
		if (m_connections.size() == 0) {
			//log::out << "no connections\n";
			quit = true;
		}
		if (!quit) {
			// ISSUE: how to track memory of shared ptr.
			//log::out << (void*)msg.m_data << " Send ptr\n";
			std::shared_ptr<MessageBuffer> ptr = std::make_shared<MessageBuffer>(msg);
			msg.m_sharing = true;

			if (!ignore) {
				if (uuid != 0) {
					auto pair = m_connections.find(uuid);
					if (pair != m_connections.end()) {
						pair->second->sendMsg(ptr, synchronous);
					}
				}
			} else {
				for (auto& pair : m_connections) {
					if (uuid != pair.first) {
						pair.second->sendMsg(ptr, synchronous);
					}
				}
			}
		}
		unlock();
	}
	void Server::lock() {
		if (m_mutexOwner != std::this_thread::get_id()) {
			//ENGONE_DEBUG(log::out << "server maybe lock\n";)
			m_mutex.lock();
			//ENGONE_DEBUG(log::out << "server locked\n";)
			m_mutexOwner = std::this_thread::get_id();
			mutexDepth++;
		} else {
			mutexDepth++;
		}
	}
	void Server::unlock() {
		if (m_mutexOwner == std::this_thread::get_id()) {
			mutexDepth--;
			if (mutexDepth == 0) {
				m_mutexOwner = {};
				//ENGONE_DEBUG(log::out << "server unlocked\n";)
				m_mutex.unlock();
			}
		}
	}
	/*
		CLIENT
	*/
	Client::~Client() {
		cleanup();
		IF_LEVEL(NETWORK_LEVEL_INFO)
			log::out << "Client : Destructor finished\n";
	}
	bool Client::start(const std::string& ip, const std::string& port) {
		m_workMutex.lock();
		m_workQueue.push_back({Action::START,ip,port});
		m_workMutex.unlock();
		work();

		return true;
	}
	void Client::stop() {
		m_workMutex.lock();
		m_workQueue.push_back({Action::STOP});
		m_workMutex.unlock();
		work();
		//if (keepRunning) {
		//	keepRunning = false;
		//	//ENGONE_DEBUG(log::out << "Client started stop\n";)
		//	if (m_workerThread.joinable() && std::this_thread::get_id() != m_workerThread.get_id())
		//		m_workerThread.join();
		//	lock();
		//	m_workerThread = std::thread([this]() {

		//		//ENGONE_DEBUG(log::out << "client work thread(clean) start\n";)
		//		cleanup();
		//		//ENGONE_DEBUG(log::out << "client work thread(clean) stop\n";)
		//
		//	});
		//	unlock();
		//	//ENGONE_DEBUG(log::out << "Client finish stop\n";)
		//}
	}
	void Client::work() {
		m_workMutex.lock();
		if (!m_working) {
			m_working = true;
			if (m_workerThread.joinable())
				m_workerThread.join();
			m_workerThread = std::thread([this]() {
				engone::SetThreadName(-1, "Client worker");
				IF_LEVEL(NETWORK_LEVEL_INFO)
					log::out << "Client : Started worker thread\n";
				m_workMutex.lock();
				while (!m_workQueue.empty()) {
					Action action = m_workQueue.front();
					m_workQueue.erase(m_workQueue.begin());
					m_workMutex.unlock();

					if (action.type == Action::START) {
						//if (keepRunning || m_connection) {
						//	return false;
						//}
						if (!m_onEvent || !m_onReceive) {
							log::out << "Client : Missing lambdas\n";
						} else {
							InitNetworking();

							cleanup(); // can't start client if everything isn't cleaned up.
							lock();
							keepRunning = true;

							struct addrinfo* result = NULL, hints;

							ZeroMemory(&hints, sizeof(hints));
							hints.ai_family = AF_UNSPEC;
							hints.ai_socktype = SOCK_STREAM;
							hints.ai_protocol = IPPROTO_TCP;

							int err = getaddrinfo(action.ip.c_str(), action.port.c_str(), &hints, &result);
							if (err != 0) {
								// Error getaddr
								keepRunning = false;
							} else {
								SOCKET newSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
								if (newSocket == INVALID_SOCKET) {
									// Error with socket
									freeaddrinfo(result);
									keepRunning = false;
								} else {
									m_connection = ALLOC_NEW(Connection)(newSocket);
									m_connection->m_uuid = 0;
									m_connection->m_client = this;
									unlock();
									int error = connect(newSocket, result->ai_addr, (int)result->ai_addrlen);
									lock();
									freeaddrinfo(result);

									if (error == SOCKET_ERROR) {
										int code = WSAGetLastError();
										// Error with client connect
										if (code == WSAETIMEDOUT) {
											log::out << "Client : Connection timed out (err: " << code << ")\n";
										} else if (code == WSAECONNREFUSED) {
											log::out << "Client : Connection refused (err: " << code << ")\n";
										} else
											log::out << "Client : Error code: " << code << "\n";
											//ENGONE_DEBUG(printf("Did not connect %d\n", code);)
										if (m_connection) {
											ALLOC_DELETE(Connection, m_connection);
											m_connection = nullptr;
										}
										keepRunning = false;
										unlock();
										m_onEvent(NetEvent::Failed, 0);
										lock();
									} else {

										m_ip = std::move(action.ip);
										m_port = std::move(action.port);

										unlock();
										bool keep = m_onEvent(NetEvent::Connect, 0);
										lock();
										if (!keep) {
											cleanup();
										} else {
											m_connection->readThread();
										}
									}
								}
							}
							unlock();
						}
					} else if (action.type == Action::STOP) {
						cleanup();
					}

					m_workMutex.lock();
				}
				m_working = false;
				m_workMutex.unlock();
				IF_LEVEL(NETWORK_LEVEL_INFO)
					log::out << "Client : Finished worker thread\n";
			});
		}
		m_workMutex.unlock();
	}
	void Client::cleanup() {
		//if (!keepRunning) return; // already cleaned or being cleaned. ALTOUGH if worker cleanups then the work thread itself will not be cleaned.
		IF_LEVEL(NETWORK_LEVEL_INFO)
			log::out << "Client : cleanup start\n";

		// here you should disallow start work to happen.
		// you don't want the client to start back up if is being destroyed.

		keepRunning = false; // do this last? no because of if(!keepRunning) at top.
		// another bool to indicate client being destroyed.
		// client being stopped gracefully and being terminated are different things.
		// gracefully is done by player when they quit a server. termination is done when the game is forcefully shutdown.

		lock();
		m_ip.clear();
		m_port.clear();
		if (m_connection) {
			//delete m_connection;
			ALLOC_DELETE(Connection, m_connection);
			m_connection = nullptr;
		}
		unlock();

		// wait for work to finish. the only work is
		if (m_workerThread.joinable()&&std::this_thread::get_id()!=m_workerThread.get_id()) {
			m_workerThread.join();
		}
		IF_LEVEL(NETWORK_LEVEL_INFO)
			log::out << "Client : cleanup finished\n";
	}
	void Client::send(MessageBuffer& msg, UUID uuid, bool ignore, bool synchronous) {
		if (!keepRunning) return;
		if (!m_connection) return;
		if (msg.size() == 0) {
			log::out << log::RED << "You tried to send no data?\n";
			return;
		}
		if (msg.size() > getTransferLimit()) {
			log::out << log::RED << "Buffer is bigger than the transfer limit!\n";
		}
		// ISSUE: how to track memory of shared ptr. increment sizeof(MessageBuffer) when is it destroyed?
		// Make a custom ptr? heeeeeellllllp...
		std::shared_ptr<MessageBuffer> ptr = std::make_shared<MessageBuffer>(msg);
		
		lock();
		// lock a different connection mutex? replacement for lock()?
		m_connection->sendMsg(ptr,synchronous);
		msg.m_sharing = true;
		unlock();
	}
	void Client::lock() {
		if (m_mutexOwner != std::this_thread::get_id()) {
			//ENGONE_DEBUG(log::out << "client maybe lock\n";)
			m_mutex.lock();
			//ENGONE_DEBUG(log::out << "client locked\n";)
			m_mutexOwner = std::this_thread::get_id();
			mutexDepth++;
		} else {
			mutexDepth++;
		}
	}
	void Client::unlock() {
		if (m_mutexOwner == std::this_thread::get_id()) {
			mutexDepth--;
			if (mutexDepth == 0) {
				m_mutexOwner = {};
				m_mutex.unlock();
				//ENGONE_DEBUG(log::out << "client unlocked\n";)
			}
		}
	}
}