#define TINY_ENGINE_EXPOSE_NATIVE
#include "Texture.h"
#include "IRenderer.h"
#include <iostream>

using std::cout;
using std::endl;
using Microsoft::WRL::ComPtr;

TinyEngine::Texture::Texture(IRenderer* renderer): _renderer(renderer)
{
}

TinyEngine::Texture::Texture(IRenderer* renderer, const unsigned char* data, int width, int height) : _renderer(renderer)
{
	D3D11_TEXTURE2D_DESC desc = {};
	desc.Width = width;
	desc.Height = height;
	desc.MipLevels = 0;
	desc.ArraySize = 1;
	desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	desc.SampleDesc.Count = 1;
	desc.SampleDesc.Quality = 0;
	desc.Usage = D3D11_USAGE_DEFAULT;
	desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
	desc.CPUAccessFlags = DXGI_CPU_ACCESS_NONE;
	desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

	auto device = _renderer->GetDevice();
	auto context = _renderer->GetImmediateContext();

	ComPtr<ID3D11Texture2D> texture;

	HRESULT hr = device->CreateTexture2D(&desc, nullptr, &texture);
	if (FAILED(hr) || !texture)
	{
		cout << "Failed to Create Texture2D" << endl;

		return;
	}

	unsigned int rowPitch = width * 4 * sizeof(unsigned char);

	context->UpdateSubresource(texture.Get(), 0, nullptr, data, rowPitch, 0);

	D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
	srvDesc.Format = desc.Format;
	srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
	srvDesc.Texture2D.MostDetailedMip = 0;
	srvDesc.Texture2D.MipLevels = -1;

	hr = device->CreateShaderResourceView(texture.Get(), &srvDesc, &_textureView);
	if (FAILED(hr) || !_textureView)
	{
		cout << "Failed to Create view to Texture2D" << endl;

		return;
	}

	context->GenerateMips(_textureView.Get());
}
