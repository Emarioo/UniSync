#pragma once

// #include "Engone/Utilities/Tracker.h"

namespace engone {
	
	class Window;

	enum EventType : int {
		EventNone = 0,
		EventClick = 1,
		EventMove = 2,
		EventScroll = 4,
		EventKey = 8,
		EventResize = 16,
		EventClose = 32,
	};
	typedef int EventTypes;
	struct Keybinding {
		int keys[3]{0,0,0};
	};
	struct EventInput {
		int code=0;
		bool down = false;
		uint8_t tickPressed = 0;
		uint8_t framePressed = 0;
		uint8_t tickReleased = 0;
		uint8_t frameReleased = 0;
	};
	struct Event {
		EventType eventType=EventNone;
		int action=0, button=0;
		int key=0, scancode=0;
		float mx=0, my=0, scrollX=0, scrollY=0;
		float width=0, height=0; // resize
		Window* window=nullptr;
	};
	// Can listen to multiple events. Attach it to a window with Window::attachListener
	class Listener {
	public:
		Listener(EventTypes eventTypes, std::function<EventTypes(Event&)> f);
		// @eventtypes events which will be listened to
		// @priority a higher number will call this listener before others
		// @f returning an eventtype will stop listeners with lower @priority from listening to that/those events
		Listener(EventTypes eventTypes, int priority, std::function<EventTypes(Event&)> f);
		std::function<EventTypes(Event&)> run;
		int priority = 0;
		EventTypes eventTypes=EventNone;

		// static TrackerId trackerId;
	private:
		bool m_ownedByWindow = false; // when true, window is responsible for deleting listener.

		friend class Window;
	};
	std::string PollClipboard();
	void SetClipboard(const char* str);
	std::string PollPathDrop();
	uint32_t PollChar();
	// -1 if window is out of focus
	float GetMouseX();
	float GetMouseY();
	// return 0 if no scroll and 1 or -1 if scrolled.
	float IsScrolledY();
	// return 0 if no scroll and 1 or -1 if scrolled.
	float IsScrolledX();
	// virtualKeyCode: glfw virtual key code depending on what you are using.
	// returns true if the key or mouse button is down.
	bool IsKeyDown(int code);	
	// virtualKeyCode: Windows.h or glfw virtual key code depending on what you are using.
	// returns true if the key was pressed this refresh/frame/update.
	bool IsKeyPressed(int code);
	bool IsKeyReleased(int code);
	void ResetKey(int code);

	bool IsKeybindingDown(uint16_t id);
	bool IsKeybindingPressed(uint16_t id);
	// if keybinding uses multiple keys then the first one needs to be released and the others down
	// on order for the function to return true.
	bool IsKeybindingReleased(uint16_t id);
	// When using IsKeybindingDown: All of the specified keycodes needs to be down.
	// When using IsKeybindingPressed: The last two keycodes needs to be down before pressing the first one.
	// The last two keycodes are optional
	void AddKeybinding(uint16_t id, int keyCode0, int keyCode1 = -1, int keyCode2 = -1);
	// Returns number of keys loaded. You should create default keybindings if zero is returned.
	// Less keybindings than there should be will most likely cause issues in gameplay.
	// @path is the path of the file. data/keybindings.dat for example. The .dat is optional.
	int LoadKeybindings(const std::string& path);
	// File could not be saved if false is returned. Directory might not exist.
	bool SaveKeybindings(const std::string& path);
	void ClearKeybindings();
}