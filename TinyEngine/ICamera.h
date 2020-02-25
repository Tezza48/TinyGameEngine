#pragma once

#include <DirectXMath.h>

namespace TinyEngine
{
	// Interface representing a camera used to render the game world.
	class ICamera
	{
	public:
		// Get the current "Eye" position of the camera (where the camera is).
		virtual DirectX::XMFLOAT3 GetEyePosition() = 0;

		// Get the view matrix.
		virtual DirectX::XMMATRIX GetView() = 0;

		// Get the projection matrix.
		virtual DirectX::XMMATRIX GetProjection() = 0;
	};
}

