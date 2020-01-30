#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include <comdef.h>
#include <DirectXMath.h>

#define SAFE_RELEASE(comptr) if (comptr) { comptr->Release(); comptr = nullptr; }

#define PRINT_ERROR(x) std::cerr << __FILE__ << ":" << __LINE__ << " " << x << std::endl

#define CHECK_HR(hr, message) if (FAILED(hr)) {_com_error err(hr); PRINT_ERROR(message << "\n\t" << err.ErrorMessage()); }

class TinyEngine
{
public:
protected:
	//Assets _assets;

private:
	HWND _window = 0;
	bool isRunning;

	ID3D11Device* _device;
	ID3D11DeviceContext* _immediateContext;
	IDXGISwapChain* _swapChain;

	ID3D11RenderTargetView* _backBufferView;
	ID3D11DepthStencilView* _depthStencilView;

	int _width;
	int _height;
	const char* _title;

	DirectX::XMFLOAT4 _clearColor;

public:
	TinyEngine(int width, int height, const char* Title);
	~TinyEngine();

	void Run();

protected:
	virtual void OnInit() = 0;
	virtual bool OnUpdate() = 0;

	//void DrawTexture();

private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);

	void InitWin32();
	void InitD3D();

	void OnResize(int width, int height);

	void SwapBuffers();
	void BindCurrentBackBufferView();
};
//
//class Mesh {
//
//};
//
//class Shader {
//
//};
//
//class Input {
//
//};
//
//class Scene {
//
//};
//
//class Assets {
//public:
//	//Shader* LoadShader(const char* path);
//	//Mesh* LoadMesh(const char* path);
//};

#ifdef TINY_ENGINE_IMPLEMENTATION

#ifdef TINY_ENGINE_LAZY_LIBS

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

#endif

#include <iostream>

using std::cerr;
using std::endl;

using namespace DirectX;

inline LRESULT CALLBACK TinyEngine::WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	static TinyEngine* engine;

	switch (uMsg)
	{
	case WM_CREATE:
		engine = reinterpret_cast<TinyEngine*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
		return 0;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		engine->isRunning = false;
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;

	case WM_SIZE:
		const auto width = LOWORD(lparam);
		const auto height = HIWORD(lparam);
		engine->OnResize(width, height);
		return 0;
	}

	return DefWindowProc(hwnd, uMsg, wparam, lparam);;
}

inline void TinyEngine::InitWin32()
{
	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "TinyEngine";

	if (!RegisterClass(&wc))
	{
		PRINT_ERROR("Failed to register class.");
	}

	DWORD dwStyleEX = NULL;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	RECT windowRect = { 0, 0, _width, _height };
	AdjustWindowRectEx(&windowRect, dwStyle, false, dwStyleEX);

	_window = CreateWindowEx(
		dwStyleEX,
		wc.lpszClassName,
		_title,
		dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		wc.hInstance,
		this);

	if (!_window)
	{
		PRINT_ERROR(" Failed to Create Window.");
	}
}

