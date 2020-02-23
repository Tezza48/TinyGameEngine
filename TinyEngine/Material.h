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
		
		DirectX::XMFLOAT3 ambient = {};
		Texture* ambientTexture = nullptr;

		DirectX::XMFLOAT3 diffuse = {};
		Texture* diffuseTexture = nullptr;
		
		DirectX::XMFLOAT3 specular = {};
		float specularExponent = 0.0f;
		Texture* specularTexture = nullptr;

		float transparency = 0.0f;

	public:
		Material();
		Material(const Material& material);

		~Material();
	};
}

