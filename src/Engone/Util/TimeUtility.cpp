#include "Engone/Util/TimeUtility.h"

namespace engone {
	double GetSystemTime() {
		return (double)std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::system_clock::now().time_since_epoch()).count() / 1000000.0;
		//std::cout << std::chrono::system_clock::now().time_since_epoch().count() <<" "<< (std::chrono::system_clock::now().time_since_epoch().count() / 10000000) << "\n";
		//return (double)(std::chrono::system_clock::now().time_since_epoch().count() / 10000000);
	}
	std::string GetClock() {
		time_t t;
		time(&t);
		tm tm;
		localtime_s(&tm, &t);
		std::string str = std::to_string(tm.tm_hour) + ":" + std::to_string(tm.tm_min) + ":" + std::to_string(tm.tm_sec);
		return str;
	}
	void GetClock(char* str) {
		time_t t;
		time(&t);
		tm tm;
		localtime_s(&tm, &t);

		str[2] = ':';
		str[5] = ':';

		int temp = tm.tm_hour / 10;
		str[0] = '0' + temp;
		temp = tm.tm_hour - 10 * temp;
		str[1] = '0' + temp;

		temp = tm.tm_min / 10;
		str[3] = '0' + temp;
		temp = tm.tm_min - 10 * temp;
		str[4] = '0' + temp;

		temp = tm.tm_sec / 10;
		str[6] = '0' + temp;
		temp = tm.tm_sec - 10 * temp;
		str[7] = '0' + temp;
	}
	std::string FormatTime(double seconds, bool compact, FormatTimePeriods flags) {
		bool small = false;
		for (int i = 0; i < 3; i++) {
			if (flags & (1 << i))
				small = true;
			if (small) seconds *= 1000;
		}
		return FormatTime((u64)round(seconds), compact, flags);
	}
	std::string FormatTime(u64 time, bool compact, FormatTimePeriods flags) {
		if (time == 0)
			return "0s";
		// what is the max amount of characters?
		const int outSize = 130;
		char out[outSize]{};
		int write = 0;

		u64 num[8]{};
		u64 divs[8]{ 1,1,1,1,1,1,1,1 };
		u64 divLeaps[8]{ 1000,1000,1000,60,60,24,7,1 };
		const char* lit[8]{ "nanosecond","microsecond","millisecond","second","minute","hour","day","week" };
		const char* lit_compact[8]{ "ns","us","ms","s","m","h","d","w" };

		u64 rest = time;

		bool smallest = false;
		for (int i = 0; i < 7; i++) {
			if (flags & (1 << i))
				smallest = true;
			if (smallest)
				divs[i + 1] *= divs[i] * divLeaps[i];
		}

		for (int i = 7; i >= 0; i--) {
			if (0 == (flags & (1 << i))) continue;
			num[i] = rest / divs[i];
			if (num[i] == 0) continue;
			rest -= num[i] * divs[i];

			if (write != 0) out[write++] = ' ';
			if(compact)
				write += snprintf(out + write, outSize - write, "%llu", num[i]);
			else
				write += snprintf(out + write, outSize - write, "%llu ", num[i]); // adds a space
			if (compact) {
                memcpy(out + write, lit_compact[i], strlen(lit_compact[i]));
				write += strlen(lit_compact[i]);
			}
			else {
				memcpy(out + write, lit[i], strlen(lit[i]));
				write += strlen(lit[i]);
				if (num[i] != 1)
					out[write++] = 's';
			}
		}
		return out;
	}
	// void Sleep(double seconds) {
	// 	std::this_thread::sleep_for((std::chrono::microseconds)((u64)(seconds * 1000000)));
	// }
}