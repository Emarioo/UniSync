#pragma once

#include "Engone/Assets/Asset.h"

namespace engone {
	class TextureAsset;
	class Shader;
	class MaterialAsset : public Asset {
	public:
		static const AssetType TYPE = AssetMaterial;

		MaterialAsset() : Asset(TYPE) { };
		~MaterialAsset() { cleanup(); }

		void cleanup() override;
		LoadFlags load(LoadFlags flag) override;
		bool valid() const override;

		TextureAsset* diffuse_map = nullptr;
		glm::vec3 diffuse_color = { 1,1,1 };
		glm::vec3 specular = { .5f,.5,.5f };
		float shininess = .5;

		void bind(Shader* shader, int index);

		// static TrackerId trackerId;
	private:
	};
}