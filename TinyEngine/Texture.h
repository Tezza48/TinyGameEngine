#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>

namespace TinyEngine
{
	class Renderer;

	// Class representing a Texture.
	class Texture
	{
	private:
		Renderer* _renderer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _textureView;

	public:
		// Construct a Texture with no initial data.
		//	Renderer* renderer: Renderer which this Texture belongs to
		Texture(Renderer* renderer);

		// Construct a Texture with initial data.
		//	Renderer* renderer: Renderer which this Texture belongs to
		//	const unsigned char* data: Texture data. RGBA unorm
		//  int width: Width of this texture
		//	int height: Height of this texture
		Texture(Renderer* renderer, const unsigned char* data, int width, int height);

		Texture(const Texture&) = delete;
		~Texture() = default;

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> GetTextureView() const
		{
			return _textureView;
		}
#endif
	};
}
