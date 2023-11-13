#pragma once

namespace engone {
	class Window;
	class Engone;
	class AppInstance;
	// timeStep, app, window, interpolation
	// struct LoopInfo {
	// 	double timeStep=0;
	// 	Application* app=nullptr;
	// 	Window* window=nullptr;
	// 	double interpolation=0;
	// };
	struct LoopInfo {
		double timeStep=0;
		Engone* engone=0;
		AppInstance* instance=0;
		Window* window=nullptr;
		double interpolation=0;
	};
}