#pragma once

#include "Engone/PlatformLayer.h"
#include "Engone/Util/Alloc.h"
#include "Engone/Error.h"
#include "Engone/Asserts.h"

#include <string>

namespace engone {

	// How to deal with buffers? I believe Windows does some buffering.
	// The reader is currently allocating a buffer for the whole file
	// and reading everything at once. You may want to do something different for larger files.
	class FileReader {
	public:
		FileReader(const std::string& path, bool binaryForm = false);
		~FileReader();
		// Resets everything and makes the class as good as new
		void cleanup();
		// Closes the platforms file handle. You can still read data if it is available in the buffer.
		void close();

		FileReader(const FileReader&) = delete;
		FileReader& operator=(const FileReader&) = delete;

		inline bool isOpen() const {return m_error == NoError;}
		inline bool operator!() const {return !isOpen();}
		inline operator bool() const {return isOpen();}
		inline uint32_t size() const {return m_fileSize;}

		// Data in buffer can still be read if file is closed.

		inline const std::string& getPath() const { return m_path; }
		inline Error getError() const { return m_error; }

		// Note: read functions that returns a bool and that being false has failed.
		//		 (bool)a bool and fails if the  returns a of how many bytes/numbers/characters where read.
		//		If it isn't equals to 

		// General read types
		// @return True if successful. getError() to see last error if false is returned.
		bool read(void* ptr, u64 bytes);
		
		// Used with non-binary form
		// @return Characters read.
		u64 readLine(std::string& line);
		// @return Numbers read. Probably EndOfLine error if it doesn't match count parameter.
		u64 readNumbers(char* ptr, u64 count, u32 typeSize, bool isFloat);
		
		// Specific read types
		// Note that char* will use void* and not these number functions
		bool read(u8* ptr, u64 count=1);
		bool read(i16* ptr, u64 count=1);
		bool read(u16* ptr, u64 count=1);
		bool read(i32* ptr, u64 count=1);
		bool read(u32* ptr, u64 count=1);
		bool read(i64* ptr, u64 count=1);
		bool read(u64* ptr, u64 count=1);
		bool read(bool* ptr, u64 count=1);
		bool read(float* ptr, u64 count=1);
		bool read(double* ptr, u64 count=1);
		bool read(std::string* ptr, u64 count=1);

		// bool read(glm::vec3* ptr, u64 count=1);
		// bool read(glm::mat4* ptr, u64 count=1);

		bool readAll(std::string& lines);
		
		//Todo: glm mat4

		static const u64 BYTES_PER_READ = 4096; // Todo: Increase/Decrease this?
	private:
		bool binaryForm = false;

		std::string m_path;
		APIFile m_file;
		u64 m_fileSize = 0;
		u64 m_fileHead = 0;
		Error m_error = NoError;
		
		// m_error should be set where it occurs to get the most detailed error.
		// readNumbers calls readLine which returns false if something bad happened.
		// readNumbers should NOT set error because it's not sure what happened.
		// The code in readLine should set the error because it knows what happened.
		
		u64 m_bufferHead=0;
		Memory m_buffer{ 1,ALLOC_TYPE_HEAP };
	};
}