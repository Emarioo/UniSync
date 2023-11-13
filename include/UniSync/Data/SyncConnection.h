#pragma once

#include "UniSync/Data/SyncPart.h"

#include "Engone/Utilities/Tracker.h"

namespace unisync {
	class SyncConnection : public SyncPart {
	public:
		SyncConnection() = default;

		std::string m_ip;
		std::string m_port;

		static engone::TrackerId trackerId;
	};
}