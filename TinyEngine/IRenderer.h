#pragma once
#include <WRL/client.h>

class IRenderer
{
public:
	virtual ~IRenderer() = default;

#ifdef TINY_ENGINE_EXPOSE_NATIVE
	virtual Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() const = 0;
	virtual Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetImmediateContext() const = 0;
#endif
};