
#include "UniSync/Data/UserCache.h"

// #include "Engone/Engone.h"

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
			// engone::GetTracker().untrack(m_units[i]);
			delete m_units[i];
		}
		m_units.clear();
		for (int i = 0; i < m_connections.size(); i++) {
			// engone::GetTracker().untrack(m_connections[i]);
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
        auto file = engone::FileOpen(m_path, nullptr, engone::FILE_ONLY_READ);
        if(!file) {
            log::out << log::RED << "bad path to load cache\n";
            return;
        }
        CacheHeader header{};
        bool success = false;
        
        #define READ(P,T) success = engone::FileRead(file, P, sizeof(T)); if(!success) goto load_err;
        #define READS(S) { u8 len = 0; success = engone::FileRead(file, &len, sizeof(len)); if(len > 0) { if(!success) goto load_err; S.resize(len); success = engone::FileRead(file, (char*)S.data(), len); if(!success) goto load_err; } }
        
        READ(&header, header)
        Assert(("potential corrupted cache",header.countOfConnections < 100)); // connSize is probably corrupted if more than 100
        Assert(("potential corrupted cache",header.countOfUnits < 200)); // connSize is probably corrupted if
        for (int i = 0; i < header.countOfConnections; i++) {
            SyncConnection* conn = new SyncConnection();
            m_connections.push_back(conn);
            /*
                struct {
                    name
                    states
                    ip
                    port   
                }
            */
            READS(conn->m_name)
            READ(&conn->m_status, conn->m_status)
            READS(conn->m_ip)
            READS(conn->m_port)
        }
        for (int i = 0; i < header.countOfUnits; i++) {
            SyncUnit* unit = new SyncUnit();
            m_units.push_back(unit);
            /*
                struct {
                    name
                    status
                    password
                    root
                    timestamp
                    filecount
                    files = struct {
                        name
                        status
                        timestamp
                        isdir
                    }
                }
            */
            READS(unit->m_name)
            READ(&unit->m_status, unit->m_status)
            READS(unit->m_password)
            READS(unit->m_root)
            READ(&unit->m_timestamp, unit->m_timestamp)
            
            u32 fileCount = 0;
            READ(&fileCount, fileCount)
            
            unit->m_files.reserve(fileCount + 10);
            unit->m_files.resize(fileCount);
            
            for (int j = 0; j < fileCount; j++) {
                SyncFile& sfile = unit->m_files[j];
                sfile = {};
                if (sfile.check(STATUS_FILE_DELETED))
                    continue;
                    
                READS(sfile.m_name)
                READ(&sfile.m_status, sfile.m_status)
                READ(&sfile.m_timestamp, sfile.m_timestamp)
                READ(&sfile.isDir, sfile.isDir)
            }
        }
        
        load_err:
        engone::FileClose(file);
    }
    void UserCache::save() {
        using namespace engone;
        auto file = engone::FileOpen(m_path, nullptr, engone::FILE_ALWAYS_CREATE);
        if(!file) {
            log::out << log::RED << "bad path to load cache\n";
            return;   
        }
        CacheHeader header{};
        header.countOfConnections = m_connections.size();
        header.countOfUnits = m_units.size();
        
        bool success = false;
        
        #define WRITE(P,T) success = engone::FileWrite(file, P, sizeof(T)); if(!success) goto save_err;
        #define WRITES(S) { u8 len = S.length(); success = engone::FileWrite(file, &len, sizeof(len)); if(len > 0) { if(!success) goto save_err; success = engone::FileWrite(file, S.c_str(), len); if(!success) goto save_err;} }
        
        WRITE(&header, header)
        
        for (int i = 0; i < m_connections.size(); i++) {
            SyncConnection* conn = m_connections[i];
            /*
                struct {
                    name
                    states
                    ip
                    port   
                }
            */
            WRITES(conn->m_name)
            
            Status tempState = conn->m_status;
            if (conn->m_status & (STATUS_CON_LINKED | STATUS_CON_FAILED | STATUS_CON_CONNECTING))
                tempState = (Status)(tempState | STATUS_CON_RECONNECT);
            tempState = (Status)(tempState & (~(STATUS_CON_LINKED | STATUS_CON_FAILED | STATUS_CON_CONNECTING)));
            
            WRITE(&tempState, tempState)
            WRITES(conn->m_ip)
            WRITES(conn->m_port)
        }
        for (int i = 0; i < m_units.size(); i++) {
            SyncUnit* unit = m_units[i];
            /*
                struct {
                    name
                    status
                    password
                    root
                    timestamp
                    filecount
                    files = struct {
                        name
                        status
                        timestamp
                        isdir
                    }
                }
            */
            WRITES(unit->m_name)
            WRITE(&unit->m_status, unit->m_status)
            WRITES(unit->m_password)
            WRITES(unit->m_root)
            WRITE(&unit->m_timestamp, unit->m_timestamp)
            
            u32 fileCount = 0;
            for (int j = 0; j < unit->m_files.size(); j++) {
                if (unit->m_files[j].check(STATUS_FILE_DELETED))
                    continue;
                fileCount++;
            }
            WRITE(&fileCount, fileCount)
            
            for (int j = 0; j < unit->m_files.size(); j++) {
                SyncFile& sfile = unit->m_files[j];
                if (sfile.check(STATUS_FILE_DELETED))
                    continue;
                    
                WRITES(sfile.m_name)
                WRITE(&sfile.m_status, sfile.m_status)
                WRITE(&sfile.m_timestamp, sfile.m_timestamp)
                WRITE(&sfile.isDir, sfile.isDir)
            }
        }
        
        save_err:
        engone::FileClose(file);
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
    #ifdef DEPRECATED
	void UserCache::load_old() {
		using namespace engone;
		FileReader file(m_path);
        if(!file.isOpen()) {
            log::out << log::RED << "bad path to load cache\n";
            return;
        }

		try {
            CacheHeader header{};
			u32 connSize;
			file.read(&connSize);
            Assert(("potential corrupted cache",connSize < 100)); // connSize is probably corrupted if more than 100
            if(connSize >= 100) {
                connSize = 0;   
            }
			for (int i = 0; i < connSize; i++) {
				SyncConnection* conn = new SyncConnection();
				// GetTracker().track(conn);
				m_connections.push_back(conn);
				file.read(&conn->m_name);
                int status = 0;
				file.read(&status);
                conn->m_status = (Status)status;
				file.read(&conn->m_ip);
				file.read(&conn->m_port);

				// disable reconnect on loading
				//conn->m_status &= ~(STATUS_CON_RECONNECT);
			}
			u32 unitSize;
			file.read(&unitSize);
            Assert(("potential corrupted cache",unitSize < 200)); // connSize is probably corrupted if more than 100
            if(unitSize >= 200) {
                unitSize = 0;
            }
			for (int i = 0; i < unitSize; i++) {
				SyncUnit* unit = new SyncUnit();
				// GetTracker().track(unit);
				m_units.push_back(unit);
				file.read(&unit->m_name);
                int status = 0;
				file.read(&status);
                unit->m_status = (Status)status;
				file.read(&unit->m_password);
				file.read(&unit->m_root);
				file.read(&unit->m_timestamp);
				int fileSize;
				file.read(&fileSize);
				for (int j = 0; j < fileSize; j++) {
					unit->m_files.push_back({});
					SyncFile& sfile = unit->m_files[j];
					file.read(&sfile.m_name);
                    int status = 0;
                    file.read(&status);
                    sfile.m_status = (Status)status;
					file.read(&sfile.m_timestamp);
					file.read(&sfile.isDir);
				}
			}
		} catch (Error err) {
			log::out << "Cache load: " << err << "\n";
		}
	}
	void UserCache::save_old() {
		using namespace engone;
		FileWriter file(m_path);
        if(!file.isOpen()) {
            log::out << log::RED << "bad path to save cache\n";
            return;
        }
		
		try {
			u32 connSize = m_connections.size();
			file.write(&connSize);
			for (int i = 0; i < m_connections.size(); i++) {
				SyncConnection* conn = m_connections[i];
				file.write(&conn->m_name);
				Status tempState = conn->m_status;
				if (conn->m_status & (STATUS_CON_LINKED | STATUS_CON_FAILED | STATUS_CON_CONNECTING))
					tempState = (Status)(tempState | STATUS_CON_RECONNECT);
				tempState = (Status)(tempState & (~(STATUS_CON_LINKED | STATUS_CON_FAILED | STATUS_CON_CONNECTING)));
				file.write(&tempState);
				file.write(&conn->m_ip);
				file.write(&conn->m_port);
			}
			u32 unitSize = m_units.size();
			file.write(&unitSize);
			for (int i = 0; i < m_units.size(); i++) {
				SyncUnit* unit = m_units[i];
				file.write(&unit->m_name);
				file.write(&unit->m_status);
				file.write(&unit->m_password);
				file.write(&unit->m_root);
				file.write(&unit->m_timestamp);
				u32 fileSize = 0;
				for (int j = 0; j < unit->m_files.size(); j++) {
					if (unit->m_files[j].check(STATUS_FILE_DELETED))
						continue;
					fileSize++;
				}
				file.write(&fileSize);
				for (int j = 0; j < unit->m_files.size(); j++) {
					SyncFile& sfile = unit->m_files[j];
					if (sfile.check(STATUS_FILE_DELETED))
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
    #endif
	
}