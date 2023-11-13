#pragma once

#include "Engone/Window.h"

namespace engone {
	// struct with glm::vec3 pos, glm::vec3 vel.
	struct DefaultParticle {
		glm::vec3 pos;
		glm::vec3 vel;
	};
	
	// A class for rendering particles. Requires a shader specific for rendering particles with shader buffer.
	// "delta" uniform in shader is set before rendering particles.
	// non-templated particle group.
	class ParticleGroupT {
	public:
		ParticleGroupT() = default;
		~ParticleGroupT() { cleanup(); }

		enum Flag : uint8_t {
			AlphaBlend = 1,
			//InfoAliveCount=1, // adds aliveCount to struct if it exists, make sure it is big enough.
		};
		typedef uint8_t Flags;

		// struct size is sizeof your particle
		void initT(Window* window, Shader* shader, int pSize, int iSize, Flags flags=0);

		//void update(engone::LoopInfo& info);
		// Will update projection and bind shader
		void render(engone::LoopInfo& info);

		struct ret { void* info; void* parts; };
		// will allocate particles for you to change.
		ret createParticlesT(int count, bool setZero = true);
		// Will assume glm::vec3 pos, glm::vec3 vel exists AND are the first ones in your particle struct.
		bool createCube(glm::vec3 position, glm::vec3 scale, uint32_t particleCount, glm::vec3 velocity = { 0,0,0 });

		// Always call this function when changing particle data. (unless you used createParticles).
		// The shader changes the data on the gpu side which means 
		// that it needs to be synchronized with the internal array.
		// requires correct context.
		void* getParticlesT();
		// could optimize getParticles, getInfo so that when you call both, only one call to shaderBuffer.getData will be made
		void* getInfoT();
		uint32_t getCount() const { return m_count; };
		uint32_t getCapacity() const { return m_capacity; }

		Shader* getShader() const { return m_shader; }

		void setPointSize(float size) { m_pointSize = size; };

		void clear();
		void cleanup();

		// will change the size of the internal particle array.
		// only do this when rendering because the shaderBuffer will set data and requires the correct active window
		bool resize(int size);
	private:
		Window* m_parent = nullptr;

		float m_pointSize = 1;// point render size

		Flags m_flags;

		bool m_refreshData = false;
		bool m_refreshShaderData = false;
		bool m_refreshShaderInfo = false;
		Shader* m_shader = nullptr;

		void* m_data = nullptr;
		int m_capacity = 0;
		int m_count = 0;

		int m_iSize = 0; // size of particle struct
		int m_pSize = 0; // size of particle struct

		ShaderBuffer m_shaderBuffer{};
	};
	template<typename P>
	class ParticleGroup : public ParticleGroupT {
	public:
		ParticleGroup() = default;
		~ParticleGroup() { cleanup(); }

		void init(Window* window, Shader* shader, Flags flags=0) {
			initT(window, shader, sizeof(P),0,flags);
		}

		// will allocate particles for you to change.
		P* createParticles(int count, bool setZero = true) {
			ret ptrs = createParticlesT(count, setZero);
			return (P*)ptrs.parts;
		}

		// Always call this function when changing particle data. (unless you used createParticles).
		// The shader changes the data on the gpu side which means 
		// that it needs to be synchronized with the internal array.
		// requires correct context.
		P* getParticles() {
			return (P*)getParticlesT();
		}
	private:

	};
	// some header data in shader buffer. Maybe you need an alive count which you can change within the shader.
	//template<typename P, typename I>
	//class ParticleGroupExtra : public ParticleGroupT {
	//public:
	//	ParticleGroupExtra() = default;
	//	~ParticleGroupExtra() { cleanup(); }

	//	void init(Window* window, Shader* shader, Flags flags=0) {
	//		initT(window, shader, sizeof(P), sizeof(I),flags);
	//	}

	//	// will allocate particles for you to change.
	//	struct { I* info; P* parts; } createParticles(int count, bool setZero = true) {
	//		ret ptrs= createParticlesT(count, setZero);
	//		return {(I*)ptrs.info,(P*)ptrs.parts};
	//	}

	//	// Always call this function when changing particle data. (unless you used createParticles).
	//	// The shader changes the data on the gpu side which means 
	//	// that it needs to be synchronized with the internal array.
	//	// requires correct context.
	//	P* getParticles() {
	//		return (P*)getParticlesT();
	//	}
	//	I* getInfo() {
	//		return (I*)getInfoT();
	//	}
	//private:

	//};
}