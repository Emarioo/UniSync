// #include "Engone/Assets/AnimationAsset.h"
// #include "Engone/Assets/ArmatureAsset.h"
// #include "Engone/Assets/ColliderAsset.h"
// #include "Engone/Assets/FontAsset.h"
// #include "Engone/Assets/MaterialAsset.h"
// #include "Engone/Assets/MeshAsset.h"
// #include "Engone/Assets/ModelAsset.h"
// #include "Engone/Assets/TextureAsset.h"
// #include "Engone/Assets/ShaderAsset.h"

/*
	Asset.cpp require templates from asset storage does this have something to do with the lnk errors?
*/

#include "Engone/Assets/AssetModule.h"

/* 
	This file contains definitions for all assets.

	Todo: Error handling is broken. Try and catch should be removed.
*/

namespace engone {
	// Todo: Change the error
#define CHECK_ERR(x) if(!(x)) { m_error = OtherError; m_state=Failed; return m_error; }

	// TrackerId TextureAsset::trackerId = "TextureAsset";
	// TrackerId FontAsset::trackerId = "FontAsset";
	// TrackerId MaterialAsset::trackerId = "MaterialAsset";
	// TrackerId AnimationAsset::trackerId = "AnimationAsset";
	// TrackerId MeshAsset::trackerId = "MeshAsset";
	// TrackerId ColliderAsset::trackerId = "ColliderAsset";
	// TrackerId ArmatureAsset::trackerId = "ArmatureAsset";
	// TrackerId ModelAsset::trackerId = "ModelAsset";
	// TrackerId ShaderAsset::trackerId = "ShaderAsset";
	
	// TrackerId Asset::GetTrackerId(AssetType type) {
	// 	switch (type) {
	// 	case AssetTexture: return TextureAsset::trackerId;
	// 	case AssetFont: return FontAsset::trackerId;
	// 	case AssetShader: return ShaderAsset::trackerId;
	// 	case AssetMaterial: return MaterialAsset::trackerId;
	// 	case AssetMesh: return MeshAsset::trackerId;
	// 	case AssetAnimation: return AnimationAsset::trackerId;
	// 	case AssetArmature: return ArmatureAsset::trackerId;
	// 	case AssetCollider: return ColliderAsset::trackerId;
	// 	case AssetModel: return ModelAsset::trackerId;
	// 	}
	// 	return "";
	// }

	const char* GetFormat(AssetType type) {
		switch (type) {
		case AssetTexture: return ".png";
		case AssetFont: return "";
		//case AssetShader: return "";
		case AssetMaterial: return ".material";
		case AssetMesh: return ".mesh";
		case AssetAnimation: return ".animation";
		case AssetArmature: return ".armature";
		case AssetCollider: return ".collider";
		case AssetModel: return ".model";
		}
		return "";
	}
	const char* ToString(AssetType type) {
		switch (type) {
		case AssetTexture: return "Texture";
		case AssetFont: return "Font";
		case AssetShader: return "Shader";
		case AssetMaterial: return "Material";
		case AssetMesh: return "Mesh";
		case AssetAnimation: return "Animation";
		case AssetArmature: return "Armature";
		case AssetCollider: return "Collider";
		case AssetModel: return "Model";
		}
		return "";
	}
	Logger& operator<<(Logger& log, AssetType type) {
		return log << ToString(type);
	}
	const char* ToString(Asset::LoadFlag type) {
		switch (type) {
		case Asset::LoadNone: return "LoadNone";
		case Asset::LoadIO: return "LoadIO";
		case Asset::LoadData: return "LoadData";
		case Asset::LoadGraphic: return "LoadGraphic";
		case Asset::LoadAll: return "LoadAll";
		}
		return "";
	}
	Logger& operator<<(Logger& log, Asset::LoadFlag value) {
		return log << ToString(value);
	}
	std::string Asset::CreateName(const std::string& path) {
		int beg = path.find_last_of("/")+1;
		int end = path.find_last_of(".");
		if (end == -1) end = path.length();
		return path.substr(beg, end-beg);
	}
	std::string Asset::CreateRoot(const std::string& path) {
		if (path.empty())
			return "";
		int slashIndex = path.find_last_of("/");
		if (slashIndex == -1)
			return path + "/";
		return path.substr(0, slashIndex + 1); // +1 to include 
	}
	void FontAsset::cleanup() {
		texture.cleanup();
	}

	Asset::LoadFlags FontAsset::load(LoadFlags flags) {
		LoadFlags out = 0;
		if ((flags & LoadIO) && !m_error&&!m_path.empty()) {
			FileReader file(m_path + ".txt", false);
			if (!file.isOpen()) {
				m_error = FileNotFound;
				m_state = Failed;
			} else {
				try {
					file.readAll(widthValues);
				} catch (Error err) {
					m_error = err;
					m_state = Failed;
				}
				png = PNG::ReadFile((m_path + ".png").c_str());
				if (!png) {
					m_error = FileNotFound;
					m_state = Failed;
				} else {
					png->setFlags(FlipOnLoad, true);
					if (m_error == NoError)
						out = LoadData;
				}
			}
		}
		if ((flags & LoadData) && !m_error) {
			std::vector<std::string> list = SplitString(widthValues, "\n");
			if (list.size() == 2) {
				charWid[0] = std::stoi(list.at(0));
				int num = std::stoi(list.at(1));
				for (uint32_t i = 1; i < 256; ++i) {
					charWid[i] = num;
				}
			} else {
				uint32_t i = 0;
				for (std::string s : list) {
					std::vector<std::string> list2 = SplitString(s, ",");
					for (std::string s2 : list2) {
						charWid[i] = std::stoi(s2);
						i++;
					}
				}
			}
			widthValues.clear();
			
			if (png) {
				// rawImage = Image::Convert<PNG, RawImage>(png);
				rawImage = PNGToRawImage(png);
				ALLOC_DELETE(PNG, png);
				//delete png;
				png = nullptr;
				out = LoadGraphic;
			}
		}
		if ((flags & LoadGraphic) && !m_error) {
			if (!rawImage)
				log::out << log::RED << "TextureAsset::load - rawImage is nullptr\n"; // a little log incase it happens
			// rawImage can only be nullptr if AssetStorage is broken or if you set LoadGraphic flag on an empty asset.

			texture.init(rawImage);
			//delete rawImage;
			ALLOC_DELETE(RawImage, rawImage);
			rawImage = nullptr;
			m_state = Loaded;
			out = 0;
		}
		return out;
	}
	//void FontAsset::load(const void* bufferImg, uint32_t size, const void* bufferTxt, uint32_t size2) {
	//	std::string str;
	//	str.resize(size2, 0);
	//	memcpy(str.data(), bufferTxt, size2);
	//	std::vector<std::string> list = SplitString(str, "\n");
	//	if (list.size() == 2) {
	//		charWid[0] = std::stoi(list.at(0));
	//		int num = std::stoi(list.at(1));
	//		for (uint32_t i = 1; i < 256; ++i) {
	//			charWid[i] = num;
	//		}
	//	}
	//	else {
	//		uint32_t i = 0;
	//		for (std::string s : list) {
	//			std::vector<std::string> list2 = SplitString(s, ",");
	//			for (std::string s2 : list2) {
	//				charWid[i] = std::stoi(s2);
	//				i++;
	//			}
	//		}
	//	}
	//	//DebugBreak();
	//	// fix code below
	//	texture.load(bufferImg, size);
	//	if (!texture.valid()) {
	//		error = texture.error;
	//	}
	//}
	float FontAsset::getWidth(const std::string& str, float height) {
		if (m_state != Loaded) return 0;
		float wid = 0;
		float maxWidth = 0;
		for (uint32_t i = 0; i < str.length(); ++i) {
			unsigned char chr = str[i];
			if (chr != '\n')
				wid += height * (charWid[chr] / (float)charSize);

			if (chr == '\n' || i == str.length() - 1) {
				if (maxWidth < wid)
					maxWidth = wid;
				wid = 0;
				continue;
			}
		}
		return maxWidth;
	}
	float FontAsset::getHeight(const std::string& str, float height) {
		if (m_state != Loaded) return 0;
		float maxHeight = height;
		for (uint32_t i = 0; i < str.length(); ++i) {
			unsigned char chr = str[i];
			if (chr == '\n')
				maxHeight += height;
		}
		return maxHeight;
	}
	void TextureAsset::cleanup() {
		texture.cleanup();
	}
	Asset::LoadFlags TextureAsset::load(LoadFlags flags) {
		LoadFlags out = 0;
		if ((flags & LoadIO) && !m_error&&!m_path.empty()) {
			rawImage = RawImage::ReadFromPNG(m_path);
			if (!rawImage) {
				m_error = FileNotFound;
				m_state = Failed;
			} else {
				out = LoadGraphic;
			}
		}
		if ((flags&LoadGraphic) && !m_error) {
			if (!rawImage)
				log::out << log::RED << "TextureAsset::load - rawImage is nullptr\n"; // a little log incase it happens
			// rawImage should not be nullptr if we are here so m_state does not need to be set to Failed.

			//std::destroy_at()

			texture.init(rawImage);
			//delete rawImage;
			ALLOC_DELETE(RawImage, rawImage);
			rawImage = nullptr;
			m_state = Loaded;
			out = 0;
		}
		return out;
	}
	void MaterialAsset::cleanup() {
		if (diffuse_map) {
			log::out << log::RED<< "MaterialAsset : Cleanup may be broken (deleting diffuse_map, is it used for others?)\n";
			if (!diffuse_map->getStorage()) {
				ALLOC_DELETE(TextureAsset,diffuse_map);
				// GetTracker().untrack(diffuse_map);
			} else {
				// texture belongs to storage, storage will delete texture
				// bad things happen if asset already is deleted.
			}
			diffuse_map = nullptr;
			//DebugBreak(); // fix code below
			//if (!diffuse_map->m_parent) {
			//	GetTracker().untrack(diffuse_map);
			//	delete diffuse_map;
			//	diffuse_map = nullptr;
			//}
		}
	}
	bool MaterialAsset::valid() const {
		if (diffuse_map)
			return  m_state == Loaded &&diffuse_map->getState() == Loaded;
		return m_state == Loaded;
	}
	Asset::LoadFlags MaterialAsset::load(LoadFlags flags) {
		LoadFlags out = 0;
		// free/reset data?
		if ((flags & LoadIO) && !m_error && !m_path.empty()) {
			std::string root = CreateRoot(m_path);

			FileReader file(m_path);
			try {
				std::string diffuse_mapName;
				file.read(&diffuse_mapName);
				//std::cout << "Path: "<< root << diffuse_mapName << "\n";
				if (diffuse_mapName.length() != 0) {
					if (m_storage) {
						diffuse_map = m_storage->load<TextureAsset>(root + diffuse_mapName);
					} else {
						diffuse_map = ALLOC_NEW(TextureAsset)();
						diffuse_map->loadPath(root + diffuse_mapName, LoadAll);
						// GetTracker().track(diffuse_map);
					}
				}

				//std::cout << "Diffuse "<<diffuse_map << "\n";

				file.read(&diffuse_color);
				file.read(&specular);
				file.read(&shininess);

				//std::cout << diffuse_color[0] << " " << diffuse_color[1] << " " << diffuse_color[2] << "\n";
				m_state = Loaded;
			}
			catch (Error err) {
				m_error = err;
				m_state = Failed;
				//Logging({ "AssetManager","Material",toString(err) + ": " + path }, LogStatus::Error);
			}
			out = 0;
		}
		// add diffuse_map as a texture asset
		return out;
	}
	static MaterialAsset* loaded[4];
	void MaterialAsset::bind(Shader* shader, int index) {
		if (m_state != Loaded) return;

		if (shader != nullptr) {

			//if (this == loaded[index]) return;
			loaded[index] = this;

			if (diffuse_map != nullptr) {
				diffuse_map->texture.bind(index + 1);
				//BindTexture(index + 1, material->diffuse_map);// + 1 because of shadow_map on 0
				//std::cout << "PassMaterial - texture not bound!\n";
				shader->setInt("uMaterials[" + std::to_string(index) + "].diffuse_map", index + 1);
				shader->setInt("uMaterials[" + std::to_string(index) + "].useMap", 1);
			} else {
				shader->setInt("uMaterials[" + std::to_string(index) + "].useMap", 0);
			}
			shader->setVec3("uMaterials[" + std::to_string(index) + "].diffuse_color", diffuse_color);
			shader->setVec3("uMaterials[" + std::to_string(index) + "].specular", specular);
			shader->setFloat("uMaterials[" + std::to_string(index) + "].shininess", shininess);
		} else {
			//std::cout << "shader or material is nullptr in Passmaterial\n";
		}
	}
	Keyframe::Keyframe(PolationType polation, unsigned short frame, float value)
		: polation(polation), frame(frame), value(value) 	{ }
	FCurve::FCurve() {}
	Keyframe& FCurve::get(int index)
	{
		return frames.at(index);
	}
	void FCurve::add(Keyframe keyframe)
	{
		frames.push_back(keyframe);
	}
	FCurve& Channels::get(ChannelType channel)
	{
		return fcurves[channel];
	}
	void Channels::add(ChannelType channel, FCurve fcurve)
	{
		fcurves[channel] = fcurve;
	}
	void Channels::getValues(int frame, float blend, glm::vec3& pos, glm::vec3& euler, glm::vec3& scale, glm::mat4& quater, short* usedChannels)
	{
		float build[9]{ 0,0,0,0,0,0,1,1,1 };
		glm::quat q0 = glm::quat(1, 0, 0, 0);
		glm::quat q1 = glm::quat(1, 0, 0, 0);

		// q0[3] == q0[].w == 1

		float slerpT = 0;
		for (auto& pair : fcurves) {
			auto& type = pair.first;
			auto& curve = pair.second;
				
			Keyframe* a = nullptr;
			Keyframe* b = nullptr;
			for (Keyframe& k : curve.frames) {
				if (k.frame == frame) { // the keyframe is at the current time frame.
					a = &k;
					break;
				}
				if (k.frame < frame) { // set a to the keyframe below the time frame at this moment in time.
					a = &k;
					continue;
				}
				if (k.frame > frame) { // set b to the keyframe directly after the time frame.
					if (a == nullptr)
						a = &k;
					else
						b = &k;
					break;
				}
			}
			if (a == nullptr && b == nullptr) { // no keyframes
				//std::cout << "no frames "<< type <<" " << curve.second.frames.size() << std::endl;
				continue;
			}
			if (b == nullptr) { // one keyframe ( "a" )
				//std::cout << "one frame "<<a->frame << std::endl;
				if (type > ScaZ) {
					slerpT = 0;
					q0[type - QuaX] = a->value;
					*usedChannels = *usedChannels | (1 << type);
					continue;
				}
				else {
					build[type] = a->value;
					*usedChannels = *usedChannels | (1 << type);
					continue;
				}
			}
			//log::out << a->frame << " " << b->frame << "\n";
			// lerping between to keyframes
			float lerp = 0;
			if (a->polation == Constant) {
				lerp = 0;
			}
			else if (a->polation == Linear) {
				lerp = ((float)frame - a->frame) / (b->frame - a->frame);
			}
			else if (a->polation == Bezier) {
				lerp = bezier((float)frame, a->frame, b->frame);
			}

			if (type > ScaZ) {
				slerpT = lerp;
				q0[type - QuaX] = a->value;
				q1[type - QuaX] = b->value;
				*usedChannels = *usedChannels | (1 << type);
			}
			else {
				build[type] = a->value * (1 - lerp) + b->value * lerp;
				*usedChannels = *usedChannels | (1 << type);
			}
		}

		pos.x += build[0] * blend;
		pos.y += build[1] * blend;
		pos.z += build[2] * blend;

		euler.x += build[3] * blend;
		euler.y += build[4] * blend;
		euler.z += build[5] * blend;

		scale.x *= build[6] * blend + (1 - blend);
		scale.y *= build[7] * blend + (1 - blend);
		scale.z *= build[8] * blend + (1 - blend);

		//glm::mat4 matis = glm::mat4_cast(glm::slerp(glm::quat(0, 0, 0, 1),glm::slerp(q0, q1, slerpT), 1.f));
		//glm::mat4 matis = glm::mat4_cast(glm::slerp(q0, q1, slerpT));
		//bug::out < matis<"\n";
		//quater *= glm::slerp(q0, q1, slerpT);

		//log::out << q0 <<" - "<<q1 << "\n";

		//std::cout << slerpT << std::endl;

		//bug::outs < "b " <build[1] < "\n";

		//log::out << slerpT << "\n";

		//quater *= glm::toMat4(q0);

		if (q0 == q1 || slerpT == 0) {
			quater *= glm::mat4_cast(glm::normalize(glm::slerp(glm::quat(1, 0, 0, 0), q0, blend)));
		}
		else if (slerpT == 1) {
			quater *= glm::mat4_cast(glm::normalize(glm::slerp(glm::quat(1, 0, 0, 0), q1, blend)));
		}
		else {// Expensive. Is it possible to optimize?
			//log::out << "blend "<<q0 << " "<<q1 <<" "<<blend << "\n";
			quater *= glm::mat4_cast(glm::normalize(glm::slerp(glm::quat(1, 0, 0, 0), glm::slerp(q0, q1, slerpT), blend)));
		}
	}
	Channels* AnimationAsset::get(unsigned short i) {
		if (m_state != Loaded) return nullptr;
		return &objects[i];
	}
	void AnimationAsset::cleanup() {

		log::out <<log::RED<< "AnimationAsset::cleanup - not implemented\n";
	}
	//void AnimationAsset::addObjectChannels(int objectIndex, Channels channels) {
	//	objects[objectIndex] = channels;
	//}
	//void AnimationAsset::modify(unsigned short startFrame, unsigned short endFrame) 	{
	//	frameStart = startFrame;
	//	frameEnd = endFrame;
	//}
	//void AnimationAsset::modify(unsigned short startFrame, unsigned short endFrame, float speed) {
	//	frameStart = startFrame;
	//	frameEnd = endFrame;
	//	defaultSpeed = speed;
	//}
	Asset::LoadFlags AnimationAsset::load(LoadFlags flags) {
		LoadFlags out = 0;
		if ((flags & LoadIO) && !m_error && !m_path.empty()) {
			// clear data
			// clear data in side channels?
			objects.clear();

			FileReader file(m_path);
			try {
				file.read(&frameStart);
				file.read(&frameEnd);
				file.read(&defaultSpeed);

				uint8_t objectCount;
				file.read(&objectCount);

				//log::out << "obs " << objectCount << "\n";

				for (uint32_t i = 0; i < objectCount; ++i) {
					uint16_t index, curves;

					file.read(&index);
					file.read(&curves);

					//log::out << "index " << index << "\n";
					//log::out << "curve " << curves << "\n";

					bool curveB[13]{ 0,0,0,0,0,0,0,0,0,0,0,0,0 };
					for (int j = 12; j >= 0; --j) {
						if (0 <= curves - (uint16_t)pow(2, j)) {
							curves -= (uint16_t)pow(2, j);
							curveB[j] = 1;
						}
						else {
							curveB[j] = 0;
						}
					}

					objects[index] = Channels();
					Channels* channels = &objects[index];

					//const std::string curve_order[]{ "PX","PY","PZ","RX","RY","RZ","SX","SY","SZ","QX","QY","QZ","QW" };
					for (ChannelType cha = PosX; cha < 13; cha = (ChannelType)(cha + 1)) {
						if (curveB[cha]) {
							uint16_t keys;
							file.read(&keys);

							channels->fcurves[cha] = FCurve();
							FCurve* fcurve = &channels->fcurves[cha];

							for (uint32_t k = 0; k < keys; ++k) {
								PolationType polation;
								file.read((u8*)&polation); // 1 byte

								uint16_t frame;
								file.read(&frame);

								float value;
								file.read(&value);

								fcurve->frames.push_back(Keyframe(polation, frame, value));
							}
						}
					}
				}
				out = 0;
				m_state = Loaded;
			} catch (Error err) {
				m_error = err;
				m_state = Failed;
				//Logging({ "AssetManager","Animation",toString(err) + ": " + path }, LogStatus::Error);
			}
		}
		return out;
	}
	void MeshAsset::cleanup() {
		for (int i = 0; i < materials.size(); i++) {
			MaterialAsset* mat = materials[i];
			if (!mat->getStorage()) {
				// GetTracker().untrack(mat);
				ALLOC_DELETE(MaterialAsset, mat);
			} else {
				// material belong to storage
			}
		}
		materials.clear();
		// cleanup should be called on VBO, IBO and VAO.
	}
	bool MeshAsset::valid() const {
		if (m_state != Loaded) return false;
		// materials are done loading if state is loaded
		for (int i = 0; i < materials.size(); i++) {
			if (materials[i]->getState() != Loaded) // could be error
				return false;
		}
		return true;
	}
	Asset::LoadFlags MeshAsset::load(LoadFlags flags) {
		LoadFlags out = 0;

		// NOTE: if possible, clean up this function a bit. There are memory allocations everywhere and a lot of things going on.
		
		if ((flags & LoadIO) && !m_error && !m_path.empty()) {
			// clear data
			//buffer.Uninit();
			materials.clear();
		
			std::string root = CreateRoot(m_path);
			

			points = nullptr;
			colors = nullptr;
			weightIndices = nullptr;
			weightValues = nullptr;
			triangles = nullptr;
			vertexOut = nullptr;
			triangleOut = nullptr;

			FileReader file(m_path);
			if(!file.isOpen()){
				m_error = file.getError();
				m_state = Failed;
				return 0;
			}
			if(false){
				MESH_LOAD_FAILED:
					m_error = file.getError();
					m_state = Failed;
					Free(points, pointSize);
					Free(colors, colorSize);
					Free(weightIndices, weightIndicesSize);
					Free(weightValues, weightValuesSize);
					Free(triangles, trianglesSize);
					points = nullptr;
					colors = nullptr;
					weightIndices = nullptr;
					weightValues = nullptr;
					triangles = nullptr;
					return 0;
			}
		
			#define MESH_LOAD_CHECK(x) if(!(x)) goto MESH_LOAD_FAILED;
			uint8_t materialCount;
			MESH_LOAD_CHECK(file.read((u8*)&meshType))
			MESH_LOAD_CHECK(file.read(&pointCount))
			MESH_LOAD_CHECK(file.read(&colorCount))
			MESH_LOAD_CHECK(file.read(&materialCount))
			
			//std::cout << "uhu\n";
			for (uint32_t i = 0; i < materialCount && i < MeshAsset::maxMaterials; ++i) {
				std::string materialName;
				MESH_LOAD_CHECK(file.read(&materialName))

				//std::cout << "Matloc: " << root<<materialName<< "\n";
				MaterialAsset* asset;
				if (m_storage) {
					asset = m_storage->load<MaterialAsset>(root + materialName);
					//asset = m_parent->set<MaterialAsset>(root + materialName);
				} else {
					asset = ALLOC_NEW(MaterialAsset)();
					asset->loadPath(root + materialName,LoadAll);
					// GetTracker().track(asset);
				}
				if (asset)
					materials.push_back(asset);
				else {

				}

				//std::cout << materials.back()->error << " err\n";
			}
			if (materialCount == 0) {
				MaterialAsset* asset;
				if (m_storage) {
					// non-disc thing
					asset = m_storage->get<MaterialAsset>("defaultMaterial");
					if (!asset) {
						asset = ALLOC_NEW(MaterialAsset)();
						m_storage->set<MaterialAsset>("defaultMaterial", asset);
					}
				} else {
					asset = ALLOC_NEW(MaterialAsset)();
					// GetTracker().track(asset);
				}
				materials.push_back(asset);
			}

			weightCount = 0;
			if (meshType == MeshType::Boned) {
				MESH_LOAD_CHECK(file.read(&weightCount))
			}
			//uint16_t triangleCount;
			MESH_LOAD_CHECK(file.read(&triangleCount))

			//std::cout << "Points " << pointCount << " Textures " << textureCount <<" Triangles: "<<triangleCount<<" Weights "<<weightCount<<" Mats " << (int)materialCount << "\n";
			pointNumbers = 3 * pointCount;
			pointSize = sizeof(float) * pointNumbers;
			points = (float*)Allocate(pointSize); // using malloc because i want to track allocations. I would use the new keyword otherwise.
			MESH_LOAD_CHECK(file.read(points, pointNumbers))

			colorNumbers = colorCount * 3;
			colorSize = sizeof(float) * colorNumbers;
			colors = (float*)Allocate(colorSize);
			MESH_LOAD_CHECK(file.read(colors, colorNumbers))

			// Weight
			weightNumbers = weightCount * 3;
			weightIndicesSize = sizeof(int) * weightNumbers;
			weightIndices = (int*)Allocate(weightIndicesSize);
			weightValuesSize = sizeof(float) * weightNumbers;
			weightValues = (float*)Allocate(weightValuesSize);

			if (meshType == MeshType::Boned) {
				u8 index[3];
				float floats[3];
				for (uint32_t i = 0; i < weightCount; ++i) {
					MESH_LOAD_CHECK(file.read(index, 3))

					MESH_LOAD_CHECK(file.read(floats, 3))
					weightIndices[i * 3] = index[0];
					weightIndices[i * 3 + 1] = index[1];
					weightIndices[i * 3 + 2] = index[2];
					weightValues[i * 3] = floats[0];
					weightValues[i * 3 + 1] = floats[1];
					weightValues[i * 3 + 2] = floats[2];
				}
			}

			triangleStride = 6;
			if (meshType == MeshType::Boned)
				triangleStride = 9;

			triangleNumbers = triangleCount * triangleStride;
			trianglesSize = sizeof(uint16_t) * triangleNumbers;
			triangles = (uint16_t*)Allocate(trianglesSize);
			MESH_LOAD_CHECK(file.read(triangles, triangleNumbers))

			out = LoadData;
				
			// } catch (Error err) {
				// m_error = err;
				// m_state = Failed;
				// out = 0;
				// Free(points, pointSize);
				// Free(colors, colorSize);
				// Free(weightIndices, weightIndicesSize);
				// Free(weightValues, weightValuesSize);
				// Free(triangles, trianglesSize);
				// points = nullptr;
				// colors = nullptr;
				// weightIndices = nullptr;
				// weightValues = nullptr;
				// triangles = nullptr;
				//Logging({ "AssetManager","Mesh",toString(err) + ": " + path }, LogStatus::Error);
			// }
		}
		if ((flags & LoadData)&&!m_error) {
				
			// TODO: hardcoded numbers is bad. Change them.
			// find boundingPoint
			glm::vec3 minPoint = {999999,999999,999999};
			glm::vec3 maxPoint = {-999999,-999999,-999999};
			for (int i = 0; i < pointCount;i++) {
				glm::vec3 vec = { points[3 * i],points[3 * i + 1],points[3 * i + 2] };
				minPoint.x = minPoint.x < vec.x ? minPoint.x : vec.x;
				minPoint.y = minPoint.y < vec.y ? minPoint.y : vec.y;
				minPoint.z = minPoint.z < vec.z ? minPoint.z : vec.z;
				maxPoint.x = maxPoint.x > vec.x ? maxPoint.x : vec.x;
				maxPoint.y = maxPoint.y > vec.y ? maxPoint.y : vec.y;
				maxPoint.z = maxPoint.z > vec.z ? maxPoint.z : vec.z;
			}
			boundingPoint = (minPoint + maxPoint) / 2.f;
			// find boundingRadius
			boundingRadius = 0;
			for (int i = 0; i < pointCount; i++) {
				glm::vec3 vec = { points[3 * i],points[3 * i + 1],points[3 * i + 2] };
				float radius = glm::length(vec-boundingPoint);
				if (radius>boundingRadius) {
					boundingRadius = radius;
				}
			}

			//log::out << "Mesh " << getLoadName() << " " << minPoint << " " << maxPoint << "\n";

			std::vector<uint16_t> indexNormal;
			std::vector<float> uNormal;
			for (uint32_t i = 0; i < triangleCount; ++i) {
				for (uint32_t j = 0; j < 3; ++j) {
					if (triangles[i * triangleStride + j * triangleStride / 3] * 3u + 2u >= pointNumbers) {
						//std::cout << "Corruption at '" << i <<" "<<j<<" " << (i * triangleStride) << " " << (j * triangleStride / 3) << " " << triangles[i * triangleStride + j * triangleStride / 3] << "' : Triangle Index\n";
						Assert(("Corrupt file, error stuff not present, doing assert instead",false));
						// throw ErrorCorruptedFile;
					}
				}
				glm::vec3 p0(points[triangles[i * triangleStride + 0 * triangleStride / 3] * 3 + 0], points[triangles[i * triangleStride + 0 * triangleStride / 3] * 3 + 1], points[triangles[i * triangleStride + 0 * triangleStride / 3] * 3 + 2]);
				glm::vec3 p1(points[triangles[i * triangleStride + 1 * triangleStride / 3] * 3 + 0], points[triangles[i * triangleStride + 1 * triangleStride / 3] * 3 + 1], points[triangles[i * triangleStride + 1 * triangleStride / 3] * 3 + 2]);
				glm::vec3 p2(points[triangles[i * triangleStride + 2 * triangleStride / 3] * 3 + 0], points[triangles[i * triangleStride + 2 * triangleStride / 3] * 3 + 1], points[triangles[i * triangleStride + 2 * triangleStride / 3] * 3 + 2]);
				//std::cout << p0.x << " " << p0.y << " " << p0.z << std::endl;
				//std::cout << p1.x << " " << p1.y << " " << p1.z << std::endl;
				//std::cout << p2.x << " " << p2.y << " " << p2.z << std::endl;
				glm::vec3 cro = glm::cross(p1 - p0, p2 - p0);
				//std::cout << cro.x << " " << cro.y << " " << cro.z << std::endl;
				glm::vec3 norm = glm::normalize(cro);
				//std::cout << norm.x << " " << norm.y << " " << norm.z << std::endl;

				bool same = false;
				for (uint32_t j = 0; j < uNormal.size() / 3; ++j) {
					if (uNormal[j * 3 + 0] == norm.x && uNormal[j * 3 + 1] == norm.y && uNormal[j * 3 + 2] == norm.z) {
						same = true;
						indexNormal.push_back((uint16_t)j);
						break;
					}
				}
				if (!same) {
					uNormal.push_back(norm.x);
					uNormal.push_back(norm.y);
					uNormal.push_back(norm.z);
					indexNormal.push_back((uint16_t)(uNormal.size() / 3 - 1));
				}
			}

			std::vector<unsigned short> uniqueVertex;// [ posIndex,colorIndex,normalIndex,weightIndex, ...]

			triangleOutSize = sizeof(uint32_t) * triangleCount * 3;
			triangleOut = (uint32_t*)Allocate(triangleOutSize);

			uint32_t uvStride = 1 + (triangleStride) / 3;
			for (uint32_t i = 0; i < triangleCount; ++i) {
				for (uint32_t t = 0; t < 3; ++t) {
					bool same = false;
					for (uint32_t v = 0; v < uniqueVertex.size() / (uvStride); ++v) {
						if (uniqueVertex[v * uvStride] != triangles[i * triangleStride + 0 + t * triangleStride / 3])
							continue;
						if (uniqueVertex[v * uvStride + 1] != indexNormal[i])
							continue;
						if (uniqueVertex[v * uvStride + 2] != triangles[i * triangleStride + 1 + t * triangleStride / 3])
							continue;
						if (meshType == MeshType::Boned) {
							if (uniqueVertex[v * uvStride + 3] != triangles[i * triangleStride + 2 + t * triangleStride / 3])
								continue;
						}
						same = true;
						triangleOut[i * 3 + t] = v;
						break;
					}
					if (!same) {
						triangleOut[i * 3 + t] = uniqueVertex.size() / (uvStride);

						uniqueVertex.push_back(triangles[i * triangleStride + 0 + t * triangleStride / 3]);
						uniqueVertex.push_back(indexNormal[i]);
						uniqueVertex.push_back(triangles[i * triangleStride + 1 + t * triangleStride / 3]);
						if (meshType == MeshType::Boned) {
							uniqueVertex.push_back(triangles[i * triangleStride + 2 + t * triangleStride / 3]);
						}
					}
				}
			}

			uint32_t vStride = 3 + 3 + 3;
			if (meshType == MeshType::Boned)
				vStride += 6;

			vertexOutSize = sizeof(float) * (uniqueVertex.size() / uvStride) * vStride;
			vertexOut = (float*)Allocate(vertexOutSize);

			for (uint32_t i = 0; i < uniqueVertex.size() / uvStride; i++) {
				// Position
				for (uint32_t j = 0; j < 3; ++j) {
					if (uniqueVertex[i * uvStride] * 3 + j > pointNumbers) {
						//bug::out < bug::RED < "Corruption at '" < path < "' : Position Index\n";
						Assert(false);
					}
					vertexOut[i * vStride + j] = points[uniqueVertex[i * uvStride] * 3 + j];
				}
				// Normal
				for (uint32_t j = 0; j < 3; ++j) {
					if (uniqueVertex[i * uvStride + 1] * 3 + j > uNormal.size()) {
						//bug::out < bug::RED < "Corruption at '" < path < "' : Normal Index\n";
						Assert(false);
					}
					vertexOut[i * vStride + 3 + j] = uNormal[uniqueVertex[i * uvStride + 1] * 3 + j];
				}
				// UV
				for (uint32_t j = 0; j < 3; ++j) {
					if (uniqueVertex[i * uvStride + 2] * 3 + j > colorNumbers) {
						//bug::out < bug::RED < "Corruption at '" < path < "' : Color Index\n";
						//bug::out < (uniqueVertex[i * uvStride + 2] * 3 + j) < " > " < uTextureSize < bug::end;
						Assert(false);
					} else
						vertexOut[i * vStride + 3 + 3 + j] = (float)colors[uniqueVertex[i * uvStride + 2] * 3 + j];
				}
				if (meshType == MeshType::Boned) {
					// Bone Index
					for (uint32_t j = 0; j < 3; ++j) {
						if (uniqueVertex[i * uvStride + 3] * 3 + j > weightNumbers) {
							//bug::out < bug::RED < "Corruption at '" < path < "' : Bone Index\n";
							Assert(false);
						}
						vertexOut[i * vStride + 3 + 3 + 3 + j] = (float)weightIndices[uniqueVertex[i * uvStride + 3] * 3 + j];
					}
					// Weight
					for (uint32_t j = 0; j < 3; ++j) {
						if (uniqueVertex[i * uvStride + 3] * 3 + j > weightNumbers) {
							//bug::out < bug::RED < "Corruption at '" < path < "' : Weight Index\n";
							Assert(false);
						}
						vertexOut[i * vStride + 3 + 3 + 3 + 3 + j] = (float)weightValues[uniqueVertex[i * uvStride + 3] * 3 + j];
					}
				}
			}
			out = LoadGraphic;
			// } catch (Error err) {
			// 	m_error = err;
			// 	m_state = Failed;
			// 	out = 0;
			// 	//m_state = Failed;
			// 	//Logging({ "AssetManager","Mesh",toString(err) + ": " + path }, LogStatus::Error);
			// 	Free(vertexOut, vertexOutSize);
			// 	Free(triangleOut, triangleOutSize);
			// 	vertexOut = nullptr;
			// 	triangleOut = nullptr;
			// }
			// Cleanup - not the stuff needed for OpenGL
			Free(points, pointSize);
			Free(colors, colorSize);
			Free(weightIndices, weightIndicesSize);
			Free(weightValues, weightValuesSize);
			Free(triangles, trianglesSize);
			points = nullptr;
			colors = nullptr;
			weightIndices = nullptr;
			weightValues = nullptr;
			triangles = nullptr;
		}
		if ((flags & LoadGraphic) && !m_error) {
			vertexBuffer.setData(vertexOutSize, vertexOut);
			indexBuffer.setData(triangleOutSize, triangleOut);

			vertexArray.clear(); // reset previous attributes
			vertexArray.addAttribute(3);// Position
			vertexArray.addAttribute(3);// Normal
			if (meshType == MeshType::Boned) {
				vertexArray.addAttribute(3);// Color
				vertexArray.addAttribute(3);// Bone Index
				vertexArray.addAttribute(3, &vertexBuffer);// Weight
			} else {
				vertexArray.addAttribute(3, &vertexBuffer);// Color
				vertexArray.addAttribute(4, 1);
				vertexArray.addAttribute(4, 1);
				vertexArray.addAttribute(4, 1);
				vertexArray.addAttribute(4, 1, nullptr);// empty instance buffer	
			}
			// do some error checking if buffers failed?

			Free(vertexOut, vertexOutSize);
			Free(triangleOut, triangleOutSize);
			vertexOut = nullptr;
			triangleOut = nullptr;
			m_state = Loaded;
			out = 0;
		}
		return out;
	}
	void ColliderAsset::cleanup() {

	}
	Asset::LoadFlags ColliderAsset::load(LoadFlags flags) {
		LoadFlags out = 0;

		if ((flags&LoadIO) && !m_error && !m_path.empty()) {
			map.heights.clear();
			//map.heights.shrink_to_fit();
			FileReader file(m_path);
			try {
				file.read((u8*)&colliderType);

				//log::out << (int)colliderType << " type\n";

				switch (colliderType) {
				case CubeType: {
					file.read(&cube.size);
					break;
				}
				case SphereType: {
					file.read(&sphere.radius);
					break;
				}
				case MapType: {
					file.read(&map.gridColumns);
					file.read(&map.gridRows);
					file.read(&map.minHeight);
					file.read(&map.maxHeight);
					file.read(&map.scale);
					uint32_t count = map.gridColumns * map.gridRows;
					if (count > 1000000) {
						log::out << "Something may be corrupted\n"; // to prevent a corruption from allocating insane amounts of memory.
						// throw ErrorCorruptedFile;
						Assert(false);
					}
					map.heights.resize(count);
					file.read(map.heights.data(), count);

					//uint16_t triCount,pointCount;
					//file.read(&pointCount);
					//file.read(&triCount);
					//file.read(&furthest);

					///*if (triCount > 100000 || pointCount > 100000||furthest<0){
					//	log::out << "something may be corrupted\n";
					//	throw CorruptedData;
					//}*/

					//points.resize(pointCount);
					//tris.resize(triCount*3);
					//for (uint32_t i = 0; i < points.size();++i) {
					//	file.read(&points[i]);
					//}
					//for (uint32_t i = 0; i < tris.size(); ++i) {
					//	file.read(&tris[i]);
					//}

					break;
				}
				case CapsuleType: {
					file.read(&capsule.radius);
					file.read(&capsule.height);
					break;
				}
				}
				out = 0;
				m_state = Loaded;
			} catch (Error err) {
				m_error = err;
				out = 0;
				m_state = Failed;
				//Logging({ "AssetManager","Collider",toString(err) + ": " + path }, LogStatus::Error);
			}
		}
		return out;
	}
	void ArmatureAsset::cleanup() {

	}
	Asset::LoadFlags ArmatureAsset::load(LoadFlags flags) {
		LoadFlags out = 0;
		if ((flags & LoadIO) && !m_error && !m_path.empty()) {
			bones.clear();

			FileReader file(m_path);
			try {
				uint8_t boneCount;
				file.read(&boneCount);

				//log::out << path << "\n";
				//log::out << boneCount << "\n";

				// Acquire and Load Data
				for (uint32_t i = 0; i < boneCount; ++i) {
					Bone b;
					file.read(&b.parent);
					//log::out << b.parent << "\n";
					//log::out << i<<" matrix" << "\n";
					file.read(&b.localMat);
					file.read(&b.invModel);

					bones.push_back(b);
				}
				m_state = Loaded;
				out = 0;
			} catch (Error err) {
				m_error = err;
				out = 0;
				m_state = Failed;
				//Logging({ "AssetManager","Armature",toString(err) + ": " + path }, LogStatus::Error);
			}
		}
		return out;
	}
	void ModelAsset::cleanup() {
		if (m_storage) {
			for (int i = 0; i < animations.size(); i++) {
				AnimationAsset* anim = animations[i];
				if (!anim->getStorage()) {
					// GetTracker().untrack(anim);
					//delete anim;
					ALLOC_DELETE(AnimationAsset, anim);
				}
			}
			for (int i = 0; i < instances.size(); i++) {
				AssetInstance& instance = instances[i];

			}
		}
		animations.clear();
		instances.clear();
	}
	bool ModelAsset::valid() const {
		if (m_state != Loaded) return false;
		for (int i = 0; i < instances.size();i++) {
			if (instances[i].asset->getState() != Loaded)
				return false;
		}
		for (int i = 0; i < animations.size(); i++) {
			if (animations[i]->getState() != Loaded)
				return false;
		}
		return true;
	}
	Asset::LoadFlags ModelAsset::load(LoadFlags flags) {
		LoadFlags out = 0;
		if ((flags & LoadIO) && !m_error && !m_path.empty()) {
			std::string root = CreateRoot(m_path);
			instances.clear();
			animations.clear();

			/*
			Load model with assets A and B
			Reload model with assets A and C
			Unload model. unloads A and C

			Asset B is not unloaded
			 During reload, unload assets which aren't used. load assets which are.
			Asset A and C may be used elsewhere.
			 Actually, because of the asset file structure, assets cannot be used elsewhere unless.
			 What blender exporter does is export individual models which aren't
			 connected in any way to other models.

			*/

			// ISSUE: cleanup would be the easy way to do reload but not the fastest.
			// The fastest would be to load model and compare old assets to 
			//  assets.
			//  and see which instances are the same
			// and not. Then do a reload

			FileReader file(m_path);
			// try {
				uint16_t instanceCount;
				file.read(&instanceCount);
				for (uint32_t i = 0; i < instanceCount; ++i) {
					instances.push_back({});
					AssetInstance& instance = instances.back();

					file.read(&instance.name);

					uint8_t instanceType;
					file.read(&instanceType);

					std::string name;
					file.read(&name);

					switch (instanceType) {
					case 0:
						if (m_storage) {
							// ISSUE: assets may be loaded twice
							// load asset if first time. don't reload asset for every instance. reload meshasset if model is reloaded
							// (what can't happen is model reloading multiple times, not necessary)
							// note that if model is slow, the mesh asset may have time to be loaded twice which is bad.
							// You could have a unique meshAsset vector and if mesh exist in there you wouldn't load it again.(or if the mehs name exists.)
							instance.asset = m_storage->load<MeshAsset>(root + name); // maybe this is fine?
							//instance.asset = m_parent->set<MeshAsset>(root + name);
						} else {
							instance.asset = ALLOC_NEW(MeshAsset)();
							instance.asset->loadPath(root + name,LoadAll);
							// GetTracker().track((MeshAsset*)instance.asset);
						}
						break;
					case 1:
						if (m_storage) {
							instance.asset = m_storage->load<ArmatureAsset>(root + name);
						} else {
							instance.asset = ALLOC_NEW(ArmatureAsset)();
							instance.asset->loadPath(root + name, LoadAll);
							// GetTracker().track((ArmatureAsset*)instance.asset);
						}
						break;
					case 2:
						if (m_storage) {
							instance.asset = m_storage->load<ColliderAsset>(root + name);
						} else {
							instance.asset = ALLOC_NEW(ColliderAsset)();
							instance.asset->loadPath(root + name, LoadAll);
							// GetTracker().track((ColliderAsset*)instance.asset);
						}
						break;
					}

					file.read(&instance.parent);
					file.read(&instance.localMat);
					//file.read(&instance.invModel);
				}

				uint16_t animationCount;
				file.read(&animationCount);
				for (uint32_t i = 0; i < animationCount; ++i) {
					std::string name;
					file.read(&name);
					if (m_storage) {
						AnimationAsset* ass = m_storage->load<AnimationAsset>(root + name);
						animations.push_back(ass);
					} else {
						// ISSUE: Memory leak, this is not owned by assets, since assets doesn't exist, but it is still created.
						// ModelAsset should destroy it but how would it know if it has "ownership". vector with bool for each animations?
						AnimationAsset* as = ALLOC_NEW(AnimationAsset)();
						as->loadPath(root + name, LoadAll);
						animations.push_back(as);
					}
				}
				out = LoadData;
				//m_state = Loaded; // doesn't have any opengl stuff
				//out = 0;
			// } catch (Error err) {
			// 	m_error = err;
			// 	out = 0;
			// 	m_state = Failed;
			// 	//Logging({ "AssetManager","Model",toString(err) + ": " + path }, LogStatus::Error);
			// }
		} else if ((flags & LoadData) && !m_error) {
			// check if assets are fully loaded
			bool ready = true;
			for (AssetInstance& inst : instances) {
				if (inst.asset && inst.asset->type == MeshAsset::TYPE) {
					if (inst.asset->getState() != Asset::Loaded) {
						ready = false;
						break;
					}
				}
			}
			if (ready) {
				m_state = Loaded; // Needs to be loaded for getParentTransforms to work
				out = 0;

				struct InstSphere {
					glm::vec3 centerPoint;
					float radius;
				};

				std::vector<InstSphere> spheres;

				minPoint = { 999999, 999999, 999999 };
				maxPoint = {-999999,-999999,-999999};

				auto transforms = getParentTransforms(nullptr);

				for (int i = 0; i < instances.size(); i++) {
					AssetInstance& inst = instances[i];
					if (inst.asset && inst.asset->type == MeshAsset::TYPE) {
						// TODO: Not considering armature meshes. They may work with this code too but maybe not.
						MeshAsset* mesh = (MeshAsset*)inst.asset;
						
						glm::vec3 instBoundingPoint = (transforms[i] * inst.localMat * glm::translate(mesh->boundingPoint))[3];
						glm::vec3 instPointWithRadius = (transforms[i] * inst.localMat * glm::translate(mesh->boundingPoint + glm::vec3(mesh->boundingRadius, 0.f, 0.f)))[3];
						float newRadius = glm::length(instPointWithRadius - instBoundingPoint);

						spheres.push_back({ instBoundingPoint,newRadius });

						glm::vec3 testPoints[]{
							instBoundingPoint + glm::vec3(newRadius,0.f,0.f),
							instBoundingPoint + glm::vec3(-newRadius,0.f,0.f),
							instBoundingPoint + glm::vec3(0.f,newRadius,0.f),
							instBoundingPoint + glm::vec3(0.f,-newRadius,0.f),
							instBoundingPoint + glm::vec3(0.f,0.f,newRadius),
							instBoundingPoint + glm::vec3(0.f,0.f,-newRadius) };

						for (glm::vec3& point : testPoints) {
							minPoint.x = minPoint.x < point.x ? minPoint.x : point.x;
							minPoint.y = minPoint.y < point.y ? minPoint.y : point.y;
							minPoint.z = minPoint.z < point.z ? minPoint.z : point.z;
							maxPoint.x = maxPoint.x > point.x ? maxPoint.x : point.x;
							maxPoint.y = maxPoint.y > point.y ? maxPoint.y : point.y;
							maxPoint.z = maxPoint.z > point.z ? maxPoint.z : point.z;
						}
					}
				}
				boundingPoint = (minPoint + maxPoint) / 2.f;

				boundingRadius = 0;
				for (InstSphere& sphere : spheres) {
					float boundPointToSphereCenter = glm::length(sphere.centerPoint - boundingPoint);
					float radius = boundPointToSphereCenter + sphere.radius;
					if (radius > boundingRadius) {
						boundingRadius = radius;
					}
				}
			} else {
				// not ready, do more work later
				out = LoadData;
			}
		}
		return out;
	}
	std::vector<glm::mat4> ModelAsset::getParentTransforms(Animator* animator) {
		if (m_state != Loaded) return {};

		std::vector<glm::mat4> mats(instances.size());
		std::vector<glm::mat4> modelT(instances.size());

		for (uint32_t i = 0; i < instances.size(); ++i) {
			AssetInstance& instance = instances[i];
			glm::mat4 loc = instances[i].localMat;
			glm::mat4 inv = instances[i].invModel;

			glm::vec3 pos = { 0,0,0 };
			glm::vec3 euler = { 0,0,0 };
			glm::vec3 scale = { 1,1,1 };
			glm::mat4 quater = glm::mat4(1);

			short usedChannels = 0;

			if (animator) {
				for (uint32_t k = 0; k < Animator::maxAnimations; ++k) {
					if (animator->enabledAnimations[k].asset) {
						AnimatorProperty& prop = animator->enabledAnimations[k];
						for (uint32_t j = 0; j < animations.size(); ++j) {
							AnimationAsset* animation = animations[j];
							//log::out << "if " << prop.animationName <<" == "<<animation->baseName<<" & "<<prop.instanceName<<" == " <<instance.name<< "\n";
							if (prop.asset == animation &&
								prop.instanceName == instance.name) {

								if (animation->objects.count(0) > 0) {// the object/instance uses transform object

									//log::out << "inst " << i << "\n";
									animation->objects[0].getValues((int)prop.frame, prop.blend,
										pos, euler, scale, quater, &usedChannels);
									//log::out << " "<<pos.y <<" " << i << " " << k << " " << j << "\n";
								}
							}
						}
					}
				}
			}

			// Hello programmer! If you have issues where the matrix of an object is offset,
			//  you should clear parent inverse in blender. The offset is visible
			//  when using bpy.context.object.matrix_local. It doesn't show up in the viewport.

			// Hello again! Current issue here is the local matrix and animation colliding with each other.
			//  If there is an animation for a channel, then the local matrix should be ignored.

			/*
			for (int i = 0; i < 3;i++) {
				if (!((usedChannels<<i) & 1)) {
					pos[i] = loc[3][i];
				}
			}
			// Euler?
			for (int i = 0; i < 3; i++) {
				if (!((usedChannels << (6 + i)) & 1)) {
					scale[i] = glm::length(glm::vec3(loc[i]));
				}
			}

			const glm::mat3 rotMtx(
					glm::vec3(loc[0]) / scale[0],
					glm::vec3(loc[1]) / scale[1],
					glm::vec3(loc[2]) / scale[2]);
			glm::quat locQuat = glm::quat_cast(rotMtx);

			const glm::mat3 rotMtx2(
				glm::vec3(quater[0]) / scale[0],
				glm::vec3(quater[1]) / scale[1],
				glm::vec3(quater[2]) / scale[2]);
			glm::quat quat = glm::quat_cast(rotMtx2);

			for (int i = 0; i < 4; i++) {
				if (!((usedChannels << (9 + i)) & 1)) {
					quat[i] = (locQuat[i]);
				}
			}

			glm::mat4 dostuff = quater;//glm::mat4_cast(quat);
			*/
		
			glm::mat4 ani = glm::mat4(1);
			if (animator) {
				ani = glm::translate(glm::mat4(1), pos)
					* quater
					//* glm::rotate(euler.x, glm::vec3(1, 0, 0))
					//* glm::rotate(euler.z, glm::vec3(0, 0, 1))
					//* glm::rotate(euler.y, glm::vec3(0, 1, 0))
					* glm::scale(scale)
					;
			}

			if (instances[i].parent == -1) {
				modelT[i] = (loc * ani);
				mats[i] = (ani);
				//log::out << loc <<" "<<i << "\n";
			}
			else {
				modelT[i] = modelT[instances[i].parent] * (loc * ani);
				mats[i] = (modelT[instances[i].parent] * (ani));
				//log::out << loc << " x " << i << "\n";
			}

			//mats[i] = (modelT[i]);
		}
		return mats;
	}
	std::vector<glm::mat4> ModelAsset::getArmatureTransforms(Animator* animator, glm::mat4& instanceMat, AssetInstance* instance, ArmatureAsset* armature, std::vector<glm::mat4>* boneMats) {
		if (m_state != Loaded) return {};

		std::vector<glm::mat4> mats(armature->bones.size());
		if (armature != nullptr) {
			std::vector<glm::mat4> modelT(armature->bones.size());

			for (uint32_t i = 0; i < armature->bones.size(); ++i) {
				Bone& bone = armature->bones[i];
				glm::mat4 loc = bone.localMat;
				glm::mat4 inv = bone.invModel;
				glm::vec3 pos = { 0,0,0 };
				glm::vec3 euler = { 0,0,0 };
				glm::vec3 scale = { 1,1,1 };
				glm::mat4 quater = glm::mat4(1);

				short usedChannels = 0;
				if (animator) {
					for (uint32_t k = 0; k < Animator::maxAnimations; ++k) {
						if (animator->enabledAnimations[k].asset) {

							AnimatorProperty& prop = animator->enabledAnimations[k];
							for (uint32_t j = 0; j < animations.size(); ++j) {
								AnimationAsset* animation = animations[j];

								if (prop.asset == animation &&
									prop.instanceName == instance->name) {

									if (animation->objects.count((uint16_t)i) > 0u) {

										animation->objects[(uint16_t)i].getValues((int)prop.frame, prop.blend,
											pos, euler, scale, quater, &usedChannels);
										//log::out << " " << pos.y << " " << i << " " << k << " " << j << "\n";
										//log::out << quater<<"\n";
									}
								}
							}
						}
					}
				}

				/*
				for (int i = 0; i < 3; i++) {
					if (!((usedChannels << i) & 1)) {
						pos[i] = loc[3][i];
					}
				}
				// Euler?
				for (int i = 0; i < 3; i++) {
					if (!((usedChannels << (6 + i)) & 1)) {
						scale[i] = glm::length(glm::vec3(loc[i]));
					}
				}

				const glm::mat3 rotMtx(
					glm::vec3(loc[0]) / scale[0],
					glm::vec3(loc[1]) / scale[1],
					glm::vec3(loc[2]) / scale[2]);
				glm::quat locQuat = glm::quat_cast(rotMtx);

				const glm::mat3 rotMtx2(
					glm::vec3(quater[0]) / scale[0],
					glm::vec3(quater[1]) / scale[1],
					glm::vec3(quater[2]) / scale[2]);
				glm::quat quat = glm::quat_cast(rotMtx2);

				for (int i = 0; i < 4; i++) {
					if (!((usedChannels << (9 + i)) & 1)) {
						quat[i] = (locQuat[i]);
					}
				}*/

				//glm::mat4 dostuff = glm::mat4_cast(quat);

				glm::mat4 ani = glm::translate(glm::mat4(1), pos)
					* quater
					//* glm::rotate(euler.x, glm::vec3(1, 0, 0))
					//* glm::rotate(euler.z, glm::vec3(0, 0, 1))
					//* glm::rotate(euler.y, glm::vec3(0, 1, 0))
					* glm::scale(scale)
					;

				if (i == 0) {
					modelT[i] = (loc * ani);
					//log::out << loc << modelT[i] <<" "<< i << "\n";
				}
				else {
					modelT[i] = modelT[armature->bones[i].parent] * (loc * ani);
					//log::out << loc <<" x "<<i << "\n";
				}

				mats[i] = (modelT[i] * inv);
			}
			if (boneMats) {
				boneMats->resize(armature->bones.size());
				memcpy_s(boneMats->data(), boneMats->size() * sizeof(glm::mat4), modelT.data(), modelT.size() * sizeof(glm::mat4));
			}
		}
		return mats;
	}
}