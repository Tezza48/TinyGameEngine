#pragma once

#include <d3d11.h>
#include <dxgi.h>
#include <wrl/client.h>
#include "IRenderer.h"

namespace TinyEngine
{
	class Texture
	{
	private:
		IRenderer* _renderer;
		Microsoft::WRL::ComPtr<ID3D11ShaderResourceView> _textureView;

	public:
		Texture(IRenderer* renderer);
		Texture(IRenderer* renderer, const unsigned char* data, int width, int height);

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
