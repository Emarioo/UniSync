#pragma once

#include "Engone/RenderModule.h"
#include "Engone/EventModule.h"


#include "Engone/Core/ExecutionControl.h"
// #include "Engone/Utilities/Thread.h"
#include "Engone/PlatformModule/PlatformLayer.h"

#include "Engone/Rendering/UIRenderer.h"
#include "Engone/Rendering/CommonRenderer.h"
#include "Engone/UIModule/UIModule.h"

namespace engone {

	// note that BorderlessFullscreen is a bitmask of borderless and fullscreen
	enum WindowMode : uint8_t {
		ModeWindowed = 0,
		ModeBorderless = 1,
		ModeFullscreen = 2,
		ModeBorderlessFullscreen = 3,
		ModeTransparent = 4,
		ModeMousePassthrough = 8,
	};
	typedef uint8_t WindowModes;
	class Engone;
	// mode, w, h, x, y
	struct WindowDetail {
		WindowModes mode = ModeWindowed;
		int w = -1, h = -1;
		int x = -1, y = -1;
	};
	typedef EventType(*ListenProc)(Event*);
	// Window can be seen as a reference to a window on your computer.
	class Window {
	public:
			
		inline GLFWwindow* glfw() const { return m_glfwWindow; }

		// can be called before window has been created. Altough window is required to use this function.
		void setTitle(const std::string title);
		// 32 bit RGBA. 16x16, 32x32, 48x48 are good sizes.
		void setIcon(RawImage* img);
		// will make this window/context the active one.
		// will not set window as active if it already is.
		void setActiveContext();

		void setMode(WindowModes mode, bool force=false);
		// May send a EventMove to listeners since the mouse position is relative to window position.
		void setPosition(float x,float y);
		void setSize(float w, float h);

		float getX() const {return x; }
		float getY() const {return y; }
		// Inner width of window
		float getWidth() const { return w; }
		// Inner height of window
		float getHeight() const { return h; }

		WindowModes getMode() const { return m_windowMode; }

		bool hasFocus() const { return m_focus; }
		// True if cursor is visible.
		bool isCursorVisible() const { return m_cursorVisible; }

		Engone* getParent() { return m_parent; }
		// Application* getParent() { return m_parent; }
		AssetStorage* getStorage() { return &m_storage; }
		
		CommonRenderer* getCommonRenderer(){return &m_commonRenderer;}
		UIRenderer* getUIRenderer(){return &m_uiRenderer;}

		// Window will not destroy listener. Should it?
		void attachListener(Listener* listener);
		
		// procName is only just when reloading DLLs
		void addListener(EventTypes type, ListenProc proc, const std::string& procName);

		void addEvent(Event& e) { m_events.push_back(e); }
		void runListeners();

		void maximize(bool yes);

		float getMouseX() const { return m_mouseX; }
		float getMouseY() const { return m_mouseY; }
		// raw input from first person
		float getRawMouseX() const;
		float getRawMouseY() const;
		void setMouseX(float x) { m_mouseX=x; }
		void setMouseY(float y) { m_mouseY=y; }
		float getScrollX() const;
		float getScrollY() const;
		void setScrollX(float x) { m_frameScrollX += x; m_tickScrollX += x;}
		void setScrollY(float y) { m_frameScrollY += y; m_tickScrollY += y; }

		void setInput(int code, bool down);

		std::string pollPathDrop();
		uint32_t pollChar();

		bool isKeyDown(int code);
		bool isKeyPressed(int code);
		bool isKeyReleased(int code);
		
		// bool isKeybindingDown(uint16_t id);
		// bool isKeybindingPressed(uint16_t id);
		// // if keybinding uses multiple keys then the first one needs to be released and the others down
		// // on order for the function to return true.
		// bool isKeybindingReleased(uint16_t id);

		// void resetKey(int code);
		void resetEvents(bool resetFrameInput);

		void enableFirstPerson(bool yes);
		bool isFirstPerson() const { return m_enabledFirstPerson; }

		// Manually calls the CloseCallback function
		void close();
		// If true, the cursor will be made visible.
		void setCursorVisible(bool visible);
		// Whether the cursor is locked to the window.
		bool isCursorLocked() const { return m_cursorLocked;}
		// If true, the cursor will be made invisible and locked to the window. Use this when you want the player to lock around.
		void lockCursor(bool locked);
		// true if window is open, or should be open
		bool isOpen();

		ExecutionControl& getControl();
		ExecutionTimer& getExecTimer();

		void setFPS(double fps);
		double getFPS();
		double getRealFPS();
		
