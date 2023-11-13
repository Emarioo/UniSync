#pragma once

#include "Engone/Utilities/rp3d.h"
// #include "Engone/AssetModule.h"
#include "Engone/Assets/ModelAsset.h"
#include "Engone/Assets/Animator.h"
#include "Engone/LoopInfo.h"
#include "Engone/Utilities/RandomUtility.h"

// #include "Engone/Utilities/Locks.h"
#include "Engone/PlatformModule/PlatformLayer.h"

namespace engone {

	class EngineWorld;
	class EngineObject {
	public:
		EngineObject() = default;
		~EngineObject();
		void cleanup();

		// world is where the object belongs
		// uuid as 0 will generate a new uuid.
		void init(EngineWorld* world, UUID uuid = 0);

		// Sets the colliders based on the model too.
		void setModel(ModelAsset* asset);
		ModelAsset* getModel();

		// Creates a animator and "attaches" it to the object.
		// Do not call delete on the returned pointer as it is owned by the EngineObject.
		Animator* createAnimator();
		Animator* getAnimator();
		void removeAnimator();

#ifdef ENGONE_PHYSICS
		void setOnlyTrigger(bool yes);
		bool isOnlyTrigger();
		// Will load colliders when called and requirements are meet.
		// If requirements aren't meet, the engine will try to load them again next frame.
		// That makes this method somewhat asynchronous.
		// Requirements: modelAsset is valid, rigidbody is valid, argument world isn't nullptr.
		void loadColliders();
		rp3d::RigidBody* getRigidBody();
		void setColliderUserData(void* userData);
		// rigidBody->setTransform...
		void setPosition(const glm::vec3& position);
		glm::vec3 getPosition();
		glm::vec3 getLinearVelocity();
		void setLinearVelocity(const glm::vec3& vec3);
		glm::vec3 getAngularVelocity();
		// applyWorldForceAtCenterOfMass
		void applyForce(const glm::vec3& force);

		glm::mat4 getInterpolatedMat4(float interpolation);

#endif

		UUID getUUID() const;

		void setObjectType(uint32 type);
		uint32 getObjectType();
		void setObjectInfo(uint32 info);
		uint32 getObjectInfo();

		void setFlags(int flags);
		int getFlags();

		EngineWorld* getWorld();

	private:
		uint32 m_objectType = 0;
		UUID m_uuid=0;
		ModelAsset* m_modelAsset = nullptr;
#ifdef ENGONE_PHYSICS
		rp3d::RigidBody* m_rigidBody = nullptr;
		rp3d::Transform prevTransform; // used for interpolation
#endif
		uint32 m_objectInfo = 0;
		Animator* m_animator = nullptr;
		int m_flags = 0;

		EngineWorld* m_world = nullptr; // world the object belongs to
		uint32_t m_objectIndex = -1; // indicates where world stores it
		// Mutex m_mutex;

		void* m_colliderData = nullptr;

		static const int PENDING_COLLIDERS = 1;
		static const int ONLY_TRIGGER = 2;

		friend class EngineWorld;
		friend class Engone;
	};

//	
//	class Engone;
//	class EngineWorld;
//	// move function bodies to cpp file. having them in the header feels bad
//	class EngineObject {
//	public:
//		//-- flags
//		static const uint32_t PENDING_COLLIDERS=1;
//		static const uint32_t ONLY_TRIGGER=2; // all colliders are triggers
//		// make only trigger flag
//
//		//EngineObject() = default;
//		EngineObject() : m_uuid(UUID::New()) {};
//		// uuid as 0 will generate a new UUID
//		EngineObject(UUID uuid) : m_uuid(uuid!=0?uuid:UUID::New()) { };
//		virtual ~EngineObject() {}
//
//		virtual void update(LoopInfo& info) {};
//
//		UUID getUUID() const { return m_uuid; }
//		ModelAsset* modelAsset=nullptr;
//		Animator animator;
//		uint32_t flags=0; // make it private and provide methods?
//		uint32_t objectType = 0;
//
//#ifdef ENGONE_PHYSICS
//		rp3d::RigidBody* rigidBody=nullptr;
//		// a matrix, without scale
//		glm::mat4 getTransform() const {
//			if (rigidBody) 
//				return ToMatrix(rigidBody->getTransform());
//			return glm::mat4(1);
//		}
//		virtual void setTransform(glm::mat4 mat) {
//			if (rigidBody) {
//				rigidBody->setTransform(ToTransform(mat));
//			}
//		}
//		glm::vec3 getPosition() {
//			if(rigidBody)
//				return ToGlmVec3(rigidBody->getTransform().getPosition());
//			return { 0,0,0 };
//		}
//		virtual void setTransform(glm::vec3 vec) {
//			setTransform(glm::translate(glm::mat4(1), vec));
//		}
//		void setOnlyTrigger(bool yes);
//		bool isOnlyTrigger();
//
//		// loadColliders will be called and will require a rigidBody
//		void setModel(ModelAsset* asset);
//		void createRigidBody(EngineWorld* world);
//
//		// Will load colliders whenever requirements are meet. This method is asynchronous.
//		// These are: modelAsset is valid, rigidbody is valid.
//		// The ground parameter is used by the engine and not relevant to you.
//		void loadColliders(EngineWorld* world=nullptr);
//
//		// sets user data for all colliders of the rigidbody.
//		// if colliders aren't loaded. it will be set when they are.
//		void* colliderData=nullptr;
//		void setColliderUserData(void* data) {
//			colliderData = data; 
//			if ((flags & PENDING_COLLIDERS) == 0) { // colliders have been loaded so just set user data here
//				int cols = rigidBody->getNbColliders();
//				for (int i = 0; i < cols; i++) {
//					rigidBody->getCollider(i)->setUserData(colliderData);
//				}
//			} 
//		}
//#endif
//
//		int userData=0;
//
//	private:
//		UUID m_uuid=0;
//
//		friend class Engone;
//	};
}