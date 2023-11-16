#pragma once

#include "UniSync/Data/SyncPart.h"

namespace unisync {
	class SyncConnection : public SyncPart {
	public:
		SyncConnection() = default;

		std::string m_ip;
		std::string m_port;
	};
}