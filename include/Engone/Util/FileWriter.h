#pragma once

#include "Engone/Error.h"
#include "Engone/PlatformLayer.h"
#include "Engone/Asserts.h"

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
		bool write(const void* ptr, u64 bytes);
		
		u64 writeNumbers(const char* ptr, u64 count, u32 typeSize, bool isFloat);
		
		// Specific read types
		bool write(const u8* ptr, u64 count=1);
		bool write(const i16* ptr, u64 count=1);
		bool write(const u16* ptr, u64 count=1);
		bool write(const int* ptr, u64 count=1);
		bool write(const u32* ptr, u64 count=1);
		bool write(const i64* ptr, u64 count=1);
		bool write(const u64* ptr, u64 count=1);
		bool write(const bool* ptr, u64 count=1);
		bool write(const float* ptr, u64 count=1);
		bool write(const double* ptr, u64 count=1);
		bool write(const std::string* ptr, u64 count=1);
	
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
		APIFile m_file = {};
		u64 m_fileHead = 0;
		std::string m_path;
		Error m_error = NoError;

	};   
}