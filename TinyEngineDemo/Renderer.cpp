#define TINY_ENGINE_EXPOSE_NATIVE
// Define this so we can get Win32 internals from a window instance.
#include "Window.h"
#include "Mesh.h"

#include "Renderer.h"
#include "EngineEventType.h"
#include <DirectXMath.h>
#include <iostream>
#include <comdef.h>

using namespace DirectX;

using std::cout;
using std::endl;

#define CHECK_HR(hr, message) if (FAILED(hr)) {_com_error err(hr); cout << message << "\n\t" << err.ErrorMessage() << std::endl; }

TinyEngine::Renderer::Renderer(int width, int height, Window& window)
{
	UINT createDeviceFlags = {};
	createDeviceFlags |= D3D11_CREATE_DEVICE_SINGLETHREADED;

#if DEBUG || _DEBUG
	createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_11_1
	};

	// TODO: Check for MS support.

	DXGI_SWAP_CHAIN_DESC scd = {};
	scd.BufferDesc.Width = width;
	scd.BufferDesc.Height = height;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2;
	scd.OutputWindow = window.GetWindow();
	scd.Windowed = true;
	scd.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
	scd.Flags = NULL;

	HRESULT hr;

	hr = D3D11CreateDeviceAndSwapChain(
		nullptr,
		D3D_DRIVER_TYPE_HARDWARE,
		nullptr,
		createDeviceFlags,
		featureLevels, 1,
		D3D11_SDK_VERSION,
		&scd,
		&_swapChain,
		&_device,
		nullptr,
		&_immediateContext);

	if (FAILED(hr))
	{
		cout << "Failed to create Device and Swapchain." << endl;
	}

	BindCurrentBackBufferView();

	D3D11_TEXTURE2D_DESC dstd = {};
	dstd.Width = width;
	dstd.Height = height;
	dstd.MipLevels = 1;
	dstd.ArraySize = 1;
	dstd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dstd.SampleDesc.Count = 1;
	dstd.SampleDesc.Quality = 0;
	dstd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dstd.CPUAccessFlags = DXGI_CPU_ACCESS_NONE;
	dstd.MiscFlags = NULL;

	ID3D11Texture2D* depthStencil = nullptr;

	hr = _device->CreateTexture2D(&dstd, nullptr, &depthStencil);
	CHECK_HR(hr, "Failed to create depthStencil texture.");

	if (!depthStencil)
	{
		cout << __FILE__ << "(" << __LINE__ << ") " << "DepthStencil Texture was nullptr" << endl;

		return;
	}

	hr = _device->CreateDepthStencilView(depthStencil, 0, &_depthStencilView);
	CHECK_HR(hr, "Failed to create depthStencilView.");

	depthStencil->Release();
	depthStencil = nullptr;

	// needs re calling on swap chain present. Additionally i think the back buffer view needs to be updated to point to the new one.
	_immediateContext->OMSetRenderTargets(1, &_backBufferView, _depthStencilView);

	D3D11_RASTERIZER_DESC rd = {};
	rd.FillMode = D3D11_FILL_SOLID;
	rd.CullMode = D3D11_CULL_BACK;
	rd.FrontCounterClockwise = false;
	rd.DepthBias = 0;
	rd.DepthBiasClamp = 0.0f;
	rd.SlopeScaledDepthBias = 0;
	rd.DepthClipEnable = false;
	rd.ScissorEnable = false;
	rd.MultisampleEnable = false;
	rd.AntialiasedLineEnable = false;

	hr = _device->CreateRasterizerState(&rd, &_defaultRasterizerState);

	D3D11_SAMPLER_DESC samplerDesc;
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	hr = _device->CreateSamplerState(&samplerDesc, &_defaultSamplerState);
	CHECK_HR(hr, "Failed to create Sampler State");

	UpdateViewport(0, 0, width, height);

	D3D11_INPUT_ELEMENT_DESC inputDescs[3] = {
		{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
		{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
		{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA}
	};

	_defaultShader = new Shader(this, "./assets/shader/defaultVertexShader.cso", "./assets/shader/defaultPixelShader.cso", inputDescs, 3);
	
	_perObjectCB = new ConstantBuffer<PerObjectCBData>(this);
	_perMaterialCB = new ConstantBuffer<PerMaterialCBData>(this);
}

TinyEngine::Renderer::~Renderer()
{
	delete _perMaterialCB;
	_perMaterialCB = nullptr;

	delete _perObjectCB;
	_perObjectCB = nullptr;

	delete _defaultShader;
	_defaultShader = nullptr;


	_defaultSamplerState->Release();
	_defaultSamplerState = nullptr;

	_defaultRasterizerState->Release();
	_defaultRasterizerState = nullptr;

	_depthStencilView->Release();
	_depthStencilView = nullptr;

	_backBufferView->Release();
	_backBufferView = nullptr;

	_swapChain->Release();
	_swapChain = nullptr;

	_immediateContext->Release();
	_immediateContext = nullptr;

	_device->Release();
	_device = nullptr;
}

void TinyEngine::Renderer::SetClearColor(DirectX::XMFLOAT4 color)
{
	_clearColor = color;
}

void TinyEngine::Renderer::Clear()
{
	_immediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL
		, 1.0f, 0);
	_immediateContext->ClearRenderTargetView(_backBufferView, reinterpret_cast<float*>(&_clearColor));

}

