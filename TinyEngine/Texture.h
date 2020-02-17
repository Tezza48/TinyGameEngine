#pragma once

#include <d3d11.h>
#include <dxgi.h>

#include "IRenderer.h"

namespace TinyEngine
{
	class Texture
	{
	private:
		IRenderer* _renderer;
		ID3D11ShaderResourceView* _textureView;

	public:
		Texture(IRenderer* renderer);
		Texture(IRenderer* renderer, const unsigned char* data, int width, int height);

		Texture(const Texture&) = delete;
		~Texture();

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		ID3D11ShaderResourceView* GetTextureView() const
		{
			return _textureView;
		}
#endif
	};
}