		void* userData=0;
		
		UIModule uiModule;

		// static TrackerId trackerId;
	private:
		void init(Engone* engone, WindowDetail detail);
		// Window(Engone* engone, WindowDetail detail);
		Window() = default;
		// Window(Application* application, WindowDetail detail);
		// Deleting window will delete renderer's buffers. To do this this window becomes the active one. This means that you sohuldn't call delete on the window.
		// The game loop will delete a window that should close because it nows when the window is being rendered to and not. Deleting a window while it is being rendered to is bad.
		// if this window shares buffers with another unexepected things might happen.
		~Window();
		
		// GLFW and GLEW should not be used if the game code is loaded from a dll.
		// GLFW and GLEW has global variables which will differ between the game code dll and the main code.
		// Dll is used with LIVE_EDITING. You could We don't use GLFW and GLEW as dlls because it makes things easier.
		bool restrictDLL = false;
		
		enum PendingAction : u32 {
			PENDING_INIT,
			PENDING_TITLE,
			PENDING_LOCK_CURSOR,
			PENDING_VISIBLE_CURSOR,
			PENDING_CLOSE,
		};
		std::vector<PendingAction> pendingActions;
		WindowDetail initialDetail{};
		void performPendingActions();

		Listener* m_firstPersonListener=nullptr;

		GLFWwindow* m_glfwWindow=nullptr;
		Engone* m_parent=nullptr;
		//Assets m_assets;
		AssetStorage m_storage;
		//Renderer m_renderer;
		// RenderPipeline m_renderPipeline;
		std::vector<Listener*> m_listeners;
		std::vector<Event> m_events;
		
		struct Input {
			int code=0;
			bool down = false;
			uint8_t tickPressed = 0;
			uint8_t framePressed = 0;
			uint8_t tickReleased = 0;
			uint8_t frameReleased = 0;
		};
		
		std::vector<Input> m_inputs;
		std::vector<std::string> m_pathDrops;
		
		struct ListenerProc {
			EventTypes types;
			ListenProc proc;
			std::string procName;
		};
		std::vector<ListenerProc> listenerProcs;
		
		CommonRenderer m_commonRenderer;
		UIRenderer m_uiRenderer;

		bool initializedGLEW = false;

		ExecutionTimer executionTimer;
		ExecutionControl renderControl;
		Thread renderThread;

		uint32_t m_charIn = 0;
		uint32_t m_charOut = 0;
		bool m_emptyCharArray = true;
		static const uint32_t CHAR_ARRAY_SIZE = 20;
		uint32_t m_charArray[CHAR_ARRAY_SIZE];

		bool m_enabledFirstPerson = false;

		float m_mouseX = 0, m_mouseY = 0;
		float m_lastMouseX = -1, m_lastMouseY=-1;
		float m_tickRawMouseX = 0, m_tickRawMouseY = 0, m_frameRawMouseX = 0, m_frameRawMouseY = 0;
		float m_tickScrollX=0, m_tickScrollY = 0;
		float m_frameScrollX = 0, m_frameScrollY = 0;

		WindowModes m_windowMode = ModeWindowed; // can't be -1 because of bit masking
		bool m_cursorVisible = true, m_cursorLocked = false, m_focus = true;
		std::string m_title = "Untitled";
		float x = -1, y = -1, w = -1, h = -1; // direct coordinates of the window (updated through callbacks)
		float savedCoords[4]{ -1,-1,-1,-1 };// store coords from before fullscreen.
		void loadCoords() {
			memcpy_s(&x, sizeof(float) * 4, savedCoords, sizeof(float) * 4);
		}
		void saveCoords() {
			memcpy_s(savedCoords, sizeof(float) * 4, &x, sizeof(float) * 4);
		}

		friend class Application;
		friend class Engone;

		friend EventType FirstPerson(Event& e);
		friend EventType FirstPersonProc(Event* _e);
		friend void FocusCallback(GLFWwindow* window, int focused);
		friend void CloseCallback(GLFWwindow* window);
		friend void ResizeCallback(GLFWwindow* window, int width, int height);
		friend void PosCallback(GLFWwindow* window, int x, int y);
		friend void CharCallback(GLFWwindow* window, uint32_t chr);
		friend void DropCallback(GLFWwindow* window, int count, const char** paths);

	};

	void InitializeGLFW();
	// Window* GetMappedWindow(GLFWwindow* window);

	// May be nullptr
	Window* GetActiveWindow();
	void MakeWindowActive(Window* window);
	// // Inner width of active window
	// float GetWidth();
	// // Inner height of active window
	// float GetHeight();
}