#pragma once

#include "UniSync/Data/SyncPart.h"
#include "Engone/Util/Utilities.h"

namespace unisync {
	class SyncFile : public SyncPart {
	public:
		SyncFile() = default;
		SyncFile(const std::string& name, bool isDir, u64 timestamp) : SyncPart(name), isDir(isDir), m_timestamp(timestamp)
			//, m_lastModified(lastModified) 
		{}

		bool isDir = false;
		// in milliseconds
		u64 m_timestamp;
		//u64 m_lastModified;

		// 0 means they are equal. -1, 1 means they are different.
		int compare(SyncFile& file);
	};
	class SyncUnit : public SyncPart {
	public:
		SyncUnit() = default;
		SyncUnit(const std::string& name) : SyncPart(name) {};
		// will cleanup it's refresh thread.
		~SyncUnit() { cleanup(); }

		std::string m_password;
		u64 m_timestamp;
		std::string m_root; // should it be kept behind public functions?
		std::string m_oldRoot;
		std::vector<SyncFile> m_files;
		
		void lock() { m_fileMutex.lock(); }
		void unlock() { m_fileMutex.unlock(); }
		std::mutex m_fileMutex; // a better name would be just m_mutex since it is a general mutex for the whole unit(all files and the root).

		bool m_wasRefreshed = false; // used to syncrhonize units in update

		// starts a FileMonitor based on root path
		// will do nothing if monitor already is started
		// this should be updated if root changes
		void startMonitor();
		engone::FileMonitor m_fileMonitor;

		// in milliseconds
		u64 getTime(const std::string& fullpath);
		bool validRoot();

		// you may want to lock the mutex when doing this.
		void sort();
		// you may want to lock mutex before calling this.
		void insertFile(SyncFile file);
		// isDir is needed because a file and directory with the same name can exist.
		// the returned value is valid untill you change m_files. (the vector may have moved it's memory)
		SyncFile* getFile(const std::string& fileName, bool isDir);

		// will update SyncFiles in the unit from the current root. (whether the root was switched is not relevant)
		// will also sort files. 
		// returns true if any file was updated
		bool refresh();


		//HANDLE refreshHandle=NULL;
		// do NOT lock mutex on this. If you do there is a chance of the thread getting stuck.
		//void autoRefresh();

		void cleanup();
	};
}