#pragma once

// replacement for std malloc, realloc and free.
// useful if you want to debug, change or track something.

// Not that with ALLOC_DELETE you cannot use a class with a namespace.
// 'ALLOC_DELETE(rp3d::PhysicsCommon,common)' will give 'common->rp3d::PhysicsCommon' which isn't allowed.

// #include <cstdlib>
#include <stdlib.h>

#include "Engone/PlatformLayer.h"

#define ALLOC_NEW(CLASS) new((CLASS*)engone::Allocate(sizeof(CLASS))) CLASS
#define ALLOC_DELETE(CLASS,VAR) {VAR->~CLASS();engone::Free(VAR,sizeof(CLASS));}

#define ALLOC_TYPE_HEAP 0
#define ALLOC_TYPE_GAME_MEMORY 1

namespace engone {
	
	// Todo: MemoryAllocator for game state memory
	
	// namespace alloc {
	// 	void* malloc(u64 size);
	// 	// oldSize is used for tracking. you usually keep it somewhere.
	// 	void* realloc(void* ptr, u64 oldSize, u64 newSize);
	// 	// size is used for tracking. you usually keep it somewhere.
	// 	void free(void* ptr, u64 size);
	// 	u64 allocatedBytes();
	// }
	struct Memory {
		// @param allocType ALLOC_TYPE_HEAP or ALLOC_TYPE_GAME_MEMORY
		Memory(u32 typeSize, u32 allocType) : m_typeSize(typeSize), m_allocType(allocType) {}

		u64 max = 0;
		u64 used = 0; // may be useful to you.
		void* data = nullptr;

		// count is not in bytes.
		// function is only defined for Memory<char>
		// Rename to reserve?
		bool resize(u64 count);

		inline u32 getTypeSize() { return m_typeSize; }
		inline u32 getAllocType() { return m_allocType; }
	private:
		u32 m_typeSize = 0;
		u32 m_allocType = 0;
	};
	// Funct is not tested!
	void* MemoryResize(void* ptr, u32 bytes, u32 allocType);
	// Does not have a destructor. You need to free the memory with resize(0)
	// template<class T>
	// struct Memory {
	// 	u64 max = 0;
	// 	u64 used = 0; // may be useful to you.
	// 	T* data = nullptr;

	// 	// count is not in bytes.
	// 	bool resize(u64 count) {
	// 		return ((Memory<char>*)this)->resize(sizeof(T), count);
	// 	}
	// 	// function is only defined for Memory<char>
	// 	bool resize(u64 count, u64 resize);
	// };
}