#include "UniSync/Data/Status.h"

namespace unisync {
	const char* ToString(SyncNet net) {
		if (net == NET_SYNC_UNITS) return "NET_SYNC_UNITS";
		if (net == NET_SYNC_FILES) return "NET_SYNC_FILES";
		if (net == NET_SEND_FILE) return "NET_SEND_FILE";
		if (net == NET_REQUEST_FILE) return "NET_REQUEST_FILE";
		return "NetStateUnknown";
	}
	engone::Logger& operator<<(engone::Logger& log, SyncNet net) {
		return log << ToString(net);
	}
}