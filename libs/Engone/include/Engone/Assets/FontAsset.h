#pragma once

#include "Engone/Assets/Asset.h"
#include "Engone/Rendering/Buffer.h"

namespace engone {
	class FontAsset : public Asset {
	public:
		static const AssetType TYPE = AssetFont;
		FontAsset() : Asset(TYPE) {  };
		~FontAsset() { cleanup(); }
		// don't forget to call load on the asset. You could also load it with AssetStorage::set
		FontAsset(int id, const std::string& widths) : Asset(TYPE), widthValues(widths),png(PNG::Load(id)) {
			m_flags = LoadData; // skip io
			png->setFlags(FlipOnLoad,true);
		}
		// path should not include format, .txt and .png will be read
		FontAsset(const std::string& path) : Asset(TYPE) {
			m_flags = LoadIO;
			m_path = path;
		}
		void cleanup() override;
		// font is special in that it requires to files
		// a .txt and .png with the same name.
		// path is without the format
		LoadFlags load(LoadFlags flags) override;

		//void load(const void* bufferImg, uint32_t size, const void* bufferTxt, uint32_t size2);

		float getWidth(const std::string& str, float height);
		// \n is included
		float getHeight(const std::string& str, float height);

		Texture texture;
		int charWid[256];

		static const int imgSize = 1024;
		static const int charSize = imgSize / 16;// 64

		// static TrackerId trackerId;
	
	private:
		PNG* png = nullptr;
		RawImage* rawImage = nullptr;
		std::string widthValues;
	};
}