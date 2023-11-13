#pragma once

#include "Engone/Logger.h"

namespace unisync {

	// NOTE: if you change existing values, the cache would be invalid
	enum SyncState : int {
		// General states
		StateNone = 0x000,
		StateOpen = 0x001,

		// Connection states
		StateServerType = 0x002,
		StateConnLinked = 0x004,
		StateConnFailed = 0x008,
		StateConnConnecting = 0x010,
		StateConnReconnect = 0x020,

		// File/Unit states
		StateUnitFrozen = 0x040, // all files will be frozen by default except for those with Melting
		StateFileNew = 0x080, // File is new or updated on disc
		StateFileOverride = 0x100,
		StateFileCollision = 0x200, // This state is ONLY used to show that two files are new and an override is needed.
		StateFileMelted = 0x400, // This file is never frozen
		StateFileFrozen = 0x800, // This file is frozen
		StateFileDeleted = 0x1000, // indicates that the file is deleted
	};
	typedef int SyncStates;
	enum SyncNet : int {
		//None = 0,
		NetSynchronizeUnits = 1, // send all files for which units?
		NetSynchronizeFiles = 2, // send all files
		NetSendFile = 3, // 
		NetRequestFiles = 4,
		NetNotifyFiles = 5,
	};
	const char* ToString(SyncNet net);
	engone::Logger& operator<<(engone::Logger& logger, SyncNet net);

}