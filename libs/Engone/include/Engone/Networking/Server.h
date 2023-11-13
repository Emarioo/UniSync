#pragma once

#include "Engone/Networking/NetEvent.h"
#include "Engone/Networking/Sender.h"
#include "Engone/Networking/MessageBuffer.h"

//#include <functional>
//#include <unordered_map>
#include <thread>
#include <mutex>

// #ifdef ENGONE_TRACKER
// #include "Engone/Utilities/Tracker.h"
// #endif

namespace engone {
	class Connection;
	class Server : public Sender {
	public:
		Server() : Sender(true, "127.0.0.1") {}
		~Server();

		// Set lambdas before starting.
		// Returns false if port was invalid or if it already is running.
		// Returns true if server started successfully.
		bool start(const std::string& port);
		// Close one connection
		void disconnect(UUID uuid);

		// Stop server and all connections asynchronously.
		// Does not wait for connections to close.
		void stop();
		void send(MessageBuffer& msg, UUID uuid = 0, bool excludeUUID = false, bool synchronous = false) override;

		uint32_t getConnectionCount() const { return m_connections.size(); }

		inline bool isRunning() { return keepRunning; }

		// Will wait for everything to terminate unlike stop
		// Never mutex lock this. A deadlock may occur
		void cleanup();

// #ifdef ENGONE_TRACKER
// 		static TrackerId trackerId;
// #endif
	private:
		bool keepRunning = false;

		std::unordered_map<UUID, Connection*> m_connections;
		std::mutex m_connectionsMutex;

		// void* should be SOCKET, but isn't so that you don't need to include in this header.
		void* m_socket=nullptr;
		std::thread m_acceptThread;
		std::thread m_workerThread;

		struct Action {
			static const int START = 0;
			static const int STOP = 1;
			static const int DISCONNECT = 2; // disconnect a client/socket/connection

			int type;
			UUID uuid;
			std::string port;
		};
		std::vector<Action> m_workQueue;
		std::mutex m_workMutex;
		bool m_working = false;
		void work();

		std::mutex m_mutex;
		std::thread::id m_mutexOwner;
		int mutexDepth = 0;
		void lock();
		void unlock();

		friend class Connection;
	};
}