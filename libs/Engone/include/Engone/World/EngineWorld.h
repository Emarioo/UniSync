#pragma once

#include "Engone/EngineObject.h"
// #include "Engone/Networking/NetworkModule.h"
#include "Engone/ParticleModule.h"

#include "Engone/Structures/FrameArray.h"
#include "Engone/Structures/Array.h"
#include "Engone/Structures/HashMap.h"

namespace engone {

	class EngineWorld;
	// supposed to be safe from multi-threading
	// it's not at the moment
	// Also be careful when deleting objects within the while loop when using the iterator.
	// deleting the returned object of next in a sub function and then trying to use the object
	// in the while loop will cause a crash.
	class EngineObjectIterator {
	public:
		// list CANNOT be nullptr. crash is bound to happen.
		EngineObjectIterator(EngineWorld* world);

		EngineObject* next();
		void restart();
		// Has to be called to release access to the last used object.
		// Does not need to be called if next returned nullptr
		void finish();

	private:
		FrameArray::Iterator m_iterator;
		//FrameArray<EngineObject>::Iterator m_iterator;
		EngineWorld* m_world = nullptr;

		friend class EngineWorld;
	};

	class Engone;
	// Template for game ground.
	// you need to make a class which derives from this and add a list which stores game objects.
	// The game objcets may be a custom class but has to inherit EngineObject.
	class EngineWorld {
	public:
		EngineWorld(Engone* engone);
		~EngineWorld();
		void cleanup();

		// Creates a new object with rigidbody and sets the rigidbody's user data to the engine object.
		// Always use this function when making objects.
		// createObject is simular to requestAccess and requires you to call releaseAccess
		EngineObject* createObject(UUID uuid=0);
		//EngineObject* getObject(UUID uuid);
		// Asynchronous. UUID is added to a queue which is processed in the update method.
		// releaseAccess is also called
		void deleteObject(UUID uuid);
		

		// Todo: Iterator does not need to be tracked by the world.
		//		Because the objects are stored in FrameArray, insertions and deletions does not affect the position/index value.
		//		It's just an object we should have iterated through next round just got deleted meaning it is just skipped and moved on
		//		to the next object. This is totally fine. The game may not be deterministic with this though.
		// Remeber to delete iterator with deleteIterator
		// Allocation could be done with an Arena Allocator
		EngineObjectIterator createIterator();
		//void deleteIterator(EngineObjectIterator* iterator);
		// Should return nullptr if out of bounds

		int getObjectCount();

		//-- Multithreading safety
		// IMPORTANT: Ensure that no other lock is held when calling this function. Deadlock could occur if you don't know what you are doing.
		// nullptr indicates a failure. uuid may not exist.
		EngineObject* requestAccess(UUID uuid);
		// Only call this after a successful call to requestAccess
		void releaseAccess(UUID uuid);
		//void releaseAccess(EngineObject* obj);

		void addParticleGroup(ParticleGroupT* group);

		inline Array& getParticleGroups() { return m_particleGroups; };

		void update(LoopInfo& info);

		EngineObject* getObject(UUID uuid);

#ifdef ENGONE_PHYSICS
		// does a raycast and returns an array with hit objects.
		// object limit determines how many objects.
		// ignoreObjectType will skip objects of that type. -1 can be seen as skipping no objects
		// Altough if an object has that has type then it will be skipped.
		std::vector<EngineObject*> raycast(rp3d::Ray ray, int objectLimit = 1, int ignoreObjectType=-1);
		rp3d::PhysicsCommon* getPhysicsCommon();
		rp3d::PhysicsWorld* getPhysicsWorld();
		void lockPhysics() { m_physicsMutex.lock();	}
		void unlockPhysics() { 	m_physicsMutex.unlock(); }
		void lockCommon();
		void unlockCommon();
#endif

		void* getUserData() { return m_userData; }
		void setUserData(void* ptr) { m_userData=ptr; }

	private:
#ifdef ENGONE_PHYSICS
		Mutex m_physicsMutex;
		rp3d::PhysicsWorld* m_physicsWorld = nullptr;
#endif

		// Todo: cache friendly mutex allocation? mutexes cannot just be moved. Once allocated and used by mutexes the allocation cannot be reallocated.
		//		Due to how std::mutex is implemented. Should be fine on Windows. More explanations in the link.
		//		https://stackoverflow.com/questions/7557179/move-constructor-for-stdmutex
		

		//Mutex m_iteratorsMutex;
		//std::vector<EngineObjectIterator*> m_iterators;

		// you may want to make an entity component system or something
		//std::vector<ParticleGroupT*> m_particleGroups;

		Array m_particleGroups{sizeof(ParticleGroupT*),ALLOC_TYPE_GAME_MEMORY};

		Mutex m_objectsMutex;
		FrameArray m_objects = { sizeof(EngineObject),32,ALLOC_TYPE_GAME_MEMORY }; // number stands for how many objects in one frame
		//FrameArray<EngineObject> m_objects = { 8 }; // number stands for how many objects in one frame
		//FrameArray<EngineObject> m_objects = { 64 }; // number stands for how many objects in one frame
		
		//std::vector<EngineObject*> m_objects;

		//struct ObjectDetail {
		//	EngineObject* object = nullptr;
		//	int waitingThreads = 0;
		//	bool pendingDelete = false;
		//	int lockingDepth=0;
		//};
		//Mutex m_objectMapMutex;
		//std::unordered_map<UUID, ObjectDetail> m_objectMap;

		//HashMap m_objectMap{sizeof(ObjectDetail),ALLOC_TYPE_GAME_MEMORY,50};

		Engone* m_engone=nullptr;

		void* m_userData=nullptr;

		friend class Application;
		friend class EngineObjectIterator;
	};
}

