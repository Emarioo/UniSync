#pragma once

#ifdef ENGONE_SOUND

#include "Engone/Sound/SoundSource.h"

namespace engone {
	class SoundStream {
	public:
		SoundStream()=default;
		~SoundStream();
		void Init(const std::string& path);
		void UpdateStream();

		SoundSource source;
	private:
		const std::uint32_t NUM_BUFFERS = 4;
		const std::uint32_t BUFFER_SIZE = 65536;// 32kb
		char* bufferData = nullptr;
		int bufferSize=0;
		int bufferFreq=0;
		int32_t bufferFormat=0;
		std::uint32_t cursor=0;
		uint32_t buffer_id[4];
		bool isInitialized = false;
	};
}

#endif // ENGONE_SOUND