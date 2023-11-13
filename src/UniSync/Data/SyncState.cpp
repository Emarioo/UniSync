#include "UniSync/Data/SyncState.h"

namespace unisync {
	const char* ToString(SyncNet net) {
		if (net == NetSynchronizeUnits) return "NetSynchronizeUnits";
		if (net == NetSynchronizeFiles) return "NetSynchronizeFiles";
		if (net == NetSendFile) return "NetSendFile";
		if (net == NetRequestFiles) return "NetRequestFiles";
		return "NetStateUnknown";
	}
	engone::Logger& operator<<(engone::Logger& log, SyncNet net) {
		return log << ToString(net);
	}
}