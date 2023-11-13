#pragma once

#include "Engone/Utilities/Typedefs.h"

namespace engone {
	namespace log {
		enum Color : uint8 {
			BLACK = 0x00,
			NAVY = 0x01,
			GREEN = 0x02,
			CYAN = 0x03,
			BLOOD = 0x04,
			PURPLE = 0x05,
			GOLD = 0x06,
			SILVER = 0x07,
			GRAY = 0x08,
			BLUE = 0x09,
			LIME = 0x0A,
			AQUA = 0x0B,
			RED = 0x0C,
			MAGENTA = 0x0D,
			YELLOW = 0x0E,
			WHITE = 0x0F,
			_BLACK = 0x00,
			_BLUE = 0x10,
			_GREEN = 0x20,
			_TEAL = 0x30,
			_BLOOD = 0x40,
			_PURPLE = 0x50,
			_GOLD = 0x60,
			_SILVER = 0x70,
			_GRAY = 0x80,
			_NAVY = 0x90,
			_LIME = 0xA0,
			_AQUA = 0xB0,
			_RED = 0xC0,
			_MAGENTA = 0xD0,
			_YELLOW = 0xE0,
			_WHITE = 0xF0,
			NO_COLOR = 0x00,
		};
		enum Filter : uint8 {
			INFO = 1,
			WARN = 2,
			ERR = 4,
			FilterAll = INFO | WARN | ERR,
		};
	}
}