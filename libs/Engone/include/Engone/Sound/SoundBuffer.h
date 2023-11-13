#pragma once

#ifdef ENGONE_SOUND

// only use alCall with an active context
#define alCall(x) x; if(al_check_error()) __debugbreak();
bool al_check_error();

namespace engone {
	class SoundBuffer {
	public:
		SoundBuffer() = default;
		~SoundBuffer();
		void Init(const char* path);

		inline bool valid() { return id; }

	private:
		unsigned int id=0;

		friend class SoundSource;
	};
}

#endif // ENGONE_SOUND