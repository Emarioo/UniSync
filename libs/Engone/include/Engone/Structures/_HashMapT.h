#pragma once

#include "Engone/Structures/Stack.h"
#include "Engone/Logger.h"

namespace engone {
	// Integer based hash map meant for few pairs.
	// Few means 100 but more can be used but maybe not optimal.
	// Or maybe it is but that isn't the goal
	// Todo: More testing to ensure that this class is flawless. There may still be issues.
	class HashMap {
	public:
		typedef int Key;
		typedef int Value;
		static const int INVALID_INDEX = -1;

		struct IterationInfo {
			Key key;
			Value value;
			int index;
			int depth;
			
			// uses printf
			void print();
		private:
			int position=0;

			friend class HashMap;
		};
		// keymapSize is the size used when allocating initial array where key/values are
		// stored. Duplicates will be stored in a seperate allocation.
		// The size should be high enough to avoid duplicates and small enough to
		// be memory efficient. A fairly good size would be the amount of expected values
		// multiplied by 1.5.
		// keymapSize is just a variable that is set within the class. Nothing else is done in
		// the constructor.
		HashMap(int keymapSize=50);
		~HashMap();
		// frees memory and makes the hash map as good as new.
		// even preventDeconstructor
		// keymapSize stays the same
		void cleanup();

		void preventDeconstructor();

		bool insert(Key key, Value value);
		
		// found is a pointer to a bool which if true indicates the value being found.
		// false means not found.
		Value get(Key key, bool* found = nullptr);
		
		// An alternative function
		bool get(Key key, Value& outValue);

		bool erase(Key key);

		// Primarly works as get. If value isn't found then a value will be created which can
		// then be change with the returned reference.
		// Todo: operator[] to work with both insert and get
		//Value& operator[](int key);

		// Any changes made to hash map will invalidate the position which
		// will cause unexpected results but no crash.
		bool iterate(IterationInfo& info);

		// Equals operator for sameAs?

		// returns the amount of inserted things
		int getSize();

		bool sameAs(HashMap& map);

		int getMemoryUsage();

		// outMap will be cleaned and then all data of this map will
		// be copied to outMap. (new allocations will be done making the maps independent)
		// returns false if allocations failed
		bool copy(HashMap& outMap);

		void print();

	private:
		// size of this struct is 11 (12 with sizeof(chain)). It used to be 13 (16 with sizeof(Chain)).
		// indexNext is short and next to used to achive the 12 byte size instead of 16.
		struct Chain {
			bool used=false;
			short indexNext = INVALID_INDEX; // for overflow and linked list
			Key key;
			Value value;
		};
		
		
		Memory keymap{sizeof(Chain)};
		Memory chainData{sizeof(Chain)};
		Stack<int> emptySpots;
		int keymapSize=20; // preferred size by the user
		int size=0; // amount of insertions
		bool m_preventDeconstructor = false;

		int hash(Key key);

		// the created chain is reset to default values from Chain()
		// the function returns indexNext
		// -1 is returned if something failed. usually allocation failure.
		int createChain();

		friend void HashMapTestCase();
	};

	void HashMapTestCase();
}