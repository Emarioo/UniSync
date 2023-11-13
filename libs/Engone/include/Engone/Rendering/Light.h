#pragma once

namespace engone {
	enum class LightType {
		None,
		Direction,
		Spot,
		Point
	};
	class Shader;
	class Light {
	public:
		Light(LightType type) : lightType(type), position({ 0,0,0 }), direction({ -1,-1,-1 }),
			ambient({ 0.2,0.2,0.2 }), diffuse({ 1,1,1 }), specular({ 1,1,1 }) {}
		Light(LightType type, glm::vec3 position,glm::vec3 direction) : lightType(type), position(position), direction(direction),
			ambient({ 0.2,0.2,0.2 }), diffuse({ 1,1,1 }), specular({ 1,1,1 }) {}
		Light(LightType type, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
			: lightType(type), position({ 0,0,0 }), direction({-1, -1, -1}), ambient(ambient), diffuse(diffuse), specular(specular) {}
		Light(LightType type, glm::vec3 position, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse,glm::vec3 specular) : 
			lightType(type), position(position), direction(direction), ambient(ambient), diffuse(diffuse), specular(specular) {}
		
		LightType lightType;
		glm::vec3 position;
		glm::vec3 direction;
		glm::vec3 ambient;
		glm::vec3 diffuse;
		glm::vec3 specular;

		virtual void bind(Shader* shader, int index);
	};
	class DirLight : public Light {
	public:
		DirLight() : Light(LightType::Direction) {}
		DirLight(glm::vec3 direction) : Light(LightType::Direction, {0,0,0},direction) {}
		DirLight(glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
			: Light(LightType::Direction,ambient,diffuse,specular) {}

		void bind(Shader* shader, int index=0) override;
	};
	class PointLight : public Light {
	public:
		PointLight() : Light(LightType::Point) {}
		PointLight(glm::vec3 position) : Light(LightType::Point, position, {-1,-1,-1}) {}
		PointLight(glm::vec3 position, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
			: Light(LightType::Point, position, { -1,-1,-1 }, ambient, diffuse, specular) {}

		float constant = 1;
		float linear = 0.09f;
		float quadratic = 0.032f;
		
		void bind(Shader* shader, int index) override;
	};
	class SpotLight : public Light {
	public:
		SpotLight() : Light(LightType::Spot) {}
		SpotLight(glm::vec3 position, glm::vec3 direction) : Light(LightType::Spot,position,direction) {}
		SpotLight(glm::vec3 position, glm::vec3 direction, glm::vec3 ambient, glm::vec3 diffuse, glm::vec3 specular)
			: Light(LightType::Spot,position,direction,ambient, diffuse, specular) {}

		float cutOff = glm::cos(glm::radians(30.f));
		float outerCutOff = glm::cos(glm::radians(35.f));
		
		void bind(Shader* shader, int index) override;
	};
}