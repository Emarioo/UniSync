#pragma once

#include "Engone/Utilities/Alloc.h"

#include "Engone/Logger.h"
namespace engone {
	
	template<class Value>
	class FrameArray {
	public:
		//typedef int Value;

		// constructor does nothing except remember the variable
		// valuesPerFrame is forced to be divisible by 64. (data alignment * bits as bools)
		FrameArray(uint32 valuesPerFrame, int allocType) : m_valuesPerFrame(valuesPerFrame), m_allocType(allocType), m_frames{sizeof(Frame),allocType) {
			//uint32 off = valuesPerFrame & 63;
			//if(off!=0)
			//	m_valuesPerFrame += 64 - off;
			uint32 off = valuesPerFrame & 7;
			if (off != 0)
				m_valuesPerFrame += 8 - off;
		}
		~FrameArray() {
			cleanup();
		}
		void cleanup() {
			for (int i = 0; i < m_frames.max;i++) {
				Frame& frame = *((Frame*)m_frames.data+i);
				if (frame.memory.max != 0) {
					for (int j = 0; j < m_valuesPerFrame; j++) {
						bool yes = frame.getBool(j);
						if (yes) {
							Value* ptr = frame.getValue(j, m_valuesPerFrame);
							ptr->~Value();
						}
					}
				}
				frame.memory.resize(0);
			}
			m_frames.resize(0);
			m_valueCount = 0;
		}
		
		// Returns -1 if something failed
		uint32 add(Value value) {
			if (m_valuesPerFrame == 0) return -1;

			//if(m_valuesPerFrame !=8)
			//	log::out << log::Disable;
			
			//log::out << "FA : ADD "<<m_valuesPerFrame<<"\n";
			// Find available frame
			int frameIndex = -1;
			for (int i = 0; i < m_frames.max; i++) {
				Frame& frame = *((Frame*)m_frames.data+i);
				//log::out << "FA : " << frame.count << " != " << m_valuesPerFrame<< "\n";
				if (frame.count != m_valuesPerFrame) {
					frameIndex = i;
					//log::out << "FA : found frame "<<i<<" with "<< frame.count << " objects\n";
					break;
				}
			}
			// Create new frame if non found
			if (frameIndex==-1) {
				int initialMax = m_frames.max;
				bool yes = m_frames.resize(m_frames.max*1.5+1);
				if (!yes) {
					//log::out >> log::Disable;
					return -1;
				}
				// memset((Frame*)m_frames.data + initialMax, 0, (m_frames.max - initialMax) * sizeof(Frame));
				frameIndex = initialMax;
				for(int i=initialMax;i<m_frames.max;i++){
					*((Frame*)m_frames.data + i) = {1}; // char
				}
				//log::out << "FA : Create frame " << frameIndex << "\n";
			}
			
			// Insert value into frame
			Frame& frame = *((Frame*)m_frames.data+frameIndex);
			if (frame.memory.max==0) {
				bool yes = frame.memory.resize(m_valuesPerFrame+ m_valuesPerFrame * sizeof(Value));
				//bool yes = frame.memory.resize(m_valuesPerFrame/8+ m_valuesPerFrame * sizeof(Value));
				if (!yes) { 
					//log::out >> log::Disable; 
					return -1; 
				}

				//memset(frame.memory.data,0,m_valuesPerFrame/8);
				memset(frame.memory.data,0,m_valuesPerFrame);
				//log::out << "FA : Reserve values\n";
			}
			
			// Find empty slot
			int valueIndex = -1;
			for (int i = 0; i < m_valuesPerFrame;i++) {
				bool yes = frame.getBool(i);
				if (!yes) {
					valueIndex = i;
					//log::out << "FA : found spot " << i << "\n";
					break;
				} else {
					//log::out << "FA : checked spot " << i <<"\n";
				}
			}

			if (valueIndex == -1) {
				log::out << log::RED << "FrameArray : Impossible error adding value in frame " << frameIndex << "\n";
				//log::out >> log::Disable;
				return -1;
			}
			frame.count++;
			//log::out << "Frame : New object count " << frame.count << "\n";
			m_valueCount++;
			frame.setBool(valueIndex, true);
			Value* ptr = frame.getValue(valueIndex, m_valuesPerFrame);
			new(ptr)Value(value);
			//log::out >> log::Disable;
			return frameIndex*m_valuesPerFrame+valueIndex;
		}
		
		Value* get(uint32 index) {
			uint32 frameIndex = index / m_valuesPerFrame;
			uint32 valueIndex = index % m_valuesPerFrame;

			if (frameIndex >= m_frames.max)
				return nullptr;

			Frame& frame = *((Frame*)m_frames.data+frameIndex);

			if (valueIndex >= frame.memory.max)
				return nullptr;

			bool yes = frame.getBool(valueIndex);
			if (!yes) // Check if slot is empty
				return nullptr;

			Value* ptr = frame.getValue(valueIndex, m_valuesPerFrame);
			return ptr;
		}
		void remove(uint32 index) {
			uint32 frameIndex = index / m_valuesPerFrame;
			uint32 valueIndex = index % m_valuesPerFrame;

			if (frameIndex >= m_frames.max)
				return;

			Frame& frame = *((Frame*)m_frames.data+frameIndex);

			if (frame.memory.max==0)
				return;
			bool yes = frame.getBool(valueIndex);
			if (!yes)
				return;
			
			frame.count--;
			m_valueCount--;
			frame.setBool(valueIndex,false);
			Value* ptr = frame.getValue(valueIndex, m_valuesPerFrame);
			ptr->~Value();

			//if (frame.count==0) {
			//	frame.resize(0);
			//}
		}
		// in bytes
		uint32 getMemoryUsage() {
			uint32_t bytes = m_frames.max*sizeof(Frame);
			for (int i = 0; i < m_frames.max; i++) {
				bytes += ((Frame*)m_frames.data+i)->memory.max;
			}
			return bytes;
		}
		// YOU CANNOT USE THIS VALUE TO ITERATE THROUGH THE ELEMENTS!
		uint32 getCount() {
			return m_valueCount;
		}
		struct Iterator {
			uint32 position=0;
			Value* ptr=nullptr;
		};
		// Returns false if nothing more to iterate. Values in iterator are reset
		bool iterate(Iterator& iterator) {
			while (true) {
				uint32 frameIndex = iterator.position / m_valuesPerFrame;
				if (frameIndex >= m_frames.max)
					break;
				Frame& frame = *((Frame*)m_frames.data+frameIndex);

				uint32 valueIndex = iterator.position % m_valuesPerFrame;
				if (valueIndex >= frame.memory.max)
					break;
				
				iterator.position++;

				bool yes = frame.getBool(valueIndex);
				if (!yes)
					continue;
				
				iterator.ptr = frame.getValue(valueIndex, m_valuesPerFrame);
				return true;
			}
			iterator.position = 0;
			iterator.ptr = nullptr;
			return false;
		}
		// max = x/8+x*C
		// max = x*(1/8+C)
		// max/(1/8+C)
		// 8*max / (1+8*C)
	private:
		struct Frame {
			// Memory<char> memory{};
			Memory memory{1};
			int count = 0;
			Value* getValue(uint32 index, uint32 vpf) {
				//return (Value*)(memory.data+ vpf/8+index*sizeof(Value));
				return (Value*)((char*)memory.data+ vpf+index*sizeof(Value));
			}
			bool getBool(uint32 index) {
				//uint32 i = index / 8;
				//uint32 j = index % 8;
				//char byte = memory.data[i];
				//char bit = byte&(1<<j);

				return *((char*)memory.data+index);
			}
			void setBool(uint32 index, bool yes) {
				//uint32 i = index / 8;
				//uint32 j = index % 8;
				//if (yes) {
				//	memory.data[i] = memory.data[i] | (1 << j);
				//} else {
				//	memory.data[i] = memory.data[i] & (~(1 << j));
				//}
				*((char*)memory.data+index) = yes;
			}
		};

		// Memory<Frame> m_frames{};
		Memory m_frames;
		int m_valueCount = 0;
		
		uint32 m_valuesPerFrame = 0;
		int m_allocType = 0;
	
	};

	void FrameArrayTest();
}