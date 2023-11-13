#pragma once

#include "Engone/Utilities/Alloc.h"

#include "Engone/PlatformModule/PlatformLayer.h"

namespace engone {
	// General allocator methods for all types
	// Also has some allocations stats. Some stats may not be 100% accurate if you use the methods in wierd ways.
	// Like deallocating a ptr with 0 in size. But for the most part it is very good.
	class Allocator {
	public:
		Allocator() = default;

		// Returns nullptr if failed. If size is 0, nullptr is returned.
		virtual char* allocate(uint32_t size) = 0;
		template <class T>
		inline T* allocate() { return (T*)allocate(sizeof(T)); };
		// Returns the input ptr if failed. Previous memory is still vaid, i think...
		// If ptr is nullptr or oldSize is 0 then allocate will be called instead.
		// if newSize is 0, deallocate will be called.
		// Not sure what happens if ptr is a random invalid value.
		virtual char* reallocate(char* ptr, uint32_t oldSize, uint32_t newSize) = 0;
		// Not sure what happens if ptr is invalid. If ptr or size is nullptr or 0 nothing happens.
		virtual void deallocate(char* ptr, uint32_t size) = 0;
		template<class T>
		inline void deallocate(T* ptr) { deallocate(ptr,sizeof(T)); };
		
	protected:
		// the allocations you wished for.
		uint32_t dreamAllocation = 0;
		// the actual allocations.
		uint32_t realAllocation = 0;
		// Total allocation is kind of deprecated. Not sure what to do with it.
		// wished allocations without deallocations. Reallocation is a bit floaty, look at implementation to se how this is counted
		uint32_t totalAllocation = 0;
	};
	// Slower but efficient allocations. Allocations are individual which means you need to know the pointers to free them.
	// realAllocations = dreamAllocations
	class HeapAllocator : public Allocator {
	public:
		HeapAllocator() = default;

		// See Allocator's allocate
		virtual char* allocate(uint32_t size) override {
			if (size == 0) return nullptr;
			char* ptr = (char*)Allocate(size);
			if (ptr) {
				realAllocation += size;
				dreamAllocation += size;
				totalAllocation += size;
			}
			return ptr;
		}
		template <class T>
		inline T* allocate() { return (T*)allocate(sizeof(T)); };
		// See Allocator's reallocate.
		// Is previous memory safe if realloc fails?
		virtual char* reallocate(char* ptr, uint32_t oldSize, uint32_t newSize) override {
			if (!ptr || oldSize == 0) return allocate(newSize);
			if (newSize == 0) {
				deallocate(ptr, oldSize);
				return nullptr;
			}
			char* newPtr = (char*)Reallocate(ptr, oldSize, newSize);
			if (newPtr) {
				realAllocation -= oldSize;
				realAllocation += newSize;
				dreamAllocation -= oldSize;
				dreamAllocation += newSize;
				totalAllocation += newSize; // +newSize
				return newPtr;
			} else {
				return ptr;
			}
		}
		// See Allocator's deallocate
		virtual void deallocate(char* ptr, uint32_t size) override {
			if (!ptr || size == 0) return;
			realAllocation -= size;
			dreamAllocation -= size;
			Free(ptr,size);
		}
		template<class T>
		inline void deallocate(T* ptr) { deallocate(ptr, sizeof(T)); };
	};
	// Stackwise allocations. Fast but unefficient allocations. You can free all memory at once. See clear and reset.
	// realAllocation is never decreased.
	// realAllocation is not counted if you are using memory from elsewhere.
	class StackAllocator : public Allocator {
	public:
		// Uses malloc
		StackAllocator(int virtualSize, bool canResize = true) : m_canResize(canResize) { init(virtualSize); }
		// Memory you allocated
		StackAllocator(char* stackPtr, uint32_t size) { init(stackPtr, size); }

