#pragma once

#include <DirectXMath.h>

namespace TinyEngine
{
	class ICamera
	{
	public:
		virtual DirectX::XMFLOAT3 GetEyePosition() = 0;
		virtual DirectX::XMMATRIX GetView() = 0;
		virtual DirectX::XMMATRIX GetProjection() = 0;
	};
}

