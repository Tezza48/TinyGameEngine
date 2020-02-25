#pragma once
#include "Material.h"
#include <vector>
#include "VertexStandard.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace TinyEngine
{
	class Renderer;

	// Class representing a 3D model.
	class Mesh
	{
	private:
		struct MeshPart {
			Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;
			unsigned int size;
			unsigned int baseVertex;
		};

		Renderer* _renderer;

		Microsoft::WRL::ComPtr<ID3D11Buffer> _vertexBuffer;
		unsigned int _numVertices;

		std::vector<MeshPart> _parts;

	public:
		// Construct a Mesh instance.
		//	Renderer* renderer: referance to the renderer which this belongs to.
		Mesh(Renderer* renderer);
		~Mesh();

		Mesh(const Mesh&) = delete;

		// Set the vertices of this mesh.
		//	VertexStandard* vertices: Array of vertex data
		//	unsigned int numVertices: Number of vertices in the vertex array
		void SetVertices(VertexStandard* vertices, unsigned int numVertices);

		// Add an index buffer to this mesh.
		//	unsigned int* indices: Array of indices
		//	unsigned int* numIndices: number of indices in the index array
		//	unsigned int baseVertex: the vertex which this index buffer will
		//		use as the first one when drawing. Lets you reuse vertices.
		void AddIndexBuffer(unsigned int* indices, unsigned int numIndices, unsigned int baseVertex = 0);

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		Microsoft::WRL::ComPtr<ID3D11Buffer> GetVertexBuffer() const
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
