#pragma once

#include "Engone/Utilities/Alloc.h"

namespace engone {
	class Stack {
	public:
		//typedef int Value;
		Stack(uint32 typeSize, uint32 allocType);
		~Stack() { cleanup(); }
		void cleanup() { m_values.resize(0); }
		uint32_t size() { return m_values.used; }

		bool push(void* value);
		// memcpy into outPtr is done
		bool pop(void* outPtr=nullptr);
		void* peek(uint32_t index);
		bool reserve(uint32_t count) {
			return m_values.resize(count);
		}

		bool copy(Stack& outStack);

	private:
		Memory m_values;
	};

	void EngoneStackTest();
}