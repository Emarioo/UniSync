#include "Engone/Util/Alloc.h"

// #include "Engone/Logger.h"

#include "Engone/PlatformLayer.h"
// #include "Engone/PlatformModule/GameMemory.h"
// #include <mutex>

namespace engone {
	namespace alloc {
		// Todo: Thread safety for alloc
		// static std::mutex s_mutex;
		// static u64 s_allocatedBytes=0;

		void* malloc(u64 size) {
			return Allocate(size);
			// if (size == 0) return nullptr;
			// void* ptr = std::malloc(size);
			// if (ptr) {
			// 	// s_mutex.lock();
			// 	s_allocatedBytes += size;
			// 	// s_mutex.unlock();
			// 	//log::out << "Allocate: " << size << "("<<s_allocatedBytes<<")\n";
			// }
			// return ptr;
		}
		void* realloc(void* ptr, u64 oldSize, u64 newSize) {
			return Reallocate(ptr,oldSize,newSize);
			// if (newSize == 0) {
			// 	Free(ptr, oldSize);
			// 	return nullptr;
			// }
			// void* newPtr = std::realloc(ptr, newSize);
			// if (newPtr) {
			// 	// s_mutex.lock();
			// 	s_allocatedBytes -= oldSize;
			// 	s_allocatedBytes += newSize;
			// 	// s_mutex.unlock();
			// 	return newPtr;
			// } else {
			// 	return ptr;
			// }
		}
		void free(void* ptr, u64 size) {
			Free(ptr,size);
			// if (!ptr) return;
			// // s_mutex.lock();
			// s_allocatedBytes -= size;
			// // s_mutex.unlock();
			// std::free(ptr);
		}
		u64 allocatedBytes() {
			return GetAllocatedBytes();
			// return s_allocatedBytes; // reading doesn't really require a mutex
		}
	}
	bool Memory::resize(u64 count){
		if (count == 0) {
			if (data) {
				if (m_allocType == ALLOC_TYPE_HEAP) engone::Free(data, max * m_typeSize);
				// else if (m_allocType == ALLOC_TYPE_GAME_MEMORY) GetGameMemory()->free(data);
			}
			data = nullptr;
			max = 0;
			used = 0;
		}else if (!data) {
			if (m_allocType == ALLOC_TYPE_HEAP) data = engone::Allocate(count * m_typeSize);
			// else if (m_allocType == ALLOC_TYPE_GAME_MEMORY) data = GetGameMemory()->allocate(count * m_typeSize);
			if (data) {
				max = count;
				used = 0;
			}
		} else {
			void* newData = 0;
			if (m_allocType == ALLOC_TYPE_HEAP) newData = engone::Reallocate(data, max * m_typeSize, count * m_typeSize);
			// else if (m_allocType == ALLOC_TYPE_GAME_MEMORY) newData = GetGameMemory()->reallocate(data, count * m_typeSize);
			if (newData) {
				data = newData;
				max = count;
				if (max < used)
					used = max;
			}
		}
		//printf("Resize max: %d count: %d\n", max,count);
		return max == count; // returns true when intention was successful
	}
	void* MemoryResize(void* ptr, u32 oldBytes, u32 newBytes, u32 allocType) {
		if (newBytes == 0) {
			if (ptr) {
				if (allocType==ALLOC_TYPE_HEAP) engone::Free(ptr,oldBytes);
				// else if (allocType==ALLOC_TYPE_GAME_MEMORY) GetGameMemory()->free(ptr);
			}
		}else if (!ptr) {
			if (allocType == ALLOC_TYPE_HEAP) ptr = engone::Allocate(newBytes);
			// else if (allocType == ALLOC_TYPE_GAME_MEMORY) ptr = GetGameMemory()->allocate(newBytes);
		} else {
			void* newData = 0;
			if (allocType == ALLOC_TYPE_HEAP) newData = engone::Reallocate(ptr,oldBytes,newBytes);
			// else if (allocType == ALLOC_TYPE_GAME_MEMORY) newData = GetGameMemory()->reallocate(ptr,newBytes);
			if (newData) {
				ptr = newData;
			}
		}
		return ptr;
	}
	// template<>
	// bool Memory<char>::resize(u64 count, u64 size) {
	// 	if (count == 0) {
	// 		if (data)
	// 			Free(data, max * size);
	// 		data = nullptr;
	// 		max = 0;
	// 		used = 0;
	// 	}
	// 	if (!data) {
	// 		data = (char*)Allocate(count * size);
	// 		if (data) {
	// 			max = count;
	// 			used = 0;
	// 		}
	// 	} else {
	// 		char* newData = (char*)Reallocate(data, max * size, count * size);
	// 		if (newData) {
	// 			data = newData;
	// 			max = count;
	// 			if (max < used)
	// 				used = max;
	// 		}
	// 	}
	// 	//printf("Resize max: %d count: %d\n", max,count);
	// 	return max == count; // returns true when intention was successful
	// }
}