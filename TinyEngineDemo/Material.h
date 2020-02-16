#pragma once

#include <DirectXMath.h>
#include "Texture.h"

namespace TinyEngine
{
	struct Material
	{
	public:
		DirectX::XMFLOAT3 specular = {};
		float specularExponent = 0.0f;
		DirectX::XMFLOAT3 ambient = {};
		float transparency = 0.0f;
		DirectX::XMFLOAT3 diffuse = {};
		Texture* diffuseTexture;
		Texture* specularTexture;
	};
}

