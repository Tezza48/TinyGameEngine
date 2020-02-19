#pragma once

#include <DirectXMath.h>
#include "Texture.h"
#include "Shader.h"

namespace TinyEngine
{
	struct Material
	{
	public:
		Shader* shader = nullptr;
		DirectX::XMFLOAT3 specular = {};
		float specularExponent = 0.0f;
		DirectX::XMFLOAT3 ambient = {};
		float transparency = 0.0f;
		DirectX::XMFLOAT3 diffuse = {};
		Texture* diffuseTexture;
		Texture* specularTexture;
		Texture* ambientTexture;
	};
}

