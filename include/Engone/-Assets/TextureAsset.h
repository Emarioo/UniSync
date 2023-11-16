#pragma once

#include "Engone/Assets/Asset.h"
#include "Engone/Rendering/Buffer.h"

namespace engone {
	class TextureAsset : public Asset {
	public:
		static const AssetType TYPE = AssetTexture;
		TextureAsset() : Asset(TYPE) { };
		TextureAsset(int id) : Asset(TYPE) { 
			auto png = PNG::Load(id);
			rawImage = PNGToRawImage(png);
			engone::Free(png, sizeof(PNG));
			// ALLOC_DELETE(PNG,png);
			m_flags=LoadGraphic;
		}
		~TextureAsset() { cleanup(); }
		
		void cleanup() override;
		LoadFlags load(LoadFlags flags) override;
		// 
		//Texture(const std::string& path) : Asset(TYPE) { load(path); };
		//Texture(const char* inBuffer, uint32_t size, Assets* assets = nullptr) : Asset(TYPE, "") { load(inBuffer, size, assets); };
		//void load(const void* inBuffer, uint32_t size);

		//void init(int w, int h, void* data);

		//void subData(int x, int y, int w, int h, void* data);
		//void bind(unsigned int slot = 0);

		//int getWidth();
		//int getHeight();

		Texture texture;

		// static TrackerId trackerId;
	private:
		RawImage* rawImage = nullptr;
		//unsigned int id = 0;
		//unsigned char* buffer = nullptr;
		//int width = 0, height = 0, BPP = 0;
	};
}