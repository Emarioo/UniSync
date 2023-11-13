#pragma once

#include "Engone/Utilities/Alloc.h"

namespace engone {

	class Array {
	public:
		Array(uint32 typeSize, uint32 allocType) : m_values(typeSize,allocType) {}
		// calls cleanup
		~Array() { cleanup(); }
		// frees allocations
		void cleanup();

		uint32 size();
		void* data();

		// Item is added to the back.
		// Returns false if allocation failed
		bool add(const void* value, void** outPtr=nullptr);

		bool insert(uint32 index, const void* value);

		// nullptr if unused/invalid index
		void* get(uint32 index);
		void remove(uint32 index);

		bool copy(Array* out);
		
		void clear();

		// Allocate memory in advance.
		// Method can trim values if less than size
		// count is not in bytes!
		bool reserve(uint32 count) {
			return m_values.resize(count);
		}
		bool resize(uint32 count) {
			if (!m_values.resize(count))
				return false;
			m_values.used = count;
			return true;
		}
	private:
		Memory m_values;
	};

	void TestEngoneArray();
}