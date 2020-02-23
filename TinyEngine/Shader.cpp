#define TINY_ENGINE_EXPOSE_NATIVE
#include "Renderer.h"

#include "Shader.h"

#include <iostream>
#include <fstream>

using std::cout;
using std::endl;
using std::ifstream;

TinyEngine::Shader::Shader(Renderer* renderer, const char* vSBytecode, size_t vSSize, const char* pSBytecode, size_t pSSize, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount):
	_renderer(renderer)
{
	auto device = _renderer->GetDevice();

	// TODO: Helper methods to create from compiled source to remove this duplication
	HRESULT hr;
	hr = device->CreateVertexShader(vSBytecode, vSSize, nullptr, &_vertexShader);
	if (FAILED(hr))
	{
		cout << "Failed to create Vertex Shader." << endl;
		return;
	}

	hr = device->CreatePixelShader(pSBytecode, pSSize, nullptr, &_pixelShader);
	if (FAILED(hr))
	{
		cout << "Failed to create Pixel Shader." << endl;
		return;
	}

	hr = device->CreateInputLayout(inputDescs, inputDescCount, vSBytecode, vSSize, &_inputLayout);
	if (FAILED(hr))
	{
		cout << "Failed to create Input Layout." << endl;
		return;
	}
}

TinyEngine::Shader::Shader(Renderer* renderer, const char* vertexPath, const char* pixelPath, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount) :
	_renderer(renderer)
{
	auto device = _renderer->GetDevice();

	size_t vsLength = 0, psLength = 0;
	char* vsBytes = nullptr;
	char* psBytes = nullptr;

	const auto openFlags = std::ios::in | std::ios::binary | std::ios::ate;

	ifstream file(vertexPath, openFlags);

	if (file.is_open())
	{
		auto size = file.tellg();

		vsLength = static_cast<size_t>(size);
		vsBytes = new char[vsLength];

		file.seekg(0, std::ios::beg);
		file.read(vsBytes, size);

		file.close();
	}
	else
	{
		cout << "Could not open Vertex Shader at: " << vertexPath << endl;
	}

	file.open(pixelPath, openFlags);

	if (file.is_open())
	{
		auto size = file.tellg();

		psLength = static_cast<size_t>(size);
		psBytes = new char[psLength];

		file.seekg(0, std::ios::beg);
		file.read(psBytes, size);

		file.close();
	}
	else
	{
		cout << "Could not open Pixel Shader at: " << pixelPath << endl;
	}


	// TODO: Helper methods to create from compiled source to remove this duplication
	HRESULT hr;
	hr = device->CreateVertexShader(vsBytes, vsLength, nullptr, &_vertexShader);
	if (FAILED(hr))
	{
		cout << "Failed to create Vertex Shader." << endl;
		return;
	}

	hr = device->CreatePixelShader(psBytes, psLength, nullptr, &_pixelShader);
	if (FAILED(hr))
	{
		cout << "Failed to create Pixel Shader." << endl;
		return;
	}

	hr = device->CreateInputLayout(inputDescs, inputDescCount, vsBytes, vsLength, &_inputLayout);
	if (FAILED(hr))
	{
		cout << "Failed to create Input Layout." << endl;
		return;
	}

	delete[] vsBytes;
	delete[] psBytes;
}