inline void TinyEngine::InitD3D()
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
	scd.BufferDesc.Width = _width;
	scd.BufferDesc.Height = _height;
	scd.BufferDesc.RefreshRate.Numerator = 60;
	scd.BufferDesc.RefreshRate.Denominator = 1;
	scd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	scd.BufferDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	scd.BufferDesc.Scaling = DXGI_MODE_SCALING_STRETCHED;
	scd.SampleDesc.Count = 1;
	scd.SampleDesc.Quality = 0;
	scd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	scd.BufferCount = 2;
	scd.OutputWindow = _window;
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
		PRINT_ERROR("Failed to create Device and Swapchain.");
	}

	ID3D11Texture2D* backBuffer = nullptr;

	hr = _swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	CHECK_HR(hr, "Failed to get backbuffer from swap chain.");

	hr = _device->CreateRenderTargetView(backBuffer, NULL, &_backBufferView);
	CHECK_HR(hr, "Failed to create render target view to backBuffer.");

	backBuffer->Release();
	backBuffer = nullptr;

	D3D11_TEXTURE2D_DESC dstd = {};
	dstd.Width = _width;
	dstd.Height = _height;
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

	hr = _device->CreateDepthStencilView(depthStencil, 0, &_depthStencilView);
	CHECK_HR(hr, "Failed to create depthStencilView.");

	depthStencil->Release();
	depthStencil = nullptr;

	// needs re calling on swap chain present. Additionally i think the back buffer view needs to be updated to point to the new one.
	_immediateContext->OMSetRenderTargets(1, &_backBufferView, _depthStencilView);

	//D3D11_RASTERIZER_DESC rd = {};
	//rd.FillMode = D3D11_FILL_SOLID;
	//rd.CullMode = D3D11_CULL_NONE;
	//rd.FrontCounterClockwise = false;
	//rd.DepthBias = 0;
	//rd.DepthBiasClamp = 1.0f;
	//rd.SlopeScaledDepthBias = 0;
	//rd.DepthClipEnable = true;
	//rd.ScissorEnable = false;
	//rd.MultisampleEnable = false;
	//rd.AntialiasedLineEnable = true;

	//hr = _device->CreateRasterizerState(&rd, &_defaultRasterizerState);
	// create sound stuff.
}

inline void TinyEngine::OnResize(int width, int height)
{
	if (!isRunning) {
		return;
	}

	SAFE_RELEASE(_backBufferView);
	SAFE_RELEASE(_depthStencilView);
	
	DXGI_SWAP_CHAIN_DESC scd = {};
	_swapChain->GetDesc(&scd);
	_swapChain->ResizeBuffers(2, _width, _height, scd.BufferDesc.Format, NULL);

	BindCurrentBackBufferView();
	
	D3D11_TEXTURE2D_DESC dstd = {};
	dstd.Width = _width;
	dstd.Height = _height;
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
}

inline void TinyEngine::SwapBuffers()
{
	// Swap buffers
	_swapChain->Present(0, 0);

	_backBufferView->Release();
	_backBufferView = nullptr;

	BindCurrentBackBufferView();

	_immediateContext->OMSetRenderTargets(1, &_backBufferView, _depthStencilView);
}

inline void TinyEngine::BindCurrentBackBufferView()
{
	ID3D11Texture2D* backBuffer = nullptr;

	HRESULT hr;
	hr = _swapChain->GetBuffer(0, IID_PPV_ARGS(&backBuffer));
	CHECK_HR(hr, "Failed to get backbuffer from swap chain.");

	hr = _device->CreateRenderTargetView(backBuffer, NULL, &_backBufferView);
	CHECK_HR(hr, "Failed to create render target view to backBuffer.");

	backBuffer->Release();
	backBuffer = nullptr;
}

inline TinyEngine::TinyEngine(int width, int height, const char* title)
	: _width(width), _height(height), _title(title)
{
	InitWin32();

	InitD3D();
}

inline TinyEngine::~TinyEngine()
{
	SAFE_RELEASE(_depthStencilView);
	SAFE_RELEASE(_backBufferView);
	SAFE_RELEASE(_swapChain);
	SAFE_RELEASE(_immediateContext);
	SAFE_RELEASE(_device);
}

inline void TinyEngine::Run()
{
	isRunning = true;

	OnInit();

	MSG message;

	while (isRunning)
	{
		if (PeekMessage(&message, _window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (!isRunning) {
			break;
		}

		_immediateContext->ClearDepthStencilView(_depthStencilView, D3D11_CLEAR_DEPTH | D3D11_CLEAR_STENCIL
			, 1.0f, 0);
		_immediateContext->ClearRenderTargetView(_backBufferView, reinterpret_cast<float*>(&_clearColor));

		isRunning = OnUpdate();

		SwapBuffers();
	}
}

#endif