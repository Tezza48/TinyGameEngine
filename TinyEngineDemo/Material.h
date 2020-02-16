#pragma once

#include<DirectXMath.h>

namespace TinyEngine
{
	struct Material
	{
	public:
		DirectX::XMFLOAT3 specular;
		float specularExponent;
		DirectX::XMFLOAT3 ambient;
		float transparency;
		DirectX::XMFLOAT3 diffuse;
		//Texture* diffuseTexture;
		//Texture* specularTexture;

		Material();
		Material(DirectX::XMFLOAT3 ambient, DirectX::XMFLOAT3 diffuse, DirectX::XMFLOAT3 specular, float specularExponent, float transparency/*, OLD_Texture* diffuseTexture = nullptr*/);
	};
}

