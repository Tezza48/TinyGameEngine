#pragma once

#include <DirectXMath.h>
#include "Texture.h"
#include "Shader.h"

namespace TinyEngine
{
	// Standard material used by the renderer.
	// Any unused textures must contain a texture created
	// with this constructor (null object): Texture(renderer);
	// Not the best solution but it made the renderer simpler.
	struct Material
	{
	public:
		// Shader which this material uses.
		Shader* shader = nullptr;
		
		// Ambient component of this material. (Emissive)
		DirectX::XMFLOAT3 ambient = {};
		// Ambient texture.
		Texture* ambientTexture = nullptr;

		// Diffuse color of this material.
		DirectX::XMFLOAT3 diffuse = {};
		// Diffuse texture.
		Texture* diffuseTexture = nullptr;
		
		// Specular color of this material.
		DirectX::XMFLOAT3 specular = {};
		// Specular exponent of this material. (sharpness of the reflection)
		float specularExponent = 0.0f;
		// Specular texture. RGB = specular, A = specular exponent.
		Texture* specularTexture = nullptr;

		// Transparency. Unused by the default shader.
		float transparency = 0.0f;

	public:
		Material();
		Material(const Material& material);

		~Material();
	};
}

