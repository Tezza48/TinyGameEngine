#pragma once

class IRenderer
{
public:
	virtual ~IRenderer() {}

#ifdef TINY_ENGINE_EXPOSE_NATIVE
	virtual ID3D11Device* GetDevice() const = 0;
	virtual ID3D11DeviceContext* GetImmediateContext() const = 0;
#endif
};