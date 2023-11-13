#pragma once

#include "Engone/LogModule/LogTypes.h"

#include "Engone/PlatformModule/PlatformLayer.h"

#ifdef ENGONE_PHYSICS
#include "Engone/Utilities/rp3d.h"
#endif

#include "Engone/Utilities/Alloc.h"

#include <unordered_map>
#include <vector>

namespace engone {
	
	class Logger {
	public:
		Logger() = default;
		~Logger() { cleanup(); }
		void cleanup();

		Logger(const Logger&) = delete;
		Logger& operator =(const Logger&) = delete;

		// master report is always written to
		void enableReport(bool yes = true);
		void enableConsole(bool yes = true);
		// individual for each thread
		void setMasterColor(log::Color color);
		// all threads write to this report
		void setMasterReport(const std::string path);
		// extra report, each thread, set empty path for no report which is default.
		void setReport(const std::string path);

		// extra report which is individual for each thread
		void useThreadReports(bool yes);
		
		void flush();

		// Returns master color if it is set otherwise normal color.
		log::Color getColor();
		Logger& operator<<(log::Color value);
		Logger& operator<<(log::Filter value);

		Logger& operator<<(const char* value);
		Logger& operator<<(char* value); // treated as a string
		Logger& operator<<(char value);
		Logger& operator<<(const std::string& value);

		Logger& operator<<(void* value);
		Logger& operator<<(int64 value);
		Logger& operator<<(int32 value);
		Logger& operator<<(int16 value);
		//Logger& operator<<(int8 value); // interpreted as a character not a number.
		
		Logger& operator<<(uint64 value);
		Logger& operator<<(uint32 value);
		Logger& operator<<(uint16 value);
		Logger& operator<<(uint8 value);

		Logger& operator<<(double value);
		Logger& operator<<(float value);

#ifdef ENGONE_GLM
		Logger& operator<<(const glm::vec3& value);
		Logger& operator<<(const glm::vec4& value);
		Logger& operator<<(const glm::quat& value);
		Logger& operator<<(const glm::mat4& value);
#endif
#ifdef ENGONE_PHYSICS
		Logger& operator<<(const rp3d::Vector3& value);
		Logger& operator<<(const rp3d::Quaternion& value);
#endif

		void print(char* str, int length);
		
		Logger& TIME();

		void setRealLogger(engone::Logger* logger);

	private:
		Mutex m_printMutex;
		
		struct ThreadInfo {
			log::Color color = log::SILVER;
			log::Filter filter = log::FilterAll;

			std::string logReport;

			Memory lineBuffer{1,ALLOC_TYPE_HEAP};

			// ensure free space
			char* ensure(uint32 bytes);
			// use some of ensured space
			void use(uint32 bytes);
		};
		ThreadInfo& getThreadInfo();

		log::Color m_masterColor = log::NO_COLOR;
		std::string m_masterReportPath="master.txt";
		bool m_useThreadReports = false;
		bool m_enabledConsole = true;
		bool m_enabledReports = true;

		std::string m_rootDirectory = "logs";

		std::unordered_map<std::string, APIFile*> m_logFiles;
		
		std::unordered_map<ThreadId, ThreadInfo> m_threadInfos;
		
		Logger* realLogger=0;
	};
	
	namespace log {
		extern Logger out;
	}
}