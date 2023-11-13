#pragma once

#include "Engone/Sound/SoundStream.h"

namespace engone {
	// returns false if error
	bool InitSound();
	void UninitSound();

	void ListenerPosition(float x, float y, float z);
	void ListenerVelocity(float x, float y, float z);
	/*
	x,y,z front direction
	x1,y1,z1 up direction
	*/
	void ListenerRotation(float x, float y, float z, float x1, float y1, float z1);

	std::string ListDefaultDevice();
	std::vector<std::string> ListDevices();
}
//#endif
/* Example

	melody.Init("assets/sounds/melody.wav");
	engine::SoundBuffer trumpet;
	trumpet.Init("assets/sounds/trumpet.wav");
	engine::SoundSource trumpetS;
	trumpetS.Init();
	trumpetS.Bind(trumpet);
	//trumpetS.Play();
	//melody.source.Play();
	melody.source.Gain(0.25);

	melody.UpdateStream();
*/