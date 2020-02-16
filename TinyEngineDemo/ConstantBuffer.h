#pragma once

#include <d3d11.h>
#include <iostream>

#include "IRenderer.h"

namespace TinyEngine
{
	template<typename T>
	class ConstantBuffer
	{
	private:
		IRenderer* _renderer;

		ID3D11Buffer* _buffer;

	public:
		ConstantBuffer(IRenderer* renderer);
		~ConstantBuffer();

		void Upload(const T& data);

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		ID3D11Buffer* GetBuffer() const
		{
			return _buffer;
		}
#endif
	};
}


template<typename T>
inline TinyEngine::ConstantBuffer<T>::ConstantBuffer(IRenderer* renderer): _renderer(renderer)
{
	D3D11_BUFFER_DESC desc = {};
	desc.ByteWidth = sizeof(T);
	desc.Usage = D3D11_USAGE_DYNAMIC;
	desc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	desc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	desc.MiscFlags = NULL;
	desc.StructureByteStride = 0;

	T emptyBuffer = {};

	D3D11_SUBRESOURCE_DATA initialData;
	initialData.pSysMem = &emptyBuffer;

	HRESULT hr = _renderer->GetDevice()->CreateBuffer(&desc, &initialData, &_buffer);
	if (FAILED(hr))
	{
		std::cout << "Failed to create Constant Buffer" << std::endl;
	}
}

template<typename T>
inline TinyEngine::ConstantBuffer<T>::~ConstantBuffer()
{
	_buffer->Release();
	_buffer = nullptr;
}

template<typename T>
inline void TinyEngine::ConstantBuffer<T>::Upload(const T& data)
{
	D3D11_MAPPED_SUBRESOURCE mappedData;

	auto context = _renderer->GetImmediateContext();

	context->Map(_buffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedData);

	memcpy(mappedData.pData, &data, sizeof(T));

	context->Unmap(_buffer, 0);
}
