#pragma once

#ifdef ENGONE_PHYSICS
#include "reactphysics3d/reactphysics3d.h"
/*
	Some useful functions for ReactPhysics3D
*/
namespace engone {
	// turn glm to rp3d, can unfortunately not be constexpr
	rp3d::Vector3 ToRp3dVec3(const glm::vec3& in);
	// turn rp3d to glm
	glm::vec3 ToGlmVec3(const rp3d::Vector3& in);
	rp3d::Quaternion ToRp3dQuat(const glm::quat& in);
	// turn rp3d to glm
	glm::quat ToGlmQuat(const rp3d::Quaternion& in);
	// may not work properly
	rp3d::Vector3 ToEuler(const rp3d::Quaternion& q);
	// may not work properly
	rp3d::Transform ToTransform(const glm::mat4& m, glm::vec3* scale=nullptr);
	glm::mat4 ToMatrix(const rp3d::Transform& t);
	void DecomposeGlm(const glm::mat4& m, glm::vec3* pos, glm::quat* rot, glm::vec3* scale);
}
#endif