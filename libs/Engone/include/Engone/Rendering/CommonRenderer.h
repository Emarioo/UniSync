#pragma once

#include "Engone/AssetModule.h"
#include "Engone/Rendering/Camera.h"
#include "Engone/LoopInfo.h"

#include "Engone/Utilities/Utilities.h"

#include "Engone/Rendering/RenderPipeline.h"

// #define GET_COMMON_RENDERER() (CommonRenderer*)engone::GetActiveWindow()->getPipeline()->getComponent("common_renderer");

namespace engone {

	class Window;
	// Render pipeline with a few useful functions
	class CommonRenderer 
	// : public RenderComponent 
	{
	public:
		CommonRenderer() = default; // : RenderComponent("common_renderer",1000) {}
		~CommonRenderer();

		// buffers will be attached to the active window
		void init(Window* win);

		Camera* getCamera();
		glm::mat4& getLightProj();

		glm::mat4& getPerspective();
		glm::mat4& getOrthogonal();

		// Set when updating perspective no need to call this.
		void setPerspective();
		// Set when updating orthogonal no need to call this.
		void setOrthogonal();

		// Uses the camera's viewMatrix
		void updatePerspective(Shader* shader);
		void updatePerspective(Shader* shader, glm::mat4 viewMatrix);
		// Uses the camera's viewMatrix
		void updateOrthogonal(Shader* shader);
		void updateOrthogonal(Shader* shader, glm::mat4 viewMatrix);

		/*
		set uniforms before. uPos, USize, uColor, uTextured. Make sure Blending is on, make sure uWindows is set.
		Slower but more sophisticated. Function can center and wrap text in a box.
		*/
		void drawString(FontAsset* font, const std::string& text, bool center, float wantedHeight, float maxWidth, float maxHeight, int atChar);
		/*
		set uniforms before. uPos, USize, uColor, uTextured. Make sure Blending is on, make sure uWindows is set.
		Quick version of drawing text
		*/
		void drawString(FontAsset* font, const std::string& text, float height, int cursorPosition);

		void drawCube(glm::mat4 matrix, glm::vec3 scale = { 1,1,1 }, glm::vec3 color = { 1,1,1 });
		void drawNetCube(glm::mat4 matrix, glm::vec3 scale = { 1,1,1 }, glm::vec3 color = { 1,1,1 });
		void drawSphere(glm::vec3 position, float radius = 1, glm::vec3 color = { 1,1,1 });

		void drawLine(glm::vec3 a, glm::vec3 b, glm::vec3 rgb = { 1.f,0.f,0.f });
		void drawCubeRaw();

		//void DrawLine(glm::vec3 a, glm::vec3 b);
		void drawTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c);

		// will draw a vertex buffer with two triangles.
		// Set shader, texture, uniforms before.
		void drawQuad(LoopInfo& info);

		//// Uses standard viewMatrix from camera
		//void DrawModel(ModelAsset* modelAsset, glm::mat4 matrix);
		//
		//// Draws model orthogonally
		//void DrawOrthoModel(ModelAsset* modelAsset, glm::mat4 matrix);

		void render(LoopInfo& info);


		// ui::Draw amongst others will use the active renderer.
		void setActiveRenderer();

		VertexBuffer& getInstanceBuffer() { return instanceBuffer; }

		static const int TEXT_BATCH = 40;
		//static const int MAX_BOX_BATCH = 100;
		static const int MAX_CUBE_BATCH = 100;
		//static const int VERTEX_SIZE = 2 + 2 + 4 + 1;
		static const uint32_t INSTANCE_BATCH = 1000;

	private:
		bool m_initialized = false;

		glm::mat4 lightProjection{};

		// Projection matrices
		glm::mat4 perspectiveMatrix{};
		glm::mat4 orthogonalMatrix{};
		
		// near and far are constants in some particle shaders. don't forget to change them;
		float fov = 90.f, zNear = 0.1f, zFar = 400.f;

		VertexBuffer pipe3lineVB{};
		IndexBuffer pipe3lineIB{};
		VertexArray pipe3lineVA{};
		std::vector<float> pipe3lines;

		struct Cube {
			glm::mat4 matrix; // includes scale
			glm::vec3 color;
		};
		struct Sphere {
			glm::vec3 position;
			float radius;
			glm::vec3 color;
		};
		VertexArray cubeVAO{};
		VertexBuffer cubeVBO{};
		VertexBuffer cubeInstanceVBO{};
		IndexBuffer cubeIBO{};

		VertexArray cube2VAO{};
		VertexBuffer cube2VBO{};
		IndexBuffer cube2IBO{}; // NOTE: this is the same as cubeIBO. Could be removed.(altough, not removing this would ensure that changing the other cubes values won't affect this one)

		VertexBuffer quadVB{};
		IndexBuffer quadIB{};
		VertexArray quadVA{};

		VertexBuffer instanceBuffer;

		//struct ModelDraw {
		//	ModelAsset* modelAsset;
		//	glm::mat4 matrix;
		//	bool isOrthogonal;
		//};
		//std::vector<ModelDraw> modelObjects;

		std::vector<Cube> cubeObjects;

		float verts[16 * TEXT_BATCH]{}; // 16 floats, 4 corners for a letter, each corner has x,y,u,v
		VertexBuffer textVBO{};
		VertexArray textVAO{};
		IndexBuffer textIBO{};

		//VertexArray boxVAO{};
		//VertexBuffer boxVBO{};
		//IndexBuffer boxIBO{};
		//float floatArray[4 * VERTEX_SIZE * MAX_BOX_BATCH]{};

		Camera camera;

		friend class Window;
		friend class Engone;
	};
	void EnableBlend();
	void EnableDepth();
	CommonRenderer* GetActiveRenderer();
}