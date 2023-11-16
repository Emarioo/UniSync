#pragma once

#include "Engone/Logger.h"

namespace unisync {

	// NOTE: if you change existing values, the cache would be invalid
	enum Status : int {
		// General states
		STATUS_NONE = 0x000,
		STATUS_OPEN = 0x001,

		// Connection states
		STATUS_CON_IS_SERVER = 0x002,
		STATUS_CON_LINKED = 0x004,
		STATUS_CON_FAILED = 0x008,
		STATUS_CON_CONNECTING = 0x010,
		STATUS_CON_RECONNECT = 0x020,

		// File/Unit states
		STATUS_UNIT_FROZEN = 0x040, // all files will be frozen by default except for those with Melting
		STATUS_FILE_NEW = 0x080, // File is new or updated on disc
		STATUS_FILE_OVERRIDE = 0x100,
		STATUS_FILE_COLLISION = 0x200, // This state is ONLY used to show that two files are new and an override is needed.
		STATUS_FILE_MELTED = 0x400, // This file is never frozen
		STATUS_FILE_FROZEN = 0x800, // This file is frozen
		STATUS_FILE_DELETED = 0x1000, // indicates that the file is deleted
	};
	typedef int Status;
	enum SyncNet : int {
		//None = 0,
		NET_SYNC_UNITS = 1, // send all files for which units?
		NET_SYNC_FILES = 2, // send all files
		NET_SEND_FILE = 3, // 
		NET_REQUEST_FILE = 4,
		NET_NOTIFY_FILES = 5,
	};
	const char* ToString(SyncNet net);
	engone::Logger& operator<<(engone::Logger& logger, SyncNet net);

}