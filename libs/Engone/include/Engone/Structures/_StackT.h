#pragma once

#include "Engone/Utilities/Alloc.h"

namespace engone {
	template<typename Value>
	class Stack {
	public:
		//typedef int Value;
		Stack() = default;
		~Stack() {
			cleanup();
		}
		void cleanup() {
			m_values.resize(0);
		}
		uint32_t size() {
			return m_values.used;
		}
		bool push(const Value& value) {
			if (m_values.used == m_values.max) {
				bool yes = m_values.resize(m_values.max * 2 + 4);
				if (!yes) return false;
			}
			uint32_t index = m_values.used;
			m_values.used++;

			Value* ptr = (Value*)m_values.data + index;
			new(ptr)Value(value);
			//*ptr = value;

			return true;
		}
		Value pop() {
			bool temp;
			return pop(temp);
		}
		Value pop(bool& success) {
			if (m_values.used == 0) {
				success = false;
				return {};
			}

			m_values.used--;

			success = true;
			return *((Value*)m_values.data + m_values.used);
		}
		Value& peek(uint32_t index) {
			bool temp;
			return peek(index, temp);
		}
		Value& peek(uint32_t index, bool& success) {
			if (m_values.used <= index) {
				success = false;
				throw std::out_of_range("out of range");
			}
			success = true;
			return *((Value*)m_values.data + index);
		}

		bool reserve(uint32_t count) {
			return m_values.resize(count);
		}

		bool copy(Stack& outStack) {
			bool yes = outStack.m_values.resize(m_values.max);
			if (!yes) {
				cleanup();
				return false;
			}
			outStack.m_values.used = m_values.used;

			memcpy(outStack.m_values.data, m_values.data, m_values.max * sizeof(Value));
			return true;
		}

	private:
		Memory m_values{sizeof(Value)};
	};

	void EngoneStackTest();
}