#include "VertexStandard.h"

using namespace TinyEngine;

TinyEngine::VertexStandard::VertexStandard(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 texcoord, DirectX::XMFLOAT3 normal)
{
	this->position = position;
	this->texcoord = texcoord;
	this->normal = normal;
}