		// stackPtr would point to some memory you allocated.
		void init(char* stackPtr, uint32_t size) {
			m_write = 0;
			m_size = size;
			m_data = stackPtr;
			m_allocation = false;
			m_canResize = false;
		}
		// Previous allocations will be nullified. Returns false if fail.
		bool init(int virtualSize) {
			m_allocation = true;
			resize(virtualSize, true);
			return m_data != nullptr;
		}
		// The head position where free memory exists will be set to 0. Existing pointers are valid but possibly overwritten.
		// Very fast freeing of memory
		void clear() {
			m_write = 0;
			dreamAllocation = 0;
			totalAllocation = 0;
			// realAllocation remains unchanged
		}
		// Free all allocated data and reset allocator.
		void release() {
			resize(0);
			dreamAllocation = 0;
			totalAllocation = 0;
		}
		// See Allocator's allocate
		virtual char* allocate(uint32_t size) override {
			if (size == 0) return nullptr;
			if (m_write + size > m_size) {
				if (!resize(2 * m_size + 2 * size))
					return nullptr;
			}
			dreamAllocation += size;
			totalAllocation += size;
			char* ptr = m_data + m_write;
			m_write += size;
			return ptr;
		}
		template <class T>
		inline T* allocate() { return (T*)allocate(sizeof(T)); };
		// See Allocator's reallocate
		virtual char* reallocate(char* ptr, uint32_t oldSize, uint32_t newSize) override {
			if (!ptr || oldSize == 0) return allocate(newSize);
			if (newSize == 0) {
				deallocate(ptr, oldSize);
				return nullptr;
			}
			if (m_write + newSize > m_size) {
				if (!resize(2 * m_size + 2 * newSize))
					return nullptr;
			}
			dreamAllocation += newSize - oldSize;
			if (m_data + oldSize == ptr) {// unstack restack
				totalAllocation += newSize - oldSize;// +newSize-OldSize
				m_write += newSize - oldSize;
			} else { // new stack
				char* newPtr = m_data + m_write;
				if (oldSize > newSize) memcpy(newPtr, ptr, newSize);
				else memcpy(newPtr, ptr, oldSize);
				totalAllocation += newSize; // +newSize
				m_write += newSize;
				return newPtr;
			}
		}
		// See Allocator's deallocate
		virtual void deallocate(char* ptr, uint32_t size) override {
			if (!ptr || size == 0) return;
			dreamAllocation -= size;
			if (m_data + size == ptr) { // unstack allocation
				totalAllocation -= size;
				totalAllocation -= size;
				m_write -= size;
			}
		}
		template<class T>
		inline void deallocate(T* ptr) { deallocate(ptr, sizeof(T)); };

	private:
		char* m_data = nullptr;
		uint32_t m_size = 0;
		uint32_t m_write = 0;
		bool m_allocation = false;
		bool m_canResize = true;

		// Returns false if fail. Nullify will free previous allocations.
		// size as zero will free memory
		bool resize(int size, bool nullify = false) {
			if (m_allocation) {
				if (nullify || size == 0) {
					Free(m_data,size);
					m_data = nullptr;
					m_size = 0;
					realAllocation = 0;
					m_write = 0;
					if (size == 0) return true; // returns true because the intention was successful.
				}
				if (!m_data) {
					m_data = (char*)Allocate(size);
					if (!m_data) return false;
					m_size = size;
					realAllocation = size;
					m_write = 0;
					return true;
				} else if (m_canResize) {
					char* data = (char*)Reallocate(m_data, m_size, size);
					if (!data) return false;
					m_data = data;
					m_size = size;
					realAllocation = size;
					return true;
				}
			}
			return false;
		}
	};
	// Allocates memory as you add data.
	// Standard data, size, maxSize and resize features. (beneath that is)
	class StackMemory {
	public:
		StackMemory() = delete;
		~StackMemory() { cleanup(); }

		StackMemory(const StackMemory&) = delete;
		StackMemory& operator =(const StackMemory&) = delete;

		bool add(const char* data, uint32_t size) {
			if (m_size + size > m_maxSize) {
				if (!resize(2 * (m_size + size)))
					return false;
			}
			memcpy_s(m_data + m_size, m_maxSize - m_size, data, size);
			m_size += size;
			return true;
		}
		// request bytes
		// nullptr when failed
		char* request(uint32_t size) {
			if (m_size + size > m_maxSize) {
				if (!resize(2 * (m_size + size)))
					return nullptr;
			}
			char* ptr = m_data + m_size;
			m_size += size;
			return ptr;
		}

		// free memory
		void cleanup() {
			resize(0);
		}
		uint32_t getSize() const {
			return m_size;
		}
		// size needs to be less than maxSize
		void setSize(uint32_t size) {
			m_size = size;
		}
		uint32_t getMaxSize() const {
			return m_maxSize;
		}
		char* getData() const {
			return m_data;
		}
		bool resize(uint32_t size) {
			if (size == 0) {
				Free(m_data, size);
				m_data = nullptr;
				m_size = 0;
				m_maxSize = 0;
			}
			if (!m_data) {
				char* data = (char*)Allocate(size);
				if (!data) return false;
				m_data = data;
				m_maxSize = size;
				m_size = 0;
			} else {
				char* data = (char*)Reallocate(m_data, m_size, size);
				if (!data) return false;
				m_data = data;
				m_maxSize = size;
				if (m_size > m_maxSize) {
					m_size = m_maxSize;
				}
			}
			return true;
		}

	protected:
		char* m_data=nullptr; // char* because i don't like void*
		uint32_t m_maxSize=0;
		uint32_t m_size=0;
	};
}