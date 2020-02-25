#pragma once

#include <DirectXMath.h>

namespace TinyEngine
{
	// Standard vertex type.
	struct VertexStandard {
	public:
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
		DirectX::XMFLOAT3 normal;

	public:
		VertexStandard(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 texcoord, DirectX::XMFLOAT3 normal);
	};
}