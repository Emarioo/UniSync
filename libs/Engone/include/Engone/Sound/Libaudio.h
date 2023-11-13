#pragma once

#include "Engone/Utilities/Typedefs.h"

// http://soundfile.sapp.org/doc/WaveFormat/
// https://docs.fileformat.com/audio/wav/
// integers are little endian
struct WAVEHeader {
    static const u32 SIZE = 44; // sizeof(WAVEHeader) == 44
    
    char riff[4] = {'R','I','F','F'};
    u32 almostFileSize = 0; // fileSize - 8 (fileSize - sizeof riff - sizeof almostFileSize)
    char wave[4] = {'W','A','V','E'};
    
    char fmt[4] = {'f','m','t',' '};
    u32 lengthOfFormatData = 16; // bytes from audioFormat (inclusive) to bitsPerSample (inclusive)
    u16 audioFormat = 1; // 1 for PCM
    u16 channels = 0; // Usually 1 for mono, 2 for stereo
    u32 sampleRate = 0; // 44100 Hz
    u32 byteRate = 0; // (sampleRate * bitsPerSample * channels) / 8
    u16 blockAlign = 4; // (bitsPerSample * channels) / 8.  1 = 8 bit mono, 2 = 8 bit stereo OR 16 bit mono, 4 = 16 bit stereo
    u16 bitsPerSample = 0;
    
    char data[4] = {'d','a','t','a'};
    u32 sizeOfData = 0;
};
i8* ReadWAVE(const std::string& filename, int& channels, int& sampleRate, int& bps, int& size);
bool WriteWAVE(const std::string& filename, int& channels, int& sampleRate, int& bps, int& size, i8* data);