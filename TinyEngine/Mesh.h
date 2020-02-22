#pragma once
#include "Material.h"
#include <vector>
#include "VertexStandard.h"
#include <d3d11.h>


namespace TinyEngine
{
	class Renderer;

	class Mesh
	{
	private:
		struct MeshPart {
			ID3D11Buffer* indexBuffer;
			unsigned int size;
			unsigned int baseVertex;
			Material* mat;
		};

		Renderer* _renderer;

		ID3D11Buffer* _vertexBuffer;
		unsigned int _numVertices;

		std::vector<MeshPart> _parts;

	public:
		Mesh(Renderer* renderer);
		~Mesh();

		Mesh(const Mesh&);

		void SetVertices(VertexStandard* vertices, unsigned int numVertices);
		void AddIndexBuffer(unsigned int* indices, unsigned int numIndices, unsigned int baseVertex, Material* mat = nullptr);

		Material* GetPartMaterial(size_t part);
		void SetPartMaterial(size_t part, Material* mat);

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		ID3D11Buffer* GetVertexBuffer() const
		{
			return _vertexBuffer;
		}

		unsigned int GetNumVertices() const
		{
			return _numVertices;
		}

		MeshPart GetMeshPart(size_t part) const
		{
			return _parts[part];
		}

		size_t GetNumMeshParts() const
		{
			return _parts.size();
		}
#endif
	};
}