void TinyEngine::Renderer::SwapBuffers()
{
	_swapChain->Present(0, 0);

	_backBufferView->Release();
	_backBufferView = nullptr;

	BindCurrentBackBufferView();

	_immediateContext->OMSetRenderTargets(1, &_backBufferView, _depthStencilView);
}

void TinyEngine::Renderer::DrawMesh(Mesh* mesh, ICamera* camera, XMMATRIX world)
{
	auto context = _immediateContext;

	const unsigned int stride = sizeof(VertexStandard);
	const unsigned int offset = 0;

	auto* vertexBuffer = mesh->GetVertexBuffer();

	context->IASetVertexBuffers(0, 1, &vertexBuffer, &stride, &offset);
	context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	context->IASetInputLayout(_defaultShader->GetInputLayout());

	context->VSSetShader(_defaultShader->GetVertexShader(), nullptr, 0);
	context->PSSetShader(_defaultShader->GetPixelShader(), nullptr, 0);

	context->PSSetSamplers(0, 1, &_defaultSamplerState);

	// TODO: CBuffers
	PerObjectCBData objCb;

	memcpy(objCb.lights, lights, sizeof(lights));

	objCb.ambientLight = ambientLight;
	objCb.world = XMMatrixTranspose(world);

	auto det = XMMatrixDeterminant(world);
	objCb.worldInverseTranspose = XMMatrixInverse(&det, world);

	objCb.view = XMMatrixTranspose(camera->GetView());
	objCb.projection = XMMatrixTranspose(camera->GetProjection());
	objCb.eyePosW = camera->GetEyePosition();

	_perObjectCB->Upload(objCb);

	{
		auto buffer = _perObjectCB->GetBuffer();
		context->VSSetConstantBuffers(0, 1, &buffer);
		context->PSSetConstantBuffers(0, 1, &buffer);
	}

	if (mesh->GetNumMeshParts() > 0)
	{
		for (int i = 0; i < mesh->GetNumMeshParts(); i++)
		{
			auto part = mesh->GetMeshPart(i);

			PerMaterialCBData matCb;
			matCb.mat.diffuse = part.mat->diffuse;
			matCb.mat.ambient = part.mat->ambient;
			matCb.mat.specular = part.mat->specular;
			matCb.mat.specularExponent = part.mat->specularExponent;
			matCb.mat.transparency = part.mat->transparency;

			_perMaterialCB->Upload(matCb);

			// set textures from material

			{
				auto buffer = _perMaterialCB->GetBuffer();
				context->VSSetConstantBuffers(1, 1, &buffer);
				context->PSSetConstantBuffers(1, 1, &buffer);
			}

			context->IASetIndexBuffer(part.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

			context->DrawIndexed(part.size, 0, part.baseVertex);
		}
	}
	else
	{
		context->Draw(mesh->GetNumVertices(), 0);
	}
}

void TinyEngine::Renderer::OnNotify(const Event& event)
{
	auto type = static_cast<EngineEventType>(event.GetType());

	switch (type)
	{
    case EngineEventType::WINDOW_CREATED:
        break;
    case EngineEventType::WINDOW_CLOSE:
        break;
    case EngineEventType::WINDOW_RESIZE:
	{
        const auto& resizeEvent = static_cast<const ResizeEvent&>(event);
        OnResize(resizeEvent.x, resizeEvent.y);

        break;
	}
    case EngineEventType::WINDOW_KEY_DOWN:
        break;
    case EngineEventType::WINDOW_KEY_UP:
        break;
    default:
        break;
	}
}

void TinyEngine::Renderer::BindCurrentBackBufferView()
{
	ID3D11Texture2D* backBuffer = nullptr;

	HRESULT hr;
	hr = _swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	CHECK_HR(hr, "Failed to get backbuffer from swap chain.");

	if (!backBuffer)
	{
		cout << __FILE__ << "(" << __LINE__ << ") " << "BackBuffer was nullptr" << endl;
		return;
	}

	hr = _device->CreateRenderTargetView(backBuffer, NULL, &_backBufferView);
	CHECK_HR(hr, "Failed to create render target view to backBuffer.");

	backBuffer->Release();
	backBuffer = nullptr;
}

void TinyEngine::Renderer::UpdateViewport(int x, int y, int width, int height)
{
	D3D11_VIEWPORT viewport = {};
	viewport.TopLeftX = static_cast<float>(x);
	viewport.TopLeftY = static_cast<float>(y);
	viewport.Width = static_cast<float>(width);
	viewport.Height = static_cast<float>(height);
	viewport.MinDepth = 0;
	viewport.MaxDepth = 1.0;

	_immediateContext->RSSetViewports(1, &viewport);
}

void TinyEngine::Renderer::OnResize(int width, int height)
{
	_backBufferView->Release();
	_backBufferView = nullptr;

	_depthStencilView->Release();
	_depthStencilView = nullptr;

	DXGI_SWAP_CHAIN_DESC scd = {};
	_swapChain->GetDesc(&scd);
	_swapChain->ResizeBuffers(2, width, height, scd.BufferDesc.Format, NULL);

	BindCurrentBackBufferView();

	D3D11_TEXTURE2D_DESC dstd = {};
	dstd.Width = width;
	dstd.Height = height;
	dstd.MipLevels = 1;
	dstd.ArraySize = 1;
	dstd.Format = DXGI_FORMAT_D24_UNORM_S8_UINT;
	dstd.SampleDesc.Count = 1;
	dstd.SampleDesc.Quality = 0;
	dstd.BindFlags = D3D11_BIND_DEPTH_STENCIL;
	dstd.CPUAccessFlags = DXGI_CPU_ACCESS_NONE;
	dstd.MiscFlags = NULL;

	ID3D11Texture2D* depthStencil = nullptr;

	HRESULT hr;
	hr = _device->CreateTexture2D(&dstd, nullptr, &depthStencil);
	CHECK_HR(hr, "Failed to create depthStencil texture.");

	hr = _device->CreateDepthStencilView(depthStencil, 0, &_depthStencilView);
	CHECK_HR(hr, "Failed to create depthStencilView.");

	depthStencil->Release();
	depthStencil = nullptr;

	// needs re calling on swap chain present. Additionally i think the back buffer view needs to be updated to point to the new one.
	_immediateContext->OMSetRenderTargets(1, &_backBufferView, _depthStencilView);

	UpdateViewport(0, 0, width, height);
}
