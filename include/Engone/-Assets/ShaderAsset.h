#pragma once

#include "Engone/Assets/Asset.h"
#include "Engone/Rendering/Buffer.h"

namespace engone {

	class ShaderAsset : public Asset, public Shader {
	public:
		static const AssetType TYPE = AssetShader;
		//ShaderAsset() : Asset(TYPE) { }; // <- deleted
		ShaderAsset(const char* source) : Asset(TYPE), Shader(source) { m_state = Loaded; };
		~ShaderAsset() { cleanup(); }

		void cleanup() override {
			// Shader destructor calls itself
		}
		LoadFlags load(LoadFlags flags) override {
			// loading is done in constructor
			return 0;
		}

		// static TrackerId trackerId;
	private:

	};
}