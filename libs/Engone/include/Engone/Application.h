#pragma once

#include "Engone/Window.h"
// #include "Engone/AssetModule.h"
#include "Engone/LoopInfo.h"
#include "Engone/World/EngineWorld.h"

#include "Engone/Profiling/Profiler.h"
// #include "Engone/Utilities/Locks.h"
#include "Engone/PlatformModule/PlatformLayer.h"
#ifdef gone
namespace engone {

	// required in application
	class Engone;

	// maybe move somewhere else?
	class Application {
	public:
		Application(Engone* engone) : m_engone(engone) {}
		virtual ~Application() {
			cleanup();
		}
		// frees allocated memory that the application is responsible for.
		void cleanup();

		// these virtual functions should be = 0 but for test purposes they are not.

		virtual void update(LoopInfo& info) {};
		virtual void render(LoopInfo& info) {};

		// will be called when a window closes.
		virtual void onClose(Window* window) {};

		// will close all windows and close the app.
		// will not delete windows from the list instantly. This is done in the game loop.
		// function is somewhat asynchronous in this sense.
		void stop();

		inline Engone* getEngine() const { return m_engone; }

		// Will create a window which is attached to this application.
		// detail will determine what kind properties the window will have.
		// Window will be deleted when needed.
		Window* createWindow(WindowDetail detail = {});
		inline std::vector<Window*>& getAttachedWindows() { return m_windows; }
		// Will return nullptr if index is out of bounds.
		inline Window* getWindow(uint32_t index = 0) { if (index < m_windows.size()) return m_windows[index]; return nullptr; }
		
		// the first created windows assets
		inline AssetStorage* getStorage() {
			if (m_windows.size() != 0) return m_windows[0]->getStorage();
			return nullptr;
		};
		// Return the first created windows renderer
		//inline Renderer* getRenderer() {
		//	if (m_windows.size() != 0) return m_windows[0]->getRenderer();
		//	return nullptr;
		//}

		EngineWorld* createWorld();
		std::vector<EngineWorld*>& getWorlds();

		bool isRenderingWindow() const { return m_renderingWindows; }

		bool isStopped() const { return m_stopped; }

		Profiler& getProfiler() { return m_profiler; }

#ifdef ENGONE_PHYSICS
		rp3d::PhysicsCommon* getPhysicsCommon();
		//void lockCommon() { m_commonMutex.lock(); }
		//void unlockCommon() { m_commonMutex.unlock(); }
#endif

		ExecutionControl& getControl() { return updateControl; }
		ExecutionTimer& getExecTimer() { return executionTimer; }
		void setMultiThreaded(bool yes) { m_isThreaded = yes; }
		bool isMultiThreaded() { return m_isThreaded; }

		void setUPS(double ups);
		double getUPS();
		double getRealUPS();
		
		void setFPS(double fps, uint32 windowIndex=0);
		double getFPS(uint32 windowIndex=0);
		double getRealFPS(uint32 windowIndex=0);

		double getRuntime();

		// static TrackerId trackerId;
	private:

		bool m_isThreaded=false;

		bool m_stopped = false;
		std::vector<Window*> m_windows;
		bool m_renderingWindows=false;
		Engone* m_engone=nullptr;

		std::vector<EngineWorld*> m_worlds;

		ExecutionControl updateControl{};
		Thread updateThread{};
		ExecutionTimer executionTimer{};

		Profiler m_profiler{};

#ifdef ENGONE_PHYSICS
		//Mutex m_commonMutex;
		// DepthMutex m_commonMutex;
		//rp3d::PhysicsCommon* m_physicsCommon = nullptr;
#endif
		friend class Engone;
		friend class Window;

	};
}
#endif