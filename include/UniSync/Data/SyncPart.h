#pragma once

#include "UniSync/Data/Status.h"

namespace unisync {
	class SyncPart {
	public:
		SyncPart() = default;
		SyncPart(const std::string& name) : m_name(name) {};

		void set(Status state, bool on = true) {
			if (on) {
				m_status = m_status | state;
			} else {
				m_status = m_status & (~state);
			}
		}
		bool check(Status state) {
			return m_status & state;
		}
		Status m_status = STATUS_NONE;
		std::string m_name = "Unknown";
	};
}