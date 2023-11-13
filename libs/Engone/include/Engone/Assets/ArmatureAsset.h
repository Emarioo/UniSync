#pragma once

#include "Engone/Assets/Asset.h"

namespace engone {
	class Bone {
	public:
		Bone() = default;

		uint16_t parent = 0;

		glm::mat4 localMat = glm::mat4(1);
		glm::mat4 invModel = glm::mat4(1);


	};
	class ArmatureAsset : public Asset {
	public:
		static const AssetType TYPE = AssetArmature;
		ArmatureAsset() : Asset(TYPE) { };
		~ArmatureAsset() {}

		void cleanup() override;
		LoadFlags load(LoadFlags flag) override;


		std::vector<Bone> bones;

		// static TrackerId trackerId;
	private:
	};
}