
#include "UniSync/Data/UserCache.h"

#include "Engone/Engone.h"

namespace unisync {
	UserCache::UserCache(const std::string& path) {
		load(m_path);
	}
	UserCache::~UserCache() {
		clean();
	}
	void UserCache::clean() {
		for (int i = 0; i < m_units.size(); i++) {
			// delete files?
			engone::GetTracker().untrack(m_units[i]);
			delete m_units[i];
		}
		m_units.clear();
		for (int i = 0; i < m_connections.size(); i++) {
			engone::GetTracker().untrack(m_connections[i]);
			delete m_connections[i];
		}
		m_connections.clear();
	}
	void UserCache::load(const std::string& path) {
		m_path = path;
		load();
	}
	void UserCache::load() {
		using namespace engone;
		FileReader file(m_path);

		try {
			int connSize;
			file.read(&connSize);
			for (int i = 0; i < connSize; i++) {
				SyncConnection* conn = new SyncConnection();
				GetTracker().track(conn);
				m_connections.push_back(conn);
				file.read(&conn->m_name);
				file.read(&conn->m_status);
				file.read(&conn->m_ip);
				file.read(&conn->m_port);

				// disable reconnect on loading
				//conn->m_status &= ~(StateConnReconnect);
			}
			int unitSize;
			file.read(&unitSize);
			for (int i = 0; i < unitSize; i++) {
				SyncUnit* unit = new SyncUnit();
				GetTracker().track(unit);
				m_units.push_back(unit);
				file.read(&unit->m_name);
				file.read(&unit->m_status);
				file.read(&unit->m_password);
				file.read(&unit->m_root);
				file.read(&unit->m_timestamp);
				int fileSize;
				file.read(&fileSize);
				for (int j = 0; j < fileSize; j++) {
					unit->m_files.push_back({});
					SyncFile& sfile = unit->m_files[j];
					file.read(&sfile.m_name);
					file.read(&sfile.m_status);
					file.read(&sfile.m_timestamp);
					file.read(&sfile.isDir);
				}
			}
		} catch (Error err) {
			log::out << "Cache load: " << err << "\n";
		}
	}
	void UserCache::save() {
		using namespace engone;
		FileWriter file(m_path);
		
		try {
			uint32_t connSize = m_connections.size();
			file.write(&connSize);
			for (int i = 0; i < m_connections.size(); i++) {
				SyncConnection* conn = m_connections[i];
				file.write(&conn->m_name);
				SyncStates tempState = conn->m_status;
				if (conn->m_status & (StateConnLinked | StateConnFailed | StateConnConnecting))
					tempState = tempState | StateConnReconnect;
				tempState = tempState & (~(StateConnLinked | StateConnFailed | StateConnConnecting));
				file.write(&tempState);
				file.write(&conn->m_ip);
				file.write(&conn->m_port);
			}
			uint32_t unitSize = m_units.size();
			file.write(&unitSize);
			for (int i = 0; i < m_units.size(); i++) {
				SyncUnit* unit = m_units[i];
				file.write(&unit->m_name);
				file.write(&unit->m_status);
				file.write(&unit->m_password);
				file.write(&unit->m_root);
				file.write(&unit->m_timestamp);
				uint32_t fileSize = 0;
				for (int j = 0; j < unit->m_files.size(); j++) {
					if (unit->m_files[j].check(StateFileDeleted))
						continue;
					fileSize++;
				}
				file.write(&fileSize);
				for (int j = 0; j < unit->m_files.size(); j++) {
					SyncFile& sfile = unit->m_files[j];
					if (sfile.check(StateFileDeleted))
						continue;
					file.write(&sfile.m_name);
					file.write(&sfile.m_status);
					file.write(&sfile.m_timestamp);
					file.write(&sfile.isDir);
				}
			}
		} catch (Error err) {
			log::out << "Cache save: " << err << "\n";
		}
	}
	SyncUnit* UserCache::getUnit(const std::string& name) {
		for (int i = 0; i < m_units.size(); i++) {
			if (!m_units[i]->validRoot())
				continue;
			if (name == m_units[i]->m_name) {
				return m_units[i];
			}
		}
		return nullptr;
	}
}