#include "UniSync/Data/SyncUnit.h"

#include "Engone/Logger.h"
#include "Engone/Utilities/Utilities.h"

namespace unisync {
	engone::TrackerId SyncUnit::trackerId="SyncUnit";

	int SyncFile::compare(SyncFile& file) {
		int len = m_name.length();
		int result=-1;
		if (len > file.m_name.length()) {
			len = file.m_name.length();
			result = 1;
		}

		for (int i = 0; i < len; i++) {
			char a = m_name[i], b = file.m_name[i];
			if (a == b) {
				if (i == len-1) {
					//if (m_name.length() == file.m_name.length()) {
					//	if (isDir == file.isDir) {
					//		result = 0;
					//		break;
					//	} else if (isDir) {
					//		result = -1;
					//		break;
					//	} else if (file.isDir) {
					//		result = 1;
					//		break;
					//	}
					//} else {
					if (isDir == file.isDir) {
						//result = 0;
						break;
					} else if (isDir) {
						result = -1;
						break;
					} else if (file.isDir) {
						result = 1;
						break;
					}
					//}
				} else {
					continue;
				}
			} else if (a == '\\') {
				result = -1;
				break;
			} else if (b == '\\'){
				result = 1;
				break;
			} else if (a < b) {
				result = -1;
				break;
			} else {
				result = 1;
				break;
			}
		}
		return result;
	}
	bool SyncUnit::refresh() {
		using namespace engone;
		lock();
		int fileCount = m_files.size();
		std::vector<bool> foundFiles(fileCount);
		bool refreshed = false;
		// ISSUE: maybe sanitize away .. when reading files, root too?

		if (!std::filesystem::exists(m_root)) {
			log::out << log::RED << "SyncUnit - Invalid root " << m_root << "\n";
			unlock();
			return false;
		}

		try {
			for (const auto& dirEntry : std::filesystem::recursive_directory_iterator(m_root)) {
				std::string str = dirEntry.path().string().substr(m_root.length());
				uint64_t timestamp = std::chrono::duration_cast<std::chrono::milliseconds>(dirEntry.last_write_time().time_since_epoch()).count();
				//log::out << str << "\n";
				bool found = false;
				for (int i = 0; i < fileCount; i++) {
					SyncFile& file = m_files[i];
					if (str == file.m_name&&dirEntry.is_directory()==file.isDir) { // same file
						foundFiles[i] = true;
						found = true;

						if (timestamp != file.m_timestamp) {
							// Update file
							file.m_timestamp = timestamp;
							file.set(StateFileNew);
							refreshed = true;
						}
						break;
					}
				}
				if (!found) {
					// New file
					SyncFile syncFile = { str, dirEntry.is_directory(), timestamp };
					syncFile.set(StateFileNew);
					m_files.push_back(syncFile); // files will be sorted later
					//insertFile(syncFile);// cannot use this because it would interfere with variable foundFiles.
					refreshed = true;
				}
			}
		} catch (std::filesystem::filesystem_error err) {
			log::out << err.what() << "\n";
		}

		for (int i = foundFiles.size() - 1; i >= 0; i--) {
			if (!foundFiles[i]) {
				m_files[i].m_status |= StateFileDeleted;
				//m_files.erase(m_files.begin() + i);
				refreshed = true;
			}
		}

		sort();
		unlock();
		return refreshed;
	}
	//void SyncUnit::autoRefresh() {
	//	using namespace engone;
	//	if (refreshHandle != NULL) {
	//		FindCloseChangeNotification(refreshHandle);
	//		refreshHandle = NULL;
	//	}
	//	if (m_refreshThread.joinable())
	//		m_refreshThread.join();
	//	lock();
	//	if (!m_refreshThread.joinable()) { // this is here so that you don't call this function twice at the same time and create two threads at the same time.
	//		if (!std::filesystem::exists(m_root)) {
	//			log::out << log::RED << "Cannot auto refresh non existing dir\n";
	//		} else {
	//			std::string checkPath = m_root;
	//			refreshHandle = FindFirstChangeNotification(checkPath.c_str(), true, FILE_NOTIFY_CHANGE_LAST_WRITE);
	//			if (refreshHandle == INVALID_HANDLE_VALUE || refreshHandle == NULL) {
	//				refreshHandle = NULL;
	//				log::out << log::RED << "SynUnit - bad handle\n";
	//			} else {
	//				m_refreshThread = std::thread([this, checkPath]() {
	//					//std::string path = m_root;
	//					//log::out << "SyncUnit - start refresh thread\n";
	//					DWORD waitStatus;
	//					while (true) {
	//						waitStatus = WaitForMultipleObjects(1, &refreshHandle, false, INFINITE);
	//						//log::out.TIME() << " update dir\n";
	//						// wrong path is being checked.
	//						if (checkPath != m_root)
	//							break;
	//						if (waitStatus == WAIT_OBJECT_0 && refreshHandle != NULL) {
	//							if (m_preventRefreshDepth==0) {
	//								//log::out << "REFRESHING\n";
	//								if (refresh())
	//									m_wasRefreshed = true;
	//							} else {
	//								//log::out << "Prevented Refresh " << m_preventRefreshDepth << "\n";
	//							}
	//							if (!FindNextChangeNotification(refreshHandle)) {
	//								//log::out << log::RED << "SyncUnit - bad handle 2\n";
	//								break;
	//							}
	//						} else {
	//							break;
	//						}
	//					}
	//					//log::out << "SyncUnit - Stop refresh thread\n";
	//				});
	//			}
	//		}
	//	}
	//	unlock();
	//}
	void SyncUnit::startMonitor() {
		using namespace engone;
		if (validRoot()) {
			m_fileMonitor.check(m_root, [this](const std::string& path, int type) {
				log::out << "SyncUnit refresh " << path <<" "<<type << "\n";
				// should do something better here probably
				if (refresh())
					m_wasRefreshed = true;
			}, engone::FileMonitor::WATCH_SUBTREE);
		}
	}
	void SyncUnit::insertFile(SyncFile file) {
		bool inserted = false;
		for (int i = 0; i < m_files.size(); i++) {
			if (m_files[i].compare(file) > 0) {
				inserted = true;
				m_files.insert(m_files.begin() + i, file);
				break;
			}
		}
		if (!inserted)
			m_files.push_back(file);
	}
	SyncFile* SyncUnit::getFile(const std::string& fileName, bool isDir) {
		for (int i = 0; i < m_files.size(); i++) {
			SyncFile& file = m_files[i];
			if (file.m_name == fileName && file.isDir == isDir)
				return &m_files[i];
		}
		return nullptr;
	}
	// this functions has no mutex stuff
	void SyncUnit::sort() {
		using namespace engone;
		//log::out << "START SORTING ###############\n";
		int count = m_files.size();
		int* fileIndex = (int*)alloc::malloc(sizeof(int) * count); // no need to track since it will be freed later
		for (int i = 0; i < count; i++) {
			fileIndex[i] = i;
			//log::out << m_files[i].m_name << "\n";
		}
		// classic bubble sort
		for (int i = 0; i < count; i++) {
			bool moved = false;
			for (int j = 1; j < count; j++) { // can be improved
				SyncFile& a = m_files[fileIndex[j - 1]];
				SyncFile& b = m_files[fileIndex[j]];

				if (a.compare(b) > 0) {
					int temp = fileIndex[j - 1];
					fileIndex[j - 1] = fileIndex[j];
					fileIndex[j] = temp;
					moved = true;
					//log::out << a.m_name<<" > "<<b.m_name<<  "\n";
				} else {
					//log::out << a.m_name << " < " << b.m_name << "\n";
				}
			}
			if (!moved) break;
		}
		//log::out << "--------------\n";
		// easier
		SyncFile* temp = (SyncFile*)alloc::malloc(sizeof(SyncFile)*count);
		std::memcpy(temp, m_files.data(), sizeof(SyncFile) * count);
		for (int i = 0; i < count; i++) {
			std::memcpy(m_files.data() + i, temp+fileIndex[i], sizeof(SyncFile));
			//log::out << m_files[i].m_name << "\n";
		}
		alloc::free(temp, sizeof(SyncFile) * count);
		alloc::free(fileIndex, sizeof(int) * count);
	}
	uint64_t SyncUnit::getTime(const std::string& fullpath) {
		if (std::filesystem::exists(fullpath))
			return std::chrono::duration_cast<std::chrono::milliseconds>(std::filesystem::last_write_time(fullpath).time_since_epoch()).count();
		engone::log::out << engone::log::RED << "getTime - path not valid\n";
		return 0;
	}
	bool SyncUnit::validRoot() {
		return std::filesystem::exists(m_root);
	}
	void SyncUnit::cleanup() {
		m_fileMonitor.cleanup(); // good to clear up now so that it doesn't mess with variables later
		//if (refreshHandle != NULL) {
		//	FindCloseChangeNotification(refreshHandle);
		//	refreshHandle = NULL;
		//}
		//if (m_refreshThread.joinable())
		//	m_refreshThread.join();
		lock();
		m_files.clear();
		unlock();
	}
}