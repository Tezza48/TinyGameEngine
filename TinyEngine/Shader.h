#pragma once

#include<d3d11.h>
#include <wrl/client.h>

namespace TinyEngine
{
	class Renderer;

	// A class representing a collection of shader programs.
	class Shader
	{
	private:
		Renderer* _renderer;

		Microsoft::WRL::ComPtr<ID3D11VertexShader> _vertexShader;
		Microsoft::WRL::ComPtr<ID3D11PixelShader> _pixelShader;
		Microsoft::WRL::ComPtr<ID3D11InputLayout> _inputLayout;

	public:
		// Construct a shader from pre compiled bytecode.
		// Too long, CBA. Not used at the moment anyway.
		Shader(Renderer* renderer, const char* vSBytecode, size_t vSSize, const char* pSBytecode, size_t pSSize, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount);

		// Construct a shader from a precompiled shader binary.
		//	Renderer* renderer: Renderer to create internal objects with
		//	const char* vertexPath: Path to the compiled vertex shader file
		//	const char* pixelPath: Path to the compiled pixel shader file
		//	D3D11_INPUT_ELEMENT_DESC* inputDescs: Array of input descriptions for the vertex shader
		//	unsigned int inputDescCount: Number of elements in the inputDescs array
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
