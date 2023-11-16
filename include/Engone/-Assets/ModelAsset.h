#pragma once
#include "Engone/Assets/Asset.h"
#include "Engone/Assets/Animator.h"

namespace engone {
	class ArmatureAsset;
	class AnimationAsset;
	class AssetInstance {
	public:
		AssetInstance() = default;

		std::string name;
		int16_t parent = -1;

		glm::mat4 localMat = glm::mat4(1);
		glm::mat4 invModel = glm::mat4(1);

		Asset* asset;

		// static TrackerId trackerId;
	private:
	};
	class ModelAsset : public Asset {
	public:
		static const AssetType TYPE = AssetModel;
		ModelAsset() : Asset(TYPE) {  };
		~ModelAsset() { cleanup(); }

		void cleanup() override;
		LoadFlags load(LoadFlags flag) override;
		bool valid() const override;

		/*
		List of mesh, collider and armature transforms.
		transforms[meshes.size()+colliders.size()+1] to get the second armature transform
		*/
		//std::vector<glm::mat4> transforms;
		std::vector<AssetInstance> instances;
		std::vector<AnimationAsset*> animations;

		// min max of all mesh vertices. Does not take animations into account.
		// these are not accurate if instances are reloaded
		glm::vec3 minPoint;
		glm::vec3 maxPoint;
		glm::vec3 boundingPoint;
		float boundingRadius;

		// Will give a list of combined parent matrices to instances, do mats[i] * instance.localMat to get whole transform
		// Asset has to be loaded for this to work
		std::vector<glm::mat4> getParentTransforms(Animator* animator);

		// will find the first instance with asset type T
		//template<typename T>
		//AssetInstance* findInstance() {
		//	for (int i = 0; i < instances.size(); i++) {
		//		auto& inst = instances[i];
		//		if (inst.asset->type == T::TYPE) {
		//			return inst.asset;
		//		}
		//	}
		//	return nullptr;
		//}

		// instance: The armature instance. Not the mesh instance
		// Asset has to be loaded for this to work
		std::vector<glm::mat4> getArmatureTransforms(Animator* animator, glm::mat4& instanceMat, AssetInstance* instance, ArmatureAsset* asset, std::vector<glm::mat4>* boneMats = nullptr);

		// static TrackerId trackerId;
	private:
	};
}