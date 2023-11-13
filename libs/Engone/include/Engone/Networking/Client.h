#pragma once

#include "Engone/Networking/NetEvent.h"
#include "Engone/Networking/Sender.h"
#include "Engone/Networking/MessageBuffer.h"

#include <thread>
#include <mutex>

// #ifdef ENGONE_TRACKER
// #include "Engone/Utilities/Tracker.h"
// #endif
namespace engone {
	class Connection;
	class Client : public Sender {
	public:
		// Constructor does nothing
		Client() : Sender(false) {}
		~Client();
		
		// Set lambdas before connecting.
		// Returns false if ip or port were invalid or if client already is running.
		// Returns true if client successfully connected.
		bool start(const std::string& ip, const std::string& port);
		// Will disconnect asynchonously.
		void stop();

		// Uuid and excludeUUID is not relevant for client
		void send(MessageBuffer& msg, UUID uuid = 0, bool excludeUUID = false,bool synchronous = false) override;

		inline bool isRunning() { return keepRunning; }

		// Waits for everything to terminate unlike stop.
		// Do not mutex lock this (risk for deadlock).
		void cleanup();
// #ifdef ENGONE_TRACKER
		// static TrackerId trackerId;
// #endif
	private:
		bool keepRunning = false;
		Connection* m_connection = nullptr;

		// Not the best struct but it does work
		struct Action {
			static const int START = 0;
			static const int STOP = 1;

			int type;
			std::string ip;
			std::string port;
		};
		std::vector<Action> m_workQueue;
		std::mutex m_workMutex;
		bool m_working=false;
		void work();

		std::thread m_workerThread; // connect thread
	
		std::mutex m_mutex; // general mutex
		int mutexDepth = 0;
		std::thread::id m_mutexOwner;
		void lock();
		void unlock();

		friend class Connection;
	};
}