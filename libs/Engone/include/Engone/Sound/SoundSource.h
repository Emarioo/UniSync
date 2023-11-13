#pragma once

#ifdef ENGONE_SOUND

#include "Engone/Sound/SoundBuffer.h"

namespace engone {
	class SoundSource {
	public:
		SoundSource()=default;
		~SoundSource();
		void Init();
		
		void Loop(bool);
		void Gain(float);
		void Pitch(float);
		void Position(float x,float y,float z);
		void Velocity(float x,float y,float z);

		void Bind(SoundBuffer& buffer);
		void Play();
		void Pause();
		void Stop();
		int GetState();

		bool valid() { return id; }

	private:
		unsigned int id=0;

		friend class SoundStream;
	};
}

#endif // ENGONE_SOUND