#pragma once

#ifdef ENGONE_LOGGER
#include "Engone/Logger.h"
#endif
//#include <string>

namespace engone {
	enum class NetEvent : int {
		// client/server connected
		Connect,
		// safe client/server disconnect
		Disconnect,
		// for client, abruptly failed to connect
		Failed,
		// for server when stopped
		Stopped,
	};
	std::string toString(NetEvent t);
	//log::logger operator<<(log::logger a, NetEvent b);
	Logger& operator<<(Logger& a, NetEvent b);
}