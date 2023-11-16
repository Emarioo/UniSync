#pragma once

#include "UniSync/Data/SyncConnection.h"
#include "UniSync/Data/SyncUnit.h"

namespace unisync {
	class UserCache {
	public:
		UserCache() = default;
		UserCache(const std::string& path);
		~UserCache();

		//-- Maintanence functions
		
		// will set path of cache and load data
		void load(const std::string& path);
		// the current path will be loaded
		void load();
		// saves data to path
		void save();
		// will delete all data. The path will remain
		void clean();

		void load_old();
		void save_old();
        
		//-- Data access

		const std::string& getPath() const { return m_path; }

		// frozen or units with invalid root are ignored
		SyncUnit* getUnit(const std::string& name);
		std::vector<SyncConnection*>& getConnections() { return m_connections; }
		std::vector<SyncUnit*>& getUnits() { return m_units; }

	private:
		std::string m_path;

		std::vector<SyncConnection*> m_connections;
		std::vector<SyncUnit*> m_units;

	};
    
    struct CacheHeader {
        char magic[4] = {'U','S','C','B'};
        // static char HUMAN_MAGIC[4] = {'U','S','C','H'};
        u32 reserved_version;
        u32 reserved_checksum; // or file size?
        u16 countOfConnections;
        u16 countOfUnits;
    };
}