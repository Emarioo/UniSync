#pragma once

#include "Engone/Assets/Asset.h"
#include "Engone/Rendering/Buffer.h"

namespace engone {
	class MaterialAsset;

	struct MeshAsset : public Asset {
	public:
		static const AssetType TYPE = AssetMesh;
		MeshAsset() : Asset(TYPE) {  };
		~MeshAsset() { cleanup(); }
		
		void cleanup() override;
		LoadFlags load(LoadFlags flag) override;

		enum class MeshType : u8 {
			Normal = 0,
			Boned
		};
		bool valid() const override;

		MeshType meshType = MeshType::Normal;
		static const int maxMaterials = 4;
		std::vector<MaterialAsset*> materials;
		VertexBuffer vertexBuffer;
		IndexBuffer indexBuffer;
		VertexArray vertexArray;

		glm::vec3 boundingPoint = { 0,0,0 }; // not average point
		float boundingRadius = 0.f;

		// static TrackerId trackerId;
	private:
		// This is a lot of data.
		// you could probably get away by compressing some of this or possibly use a struct on heap which
		// you can free when mesh is done loading.

		// note that this data isn't cleared after loading is done.
		// Values will carry over to next time you load. Things will be overwritten though.
		// all allocations are freed though. (even if an error occurs). haven't tested it though for the third time.
		u16 weightCount;
		u16 triangleCount;
		u16 pointCount;
		u16 colorCount;

		u32 triangleStride;

		int triangleNumbers;
		int trianglesSize;
		u16* triangles;

		int weightNumbers;
		int weightIndicesSize;
		int* weightIndices;
		int weightValuesSize;
		float* weightValues;

		int colorNumbers;
		int colorSize;
		float* colors;

		int pointNumbers;
		int pointSize;
		float* points;

		int vertexOutSize=0;
		float* vertexOut=nullptr;
		int triangleOutSize=0;
		u32* triangleOut=nullptr;
	};
}