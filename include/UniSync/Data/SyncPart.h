#pragma once

#include "UniSync/Data/SyncState.h"

namespace unisync {
	class SyncPart {
	public:
		SyncPart() = default;
		SyncPart(const std::string& name) : m_name(name) {};

		void set(SyncState state, bool on = true) {
			if (on) {
				m_status = m_status | state;
			} else {
				m_status = m_status & (~state);
			}
		}
		bool check(SyncState state) {
			return m_status & state;
		}
		SyncStates m_status = StateNone;
		std::string m_name = "Unknown";
	};
}