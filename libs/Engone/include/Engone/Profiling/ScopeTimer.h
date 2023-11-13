#pragma once

// This is a macro because you can set it to nothing. Nice for release build?
#define TIME_SECTION(NAME,profiler) ScopeTimer NAME(#NAME,profiler);

#include "Engone/Utilities/TimeUtility.h"
#include "Engone/Profiling/Profiler.h"

namespace engone {


	class ScopeTimer {
	public:
		ScopeTimer(const std::string& name, Profiler* profiler);
		~ScopeTimer();

		Profiler* profiler = nullptr;
		std::string name;
		double startSeconds=0;
	};
}