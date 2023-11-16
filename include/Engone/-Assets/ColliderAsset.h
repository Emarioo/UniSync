#pragma once

#include "Engone/Assets/Asset.h"

namespace engone {

	class ColliderAsset : public Asset {
	public:
		static const AssetType TYPE = AssetCollider;
		ColliderAsset() : Asset(TYPE) {  };
		~ColliderAsset() {}

		void cleanup() override;
		LoadFlags load(LoadFlags flag) override;

		enum Type : u8 {
			SphereType = 0,
			CubeType = 1,
			CapsuleType = 2,
			MapType = 3,
		};
		// Height map
		std::vector<glm::vec3> points;
		uint32_t gridWidth = 0, gridHeight = 0;

		Type colliderType = Type::SphereType;
		//union { // use union again
			struct {// sphere
				float radius = 0;
			} sphere;
			struct {// cube
				glm::vec3 size = { 0,0,0 };
			} cube;
			struct {
				float radius = 0, height = 0;
			} capsule;
			struct {
				uint32_t gridColumns = 0, gridRows = 0;
				float minHeight = 0, maxHeight = 0;
				glm::vec3 scale;
				std::vector<float> heights;
			} map;
		//};

		// static TrackerId trackerId;
	private:
	};
}