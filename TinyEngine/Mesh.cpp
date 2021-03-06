#define TINY_ENGINE_EXPOSE_NATIVE
#include "Renderer.h"

#include "Mesh.h"
#include <iostream>

using namespace TinyEngine;

using std::cout;
using std::endl;
using std::vector;

Mesh::Mesh(Renderer* renderer) : _renderer(renderer), _numVertices(0)
{

}

Mesh::~Mesh()
{

}

void Mesh::SetVertices(VertexStandard* vertices, unsigned int numVertices)
{
	_numVertices = numVertices;

	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = numVertices * sizeof(VertexStandard);
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
	bd.CPUAccessFlags = NULL;
	bd.MiscFlags = NULL;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA pData = {};
	pData.pSysMem = vertices;

	HRESULT hr;
	hr = _renderer->GetDevice()->CreateBuffer(&bd, &pData, &_vertexBuffer);
	if (FAILED(hr))
	{
		cout << "Failed to create Vertex Buffer." << endl;
	}
}

void Mesh::AddIndexBuffer(unsigned int* indices, unsigned int numIndices, unsigned int baseVertex)
{
	D3D11_BUFFER_DESC bd;
	bd.ByteWidth = numIndices * sizeof(unsigned int);
	bd.Usage = D3D11_USAGE_IMMUTABLE;
	bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
	bd.CPUAccessFlags = NULL;
	bd.MiscFlags = NULL;
	bd.StructureByteStride = 0;

	D3D11_SUBRESOURCE_DATA pData = {};
	pData.pSysMem = indices;

	Microsoft::WRL::ComPtr<ID3D11Buffer> indexBuffer;

	HRESULT hr;
	hr = _renderer->GetDevice()->CreateBuffer(&bd, &pData, &indexBuffer);
	if (FAILED(hr))
	{
		cout << "Failed to create Index Buffer." << endl;
	}

	_parts.push_back({ indexBuffer, numIndices, baseVertex });
}
