#pragma once

#include "Engone/Error.h"
#include "Engone/PlatformModule/PlatformLayer.h"

namespace engone{
	class FileWriter {
	public:
		FileWriter(const std::string& path, bool binaryForm = false);
		~FileWriter();
		void cleanup();
		void close();
		
		FileWriter(const FileWriter&) = delete;
		FileWriter& operator=(const FileWriter&) = delete;

		inline bool isOpen() const {return m_error == NoError;}
		inline bool operator!() const {return !isOpen();}
		inline operator bool() const {return isOpen();}
		
		inline const std::string& getPath() const { return m_path; }
		inline Error getError() const { return m_error; }
		
		// General read types
		// @return True if successful. getError() to see last error if false is returned.
		bool write(const void* ptr, uint64 bytes);
		
		uint64 writeNumbers(const char* ptr, uint64 count, uint typeSize, bool isFloat);
		
		// Specific read types
		bool write(const uint8* ptr, uint64 count=1);
		bool write(const int16* ptr, uint64 count=1);
		bool write(const uint16* ptr, uint64 count=1);
		bool write(const int* ptr, uint64 count=1);
		bool write(const uint* ptr, uint64 count=1);
		bool write(const int64* ptr, uint64 count=1);
		bool write(const uint64* ptr, uint64 count=1);
		bool write(const float* ptr, uint64 count=1);
		bool write(const double* ptr, uint64 count=1);
		bool write(const std::string* ptr, uint64 count=1);
	
		// inline void write(const glm::vec3* var, uint32_t count = 1) {
		// 	write((const float*)var, count * 3);
		// }
		// inline void write(const glm::mat4* var, uint32_t count = 1) {
		// 	write((const float*)var, count * 16);
		// }

		void writeComment(const std::string& str);
		// static TrackerId trackerId;

	private:
		bool binaryForm = false;
		APIFile* m_file = 0;
		uint64 m_fileHead = 0;
		std::string m_path;
		Error m_error = NoError;

	};   
}