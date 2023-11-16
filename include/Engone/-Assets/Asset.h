#pragma once

// #include "Engone/Utilities/Tracker.h"
#include "Engone/Logger.h"
#include "Engone/Error.h"

#include <condition_variable>

namespace engone {
	enum AssetType : uint8_t {
		AssetTexture = 0,
		AssetFont,
		AssetShader,
		AssetMaterial,
		AssetMesh,
		AssetAnimation,
		AssetArmature,
		AssetCollider,
		AssetModel,
	};

	const char* ToString(AssetType type);
	const char* GetFormat(AssetType type);
	Logger& operator<<(Logger& log, AssetType value);

	class AssetStorage;
	class Asset {
	public:
		AssetType type; // my type

		enum State : uint8_t {
			Waiting = 0, // nothing has happened yet.
			Processing = 1,
			Loaded = 2,
			Failed = 3,
		};
		enum LoadFlag : u8 {
			LoadNone = 0,
			LoadIO = 1,
			LoadData = 2,
			LoadGraphic = 4,
			LoadAll = (u8)-1,
		};
		typedef uint8_t LoadFlags;

		Asset(AssetType type) : type(type) {}
		virtual ~Asset() {}

		virtual void cleanup() = 0;
		// Same as load but with path.
		LoadFlags loadPath(const std::string& path, LoadFlags flags) {
			setPath(path);
			return load(flags);
		}
		// load specifics, returns the flags to process next.
		virtual LoadFlags load(LoadFlags flag) = 0;

		State getState() const { return m_state; }
		int getError() const { return m_error; }
		AssetStorage* getStorage() const { return m_storage; }

		// the storage which owns this asset
		// will not work if you have loaded data before calling this. (only works if m_state==None)
		inline void setStorage(AssetStorage* storage) {
			if(m_state==Waiting) // if statement is needed.
				m_storage = storage;
		}

		virtual bool valid() const {
			return m_state == Loaded;
		}
		// assets/...?
		inline const std::string& getPath() const { return m_path; }
		// only last part of name
		inline const std::string& getName() const { return m_name; }
		// name you entered when loading
		inline const std::string& getLoadName() const { return m_loadName; }

		void setPath(const std::string& path) {
			if (m_state == Waiting) {
				m_path = path;
				m_name = CreateName(m_path);
			}
		}
		void setLoadName(const std::string& name) {
			if (m_state == Waiting) {
				m_loadName = name;
			}
		}

		// gets name from path
		static std::string CreateName(const std::string& path);
		// gets root from path
		static std::string CreateRoot(const std::string& path);

		// static TrackerId GetTrackerId(AssetType type);

	protected:
		AssetStorage* m_storage = nullptr;

		State m_state = Waiting; // loaded, Processing, failed/error, also show error type
		LoadFlags m_flags = LoadNone;
		int m_error = 0;
		
		// I don't need three strings here, it's temporary untill I know what to do. 
		std::string m_path;
		std::string m_name;
		std::string m_loadName; // name used when loading
		 

		friend class AssetStorage;
		friend class AssetLoader;
		friend class AssetProcessor;
	};

	const char* ToString(Asset::LoadFlag type);
	Logger& operator<<(Logger& log, Asset::LoadFlag value);
}