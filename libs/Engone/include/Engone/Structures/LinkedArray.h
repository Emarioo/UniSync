#pragma once

#include "Engone/Utilities/Stack.h"

namespace engone {
	
	class LinkedArray {
	public:
		typedef int Value;

		LinkedArray() = default;
		~LinkedArray();
		void cleanup();

		// add value to the back
		bool add(Value value) {
			uint32_t nodeIndex=-1;
			if (m_emptySpots.size() != 0) {
				nodeIndex = m_emptySpots.pop();
			}else{
				if (m_nodes.used == m_nodes.max) {
					bool yes = m_nodes.resize(m_nodes.max * 2 + 4);
					if (!yes) return false;
				}
				nodeIndex = m_nodes.used;
				m_nodes.used++;
			}
			m_valueCount++;
			Node* node = (m_nodes.data + nodeIndex);
			node->next = -1;
			new(&node->value)Value(value);

			if (m_end != -1) {
				Node* endNode = (m_nodes.data + m_end);
				endNode->next = nodeIndex;
			}

			if (m_start == -1)
				m_start = nodeIndex;
			m_end = nodeIndex;
			return true;
		}
		Value& get(uint32_t index) {
			if (index >= m_valueCount)
				throw std::out_of_range("out of range");
			
			if(m_start==-1)
				throw std::out_of_range("empty list");

			if (index == m_valueCount - 1)
				return (m_nodes.data + m_end)->value; // quick access to the back

			Node* node = m_nodes.data + m_start;
			int atIndex = 0;
			while (true) {
				if (index == atIndex)
					return node->value;
				
				if(node->next==-1)
					throw std::out_of_range("out of range");
				node = m_nodes.data + node->next;
			}
		}
		void remove(uint32_t index) {
			if (index >= m_valueCount)
				return;
			if (m_start == -1)
				return;

			// quick removal for end? requires an extra variable like m_nearEnd

			Node* last = nullptr;
			Node* node = m_nodes.data + m_start;
			int atIndex = 0;
			int atNodeIndex = m_start;
			while (true) {
				if (index == atIndex) {
					if (atIndex == m_start) {
						m_start = node->next;
						m_emptySpots.push(atNodeIndex);
					}
					return;
				}
					//return node->value;

				if (node->next == -1)
					throw std::out_of_range("out of range");
				
				last = node;
				atNodeIndex = node->next;
				node = m_nodes.data + node->next;
			}
		}

		Value& operator[](uint32_t index);

		bool copy(LinkedArray& linkedArray) {
			return false;
			//bool yes = outStack.m_values.resize(m_values.max);
			//if (!yes) {
			//	cleanup();
			//	return false;
			//}
			//outStack.m_values.used = m_values.used;

			//memcpy(outStack.m_values.data, m_values.data, m_values.max * sizeof(Value));
		}

	private:
		struct Node {
			Value value;
			uint32_t next=-1;
		};
		Memory<Node> m_nodes;
		uint32_t m_start = -1;
		uint32_t m_end = -1;
		Stack<uint32_t> m_emptySpots;
		uint32_t m_valueCount=0;
		// you could use another allocation to keep some quick access indices.
		// arr = [295,2189,16]
		// arr[0] points to Node 0
		// arr[1] points to Node 128
		// arr[2] points to Node 256
	
	};

	// Option 1 is best. The others doesn't really follow the true spirit of Linked Arrays.
	// 
	// Option 1:
	//struct Node {
	//	Value value;
	//	uint32_t next;
	//};
	//Memory<Node> nodes;
	// Pros: one allocation

	// Option 2:
	//Memory<Value> values;
	//Memory<uint32_t> nexts;
	// Pros: values.data gives you raw Value* which you can access yourself. But you aren't really meant too.
	// Cons: two allocations

	// Option 3:
	//Memory<char> data;
	// Value value = data.data[i*sizeof(Value)];
	// uint32_t next = data.data[data.max*sizeof(Value)+i*sizeof(uint32_t)];
	// Pros: one allocation. 
}