#pragma once

#include "Engone/Utilities/Alloc.h"

#include "Engone/Logger.h"
namespace engone {
	
	// IMPORTANT
	class FrameArray {
	public:
		// constructor does nothing except remember the variable
		// valuesPerFrame is forced to be divisible by 8. (data alignment * bits as bools)
		FrameArray(uint32 typeSize, uint32 valuesPerFrame, uint32 allocType);
		~FrameArray() { cleanup(); }
		// Does not call free on the items.
		void cleanup();
		
		// Returns -1 if something failed
		// Does not initialize the item
		// @param value pointer to an item which should be added, note that a memcpy occurs. value can be nullptr.
		// @param outPtr sets the pointer for the added object
		uint32 add(void* value, void** outPtr = nullptr);
		
		void* get(uint32 index);
		void remove(uint32 index);
		// in bytes
		uint32 getMemoryUsage();
		// YOU CANNOT USE THIS VALUE TO ITERATE THROUGH THE ELEMENTS!
		inline uint32 getCount() {return m_valueCount;}
		struct Iterator {
			uint32 position=0;
			void* ptr=nullptr;
		};
		// Returns false if nothing more to iterate. Values in iterator are reset
		bool iterate(Iterator& iterator);
		// max = x/8+x*C
		// max = x*(1/8+C)
		// max/(1/8+C)
		// 8*max / (1+8*C)
	private:
		struct Frame {
			Frame(uint32 allocType);
			// Memory<char> memory{};
			Memory memory;
			int count = 0;
			void* getValue(uint32 index, uint32 typeSize, uint32 vpf);
			bool getBool(uint32 index);
			void setBool(uint32 index, bool yes);
		};

		Memory m_frames;
		int m_valueCount = 0;
		
		uint32 m_valuesPerFrame = 0;
		uint32 m_typeSize = 0;
	};

	void FrameArrayTest();
}