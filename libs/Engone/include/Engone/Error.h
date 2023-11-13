#pragma once

#ifdef ENGONE_LOGGER
#include "Logger.h"
#endif

namespace engone {
	enum Error : uint32 {
		// ErrorNone = 0,
		// ErrorMissingFile = 1, // could also mean that the file is used by another process (application/program)
		// ErrorCorruptedFile = 2,
		NoError,
		FileNotFound,
		EndOfFile,
		AllocFailure,
		DataWasLost,
		OtherError,

	};
	const char* ToString(Error value);
	#ifdef ENGONE_LOGGER
	Logger& operator<<(Logger& log, Error value);
	#endif
}