#pragma once

#include "Engone/Assets/Asset.h"

namespace engone {
	enum PolationType : unsigned char {
		Constant, Linear, Bezier
	};
	class Keyframe {
	public:
		PolationType polation;
		unsigned short frame;
		float value;
		/*
		"polation" is the type of polation between keyframes.
		*/
		Keyframe(PolationType polation, unsigned short frame, float value);
	};
	class FCurve {
	public:
		FCurve();
		Keyframe& get(int index);
		void add(Keyframe keyframe);

		std::vector<Keyframe> frames;
	};
	enum ChannelType : unsigned char {
		PosX = 0, PosY, PosZ,
		RotX, RotY, RotZ, // Not used. Deprecated, could be brought back.
		ScaX, ScaY, ScaZ,
		QuaX, QuaY, QuaZ, QuaW,
	};
	class Channels {
	public:
		Channels() {}
		FCurve& get(ChannelType channel);
		/*
		"channel" is what type of value the keyframes will be changing. Example PosX.
		Create a "FCurve" object and see the "Add" function for more details.
		*/
		void add(ChannelType channel, FCurve fcurve);
		/*
		Add values to the references given in the argument
		*/
		void getValues(int frame, float blend, glm::vec3& pos, glm::vec3& euler, glm::vec3& scale, glm::mat4& quater, short* usedChannel);
		std::unordered_map<ChannelType, FCurve> fcurves;
	};
	class AnimationAsset : public Asset {
	public:
		static const AssetType TYPE = AssetAnimation;
		AnimationAsset() : Asset(TYPE) { };
		~AnimationAsset() override { cleanup(); }

		void cleanup() override;
		LoadFlags load(LoadFlags flags) override;

		Channels* get(unsigned short i);
		/*
		objectIndex is the index of the bone. Also known as the vertex group in blender.
		Create a "Channels" object and see the "Add" function for more details.
		*/
		//void addObjectChannels(int objectIndex, Channels channels);
		/*
		Use this to create an animation by code.
		See the "AddObjectChannels" function for more details.
		*/
		//void modify(unsigned short startFrame, unsigned short endFrame);
		/*
		Use this to create an animation by code.
		"speed" is 24 by default. Speed can also be changed in the animator.
		See the "AddObjectChannels" function for more details.
		*/

		//void modify(unsigned short startFrame, unsigned short endFrame, float speed);

		std::unordered_map<unsigned short, Channels> objects;

		uint16_t frameStart = 0;
		uint16_t frameEnd = 0;
		float defaultSpeed = 24;// frames per second. 24 is default from blender.

		// static TrackerId trackerId;
	private:
	};
}