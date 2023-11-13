#pragma once

#include "Engone/Engone.h"
#include "Engone/Networking/NetworkModule.h"

#include "UniSync/Keybindings.h"

#include "UniSync/Data/UserCache.h"

namespace unisync {
	/* Design
	Features:
		see files in a unit, their tags, file size, directory or not. one unit at a time
		create remove units, connections.


	*/

	// The timestamps of files are only used to check if a file on disk is new compared to the cache. (All files are new with an empty cache)
	// When synchronizing files, New, Override, Collision is used to determine which files should be sent where.
	// ISSUE: timestamps aren't used properly. determining synchronization with New tag may cause new files to be deleted by old files if a message didn't reach the receiver.
	//  because the sender assumes it was received. Check things with timestamps instead. Simular to how I did it with the Launcher.
	//  Each file has two timestamps, one for the other end, one for my end. Instead of checking which one is earlier just check if
	//  The timestamp i knew you had last time(last connection/sync) is different from now. If so sync is needed. If my file is different from yours, then a collision occured.
	//  While reworking timestamp also fix tags so that files apply their tags on parent directories
	// 
	// ISSUE: a lot of issues
	// seeing a file's size would be nice
	// a different color if you edit a text
	// root can be larger than window, if so trim text starting from the left untill the last character of the text is visible. Also add ... in beginning.
	// most of the code isn't very efficient, a lot of for loops, bubble sort, this is fine as long as the cpu isn't high
	// GPU is insanely high with two applications which is fine, you are not supposed to have two. But it is still not optimal with one.
	// This is probably due to how this app is treated more like a game in how things are rendered and updated. Option in engine for event based rendering?
	
	// ISSUE: All files are sent when checking if any need to be synchronized. An improvement would be only sending files that where updated this is kind of difficult though.
	//	   Hard to know when who synchronized what with who.
	// ISSUE: A safety feature which only allows a certain amount of data sent per second. This way you can't overload the app.
	// ISSUE: if cache has files which don't exist then the order of the file list seems to render poorly. Sorting seems to not work(but i am pretty sure the sorting is fine so it is something else).
	// ISSUE: If you delete a unit while the network stuff writes to a file in it. A crash will occur.
	// ISSUE: Cache seems to need a new line at the end. This doesn't make sense and can easily be fixed.
	// ISSUE: change FileReader to binary form in UserCache
	// ISSUE: SyncApp.cpp is rather big. Splitting up the User interface and the networking code would be good.
	// ISSUE: there seems to be a bug where recv in NetworkModule receives bad data. The header is something like 9591258991 which isn't the correct amount of bytes that got sent.
	//		Recv and send functions run on one thread each so there is no interference between multiple send functions. (supposedly)
	
	// FEATURE: Cache should be auto-saved now and then.
	// FEATURE: revamp of the ui design, ux design. consider gpu, cpu, timestamps, tags and when to reconnect, relocate. A lot to do but i don't want a poorly app. I am better than that.
	// Maybe put all issues and features in one spot. probably in this file.
	// FEATURE: Feedback for the user so they know files have been synchronized. A popup notification somewhere would do.(using new improved UI design)
	// FEATURE: If you don't interact with the app for a while(a few seconds), it synchronizes if you made changes like overriding. This way you don't need to do it yourself.
	// FEATURE: Make the arrows bigger in the icon. They are barely visible
	// FEATURE: Freeze/Ignore directories and files
	// FEATURE: Auto reconnect after a few seconds so that you have time to turn of the reconnection.

	class SyncApp : public engone::Application {
	public:
		enum SyncOption : int {
			AutoRefresh=1,
		};
		typedef int SyncOptions;
		SyncOptions options=0;

		SyncApp(engone::Engone* engone, const std::string& cachePath);
		~SyncApp();

		// command you call
		void synchronizeUnits();

		// calls relocate when received.
		void recSyncUnits(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid);
		void recSyncFiles(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid);
		void recReqFiles(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid);
		void recNotifyFiles(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid);
		// Will change state of file, New, Override, Collision will be disabled, timestamp updated.
		void sendFile(SyncUnit* unit, SyncFile& file, engone::Sender* sender, engone::UUID uuid);
		void recSendFile(engone::MessageBuffer& buf, engone::Sender* sender, engone::UUID uuid);

		void reconnect();
		// refresh files on disc
		// check if path contains .. if it does, the program could alter files above the root directory. That would be a security issue.
		// This only need to be done before writing file
		//bool notSanitized(const std::string& path);
		void refreshUnits();
		bool mainAction();
		void linkConnection(SyncConnection* conn);
		void renderText(const std::string& text);
		void renderText(const std::string& first, const std::string& second);
		void editAction(std::string& str);
		void openAction(SyncPart* part);
		void renderPart(SyncPart* part);
		void renderState(const std::string& text, SyncStates state);

		void update(engone::LoopInfo& info) override;
		void render(engone::LoopInfo& info) override;
		void onClose(engone::Window* window) override;

		void pendSynchronize();

		void cleanup();
	public:
		UserCache userCache;
		engone::Window* m_window=nullptr;
		engone::FontAsset* consolas=nullptr;
		
		bool m_shouldSynchronize=false;

		struct FileDownload {
			SyncUnit* unit;
			std::string fileName; // not full path
			engone::FileWriter* writer=nullptr;
		};
		std::unordered_map<engone::UUID, FileDownload> fileDownloads;

		//engone::DelayCode sortUnits;
		//bool activeSortList=false;
		//std::vector<SyncUnit*> unitsToSort[2];

		//-- Connection stuff
		SyncConnection* serverConn = nullptr, * waitingServer = nullptr;
		SyncConnection* clientConn = nullptr, * waitingClient = nullptr;
		engone::Server* server = nullptr;
		engone::Client* client = nullptr;

		//float chickenY = 0;

		//-- Interface stuff
		int line = 0, indent = 0, selectedLine = 0;
		const int indentSize = 10, lineHeight = 20;
		int editingIndex = 0;
		bool editingText = false;
		int scrollY = 0;
		float upCooldown = 0, downCooldown = 0;

		bool hadFocus = false;// relocate  stuff
	};
}