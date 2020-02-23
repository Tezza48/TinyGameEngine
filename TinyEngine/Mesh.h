#pragma once
#include "Material.h"
#include <vector>
#include "VertexStandard.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace TinyEngine
{
	class Renderer;

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
		Mesh(Renderer* renderer);
		~Mesh();

		Mesh(const Mesh&) = delete;

		void SetVertices(VertexStandard* vertices, unsigned int numVertices);
		void AddIndexBuffer(unsigned int* indices, unsigned int numIndices, unsigned int baseVertex);

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
