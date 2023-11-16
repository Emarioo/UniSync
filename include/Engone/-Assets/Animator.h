#pragma once

namespace engone {
	class AnimationAsset;
	class ModelAsset;
	struct AnimatorProperty {
		bool loop;
		float speed;// multiplier to the default speed
		float blend;
		float frame;
		std::string instanceName;
		AnimationAsset* asset;

		float getRemainingSeconds() const;
	};
	class Animator {
	public:
		Animator() = default;
		void cleanup();

		ModelAsset* asset; // animations are stored in here
		// may not need to store this here, because if animator exists,
		// there should be a ModelRenderer

		static const uint32_t maxAnimations = 4;
		AnimatorProperty enabledAnimations[maxAnimations];

		void update(float delta);

		// should only be used to set blend, speed, loop or frame. don't mess with the name or asset.
		AnimatorProperty* getProp(const std::string& instName);
		//void setBlend(const std::string& name, float blend);
		//void setSpeed(const std::string& name, float speed);
		/*
		Anim. Prop. {loop,blend,speed,frame} you should maybe double check it though.
		*/
		void enable(const std::string& instanceName, const std::string& animationName, AnimatorProperty prop);
		void disable(const std::string& instName);

		bool isEnabled(const std::string& instName) const;
	};
}