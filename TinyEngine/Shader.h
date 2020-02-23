#pragma once

#include<d3d11.h>
#include <wrl/client.h>

namespace TinyEngine
{
	class Renderer;

	class Shader
	{
	private:
		Renderer* _renderer;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> _inputLayout;

	public:
		Shader(Renderer* renderer, const char* vSBytecode, size_t vSSize, const char* pSBytecode, size_t pSSize, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount);
		Shader(Renderer* renderer, const char* vertexPath, const char* pixelPath, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount);
		~Shader() = default;

		Shader(const Shader&) = delete;

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		Microsoft::WRL::ComPtr<ID3D11VertexShader> GetVertexShader() const
		{
			return _vertexShader;
		}

		Microsoft::WRL::ComPtr<ID3D11PixelShader> GetPixelShader() const
		{
			return _pixelShader;
		}

		Microsoft::WRL::ComPtr<ID3D11InputLayout> GetInputLayout() const
		{
			return _inputLayout;
		}
#endif
	};
}
