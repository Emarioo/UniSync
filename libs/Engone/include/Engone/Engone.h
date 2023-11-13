#pragma once

// #include "Engone/RenderModule.h"
#include "Engone/Application.h"
#include "Engone/Window.h"

#include "Engone/PlatformModule/GameMemory.h"

// #include "Engone/SoundModule.h"
// #include "Engone/Utilities/Utilities.h"
// #include "Engone/Utilities/MemAllocator.h"
// #include "Engone/Networking/NetworkModule.h"

#include "Engone/Rendering/FrameBuffer.h"
#include "Engone/Rendering/Light.h"

// #include "Engone/Rendering/Camera.h"

// #include "Engone/Utilities/rp3d.h"
// #include "Engone/EngineObject.h"
// #include "Engone/Utilities/RuntimeStats.h"
// #include "Engone/ParticleModule.h"

namespace engone {
	typedef int(*InitProc)(void*,void*,void*);
	typedef int(*UpdateProc)(LoopInfo*);
	struct AppInstance {
		void* instanceData=0;
		InitProc initProc=0;
		UpdateProc preUpdateProc=0;
		UpdateProc postUpdateProc=0;
		
		int libraryId=0;
	};
	Engone* GetGlobalEngone();
	class Engone {
	public:
		Engone() = default;
		// ~Engone();
		static Engone* InitializeGlobal();
		static void DestroyGlobal();
		static void UseSharedGlobal(Engone* engone);
		void cleanup();

		enum EngoneFlag : uint32_t {
			EngoneNone = 0,
			EngoneShowHitboxes=1,
			EngoneEnableDebugInfo = 2, // not used
			EngoneShowBasicDebugInfo = 4, // not used
			EngoneRenderOnResize = 8,
		};
		typedef uint32_t EngoneFlags;

		// void add(AppInstance app);
		AppInstance* createApp();
		void stopApp(AppInstance* app);
		
		// Careful when calling this from DLL?
		Window* createWindow(WindowDetail detail);
		
		EngineWorld* createWorld();

		// returns an id which refers to the loaded library.
		// 0 means null.
		int addLibrary(const char* dllPath, const char* pdbPath);

		// I wouldn't recommend adding or removing apps from the vector.
		// Instead use createApplication or Application::stop if you want to add or remove an app.
		/*
		Start the engine loop which will run applications until their windows are closed.
		*/
		void start();

		// void manageThreading();
		void manageNonThreading();

		void renderApps();

		// the run time of stats
		//inline double getEngineTime() const { return m_runtimeStats.getRunTime(); }

		void addLight(Light* l);
		void removeLight(Light* l);

		inline EngoneFlags getFlags() const {
			return m_flags;
		}
		inline void setFlags(EngoneFlags flags) {
			m_flags = flags;
		}
		
		GameMemory* getGameMemory();
		Logger* getLogger();

		//RuntimeStats& getStats() { return m_runtimeStats; }
		
		void bindLights(Shader* shader, glm::vec3 objectPos);
		// returns current loop info
		LoopInfo& getLoopInfo();

		inline ExecutionTimer& getUpdateTimer() { return mainUpdateTimer; };
		inline ExecutionTimer& getRenderTimer() { return mainRenderTimer; };
		
		void saveGameMemory(const std::string& path);
		void loadGameMemory(const std::string& path);

		GameMemory gameMemory;
		Logger logger;
		
		UIRenderer* activeUIRenderer = nullptr;
		CommonRenderer* activeCommonRenderer = nullptr;
		Window* activeWindow = nullptr;
		// static TrackerId trackerId;
	private:
		//RuntimeStats m_runtimeStats;

		EngoneFlags m_flags;
		
		// delta uniform is set in engine loop.
		std::vector<Light*> m_lights;

		bool m_loadedDefault=false;

		FrameBuffer frameBuffer;
		VertexBuffer quadBuffer;
		VertexArray quadArray;

		// Apps use these when not multithreaded
		ExecutionTimer mainUpdateTimer;
		ExecutionTimer mainRenderTimer;

		LoopInfo currentLoopInfo;


		struct Library{
			const char* dllPath=0;	
			const char* pdbPath=0;
			double lastModified=0;
			void* libraryReference=0;
		};
		std::vector<Library> libraries;
		float codeReloadTime=0;

		std::vector<AppInstance*> removeApps;

		std::vector<AppInstance*> appInstances;
		std::vector<Window*> windows;
		std::vector<EngineWorld*> worlds;

		// Todo: instead of 3 vectors, use one vector<Tracker::TrackClass>
		std::vector<uint16_t> m_appSizes{}; // used for tracker
		// std::vector<TrackerId> m_appIds{}; // used for tracker

		// only renders objects at the moment
		void render(LoopInfo& info);
		void renderObjects(LoopInfo& info);
		
		
		
		friend class Window;
	};
	void SetupStaticVariables(Engone* engone);
	void CleanupStaticVariables();
}