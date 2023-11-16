#include "Engone/Error.h"

namespace engone {
	const char* ToString(Error value) {
		#define GEN_STR(X) case X: return #X;
		switch (value) {
			GEN_STR(NoError);
			GEN_STR(FileNotFound);
			GEN_STR(EndOfFile);
			GEN_STR(AllocFailure);
			GEN_STR(OtherError);
		// case ErrorNone: return "None";
		// case ErrorMissingFile: return "MissingFile";
		// case ErrorCorruptedFile: return "CorruptedFile";
		}
		return "UnknownError";
	}
	// #ifdef ENGONE_LOGGER
	Logger& operator<<(Logger& log, Error value) {
		log << ToString(value);
		return log;
	}
	// #endif
}