#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include <comdef.h>
#include <DirectXMath.h>
#include <vector>
#include <chrono>
#include <map>

#define PRINT_ERROR(x) std::cerr << __FILE__ << ":" << __LINE__ << " " << x << std::endl

#define CHECK_HR(hr, message) if (FAILED(hr)) {_com_error err(hr); PRINT_ERROR(message << "\n\t" << err.ErrorMessage()); }

#define PI 3.1416f

namespace TinyEngine
{
	class Mesh;
	class Shader;
	class Camera;
	class TinyEngineGame;

	template<typename T>
	class ConstantBuffer;

	class RenderResource
	{
		friend class TinyEngineGame;
	protected:
		static TinyEngineGame* _game;
	};

	struct Material {
	public:
		DirectX::XMFLOAT3 specular;
		float specularExponent;
		DirectX::XMFLOAT3 ambient;
		float transparency;
		DirectX::XMFLOAT3 diffuse;

		Material();
		Material(DirectX::XMFLOAT3 ambient, DirectX::XMFLOAT3 diffuse, DirectX::XMFLOAT3 specular, float specularExponent, float transparency);
	};

	struct Light
	{
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT3 position;
		float _pad;
	};

	struct PerObjectCb
	{
	public:
		Light lights[3];
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInverseTranspose;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
		DirectX::XMFLOAT3 eyePosW;
		float _pad = 0.0f;
	};

	struct PerMaterialCB
	{
		Material mat;
		float _pad = 0.0f;
	};

	class TinyEngineGame
	{
		friend class Mesh;
		friend class Shader;
		friend class ConstantBuffer<PerMaterialCB>;
		friend class ConstantBuffer<PerObjectCb>;
	public:
	protected:
		Light _lights[3];

	private:
		HWND _window = 0;
		bool isRunning;

		ID3D11Device* _device;
		ID3D11DeviceContext* _immediateContext;
		IDXGISwapChain* _swapChain;

		ID3D11RenderTargetView* _backBufferView;
		ID3D11DepthStencilView* _depthStencilView;

		ID3D11RasterizerState* _defaultRasterizerState;

		ConstantBuffer<PerObjectCb>* _perObjectConstantBuffer;
		ConstantBuffer<PerMaterialCB>* _materialConstantBuffer;

		Shader* _defaultShader;

		int _width;
		int _height;
		const char* _title;

		DirectX::XMFLOAT4 _clearColor;

		std::chrono::high_resolution_clock::time_point _startTime;
		std::chrono::high_resolution_clock::time_point _lastTime;

	public:
		TinyEngineGame(int width, int height, const char* Title);
		~TinyEngineGame();

		void Run();

	protected:
		float GetWidth() const;
		float GetHeight() const;

		void SetClearColor(DirectX::XMFLOAT4 color);

		virtual void OnInit() = 0;
		virtual bool OnUpdate(float time, float delta) = 0;

		void DrawMesh(Mesh* mesh, Camera* camera, const DirectX::XMMATRIX& world, Shader* shader = nullptr);
	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);

		void InitWin32();
		void InitD3D();

		void OnResize(int width, int height);
		void UpdateViewport();

		void SwapBuffers();
		void BindCurrentBackBufferView();
	};

	template<typename T>
	class ConstantBuffer :
		public RenderResource
	{
		friend class TinyEngineGame;
	private:
		ID3D11Buffer* _buffer;

	public:
		ConstantBuffer();
		~ConstantBuffer();

		void Upload(const T& data);
	};

	template<typename T>
	inline ConstantBuffer<T>::ConstantBuffer()
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

		HRESULT hr = _game->_device->CreateBuffer(&desc, &initialData, &_buffer);
		CHECK_HR(hr, "Failed to create Constant Buffer");
	}

	template<typename T>
	inline ConstantBuffer<T>::~ConstantBuffer()
	{
		if (_buffer) {
			_buffer->Release();
			_buffer = nullptr;
		}
	}

	template<typename T>
	inline void ConstantBuffer<T>::Upload(const T& data)
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;

		_game->_immediateContext->Map(_buffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedData);

		memcpy(mappedData.pData, &data, sizeof(T));

		_game->_immediateContext->Unmap(_buffer, 0);
	}

	struct VertexStandard {
	public:
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
		DirectX::XMFLOAT3 normal;

	public:
		VertexStandard(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 texcoord, DirectX::XMFLOAT3 normal);
	};

	class Camera
	{
	protected:
		DirectX::XMFLOAT3 _position;

		bool _dirtyView = true;
		bool _dirtyProj = true;

		DirectX::XMMATRIX _viewMatrix;
		DirectX::XMMATRIX _projectionMatrix;
	public:
		virtual DirectX::XMMATRIX GetView();
		virtual DirectX::XMMATRIX GetProjection();

		void SetPosition(DirectX::XMFLOAT3 position);
		DirectX::XMFLOAT3 GetPosition();

	protected:
		virtual void RebuildView() = 0;
		virtual void RebuildProjection() = 0;
	};

	class PerspectiveCamera :
		public Camera
	{
	private:
		DirectX::XMFLOAT3 _target;
		float _fov = DirectX::XM_PIDIV2;
		float _aspectRatio;
		float _near = 0.01f;
		float _far = 1000.0f;

	public:
		void SetFov(float fov);
		void SetAspectRatio(float aspectRatio);

		void LookAt(DirectX::XMFLOAT3 target);
		
	protected:
		// Inherited via Camera
		virtual void RebuildView() override;
		virtual void RebuildProjection() override;
	};

	class Mesh :
		public RenderResource
	{
		friend class TinyEngineGame;
	private:
		struct MeshPart {
			ID3D11Buffer* indexBuffer;
			unsigned int size;
			unsigned int baseVertex;
			Material* mat;
		};

		ID3D11Buffer* _vertexBuffer;
		unsigned int _numVertices;

		std::vector<MeshPart> _parts;

	public:
		Mesh();
		~Mesh();

		Mesh(const Mesh&) = delete;

		void SetVertices(VertexStandard* vertices, unsigned int numVertices);
		void AddIndexBuffer(unsigned int* indices, unsigned int numIndices, unsigned int baseVertex, Material* mat = nullptr);
	};

	class Shader :
		public RenderResource
	{
		friend class TinyEngineGame;
	private:
		ID3D11VertexShader* _vertexShader;
		ID3D11PixelShader* _pixelShader;
		ID3D11InputLayout* _inputLayout;

	public:
		Shader(const char* vertexShaderBytecode, size_t vertexShaderSize, const char* pixelShaderBytecode, size_t pixelShaderSize, D3D11_INPUT_ELEMENT_DESC* inputDesc, unsigned int inputDescCount);
		Shader(const char* vertexPath, const char* fragmentPath, D3D11_INPUT_ELEMENT_DESC* inputDesc, unsigned int inputDescCount);
		~Shader();

		Shader(const Shader&) = delete;
	};

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
	//	Shader* LoadShader(const char* path);
	//	Mesh* LoadMesh(const char* path);
	//};
}

#ifdef TINY_ENGINE_IMPLEMENTATION

#define SAFE_RELEASE(comptr) if (comptr) { comptr->Release(); comptr = nullptr; }
#define SAFE_DELETE(ptr) if (ptr) { delete ptr; ptr = nullptr; }

#ifdef TINY_ENGINE_LAZY_LIBS

#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3dcompiler.lib")

#endif

#include <iostream>
#include <fstream>
#include <d3dcompiler.h>
#include <string>
#include <sstream>
#include <vector>
#include <filesystem>

using std::cerr;
using std::endl;
using std::ifstream;
using std::string;
using std::cout;
using std::stringstream;
using std::map;
using std::vector;
using std::getline;
using std::chrono::high_resolution_clock;

using namespace DirectX;

namespace TinyEngine
{
	TinyEngineGame* RenderResource::_game;

	//	-	-	-	-	-	-	-	-	-	-	-	-	Material	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	Material::Material()
	{
		ambient = {};
		diffuse = {};
		specular = {};
		specularExponent = {};
		transparency = {};
	}

	Material::Material(XMFLOAT3 ambient, XMFLOAT3 diffuse, XMFLOAT3 specular, float specularExponent, float transparency)
	{
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->specular = specular;
		this->specularExponent = specularExponent;
		this->transparency = transparency;
	}

	//	-	-	-	-	-	-	-	-	-	-	-	-	VertexStandard	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	
	TinyEngine::VertexStandard::VertexStandard(XMFLOAT3 position, XMFLOAT2 texcoord, XMFLOAT3 normal)
	{
		this->position = position;
		this->texcoord = texcoord;
		this->normal = normal;
	}

	//	-	-	-	-	-	-	-	-	-	-	-	-	Camera	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	XMMATRIX Camera::GetView()
	{
		if (_dirtyView)
		{
			RebuildView();
			_dirtyView = false;
		}

		return _viewMatrix;
	}

	XMMATRIX Camera::GetProjection()
	{
		if (_dirtyProj)
		{
			RebuildProjection();
			_dirtyProj = false;
		}

		return _projectionMatrix;
	}

	void TinyEngine::Camera::SetPosition(XMFLOAT3 position)
	{
		_position = position;
		_dirtyView = true;
	}

	DirectX::XMFLOAT3 Camera::GetPosition()
	{
		return _position;
	}

	//	-	-	-	-	-	-	-	-	-	-	-	PerspectiveCamera	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	
	void PerspectiveCamera::SetFov(float fov)
	{
		_fov = fov;
		_dirtyProj = true;
	}

	void PerspectiveCamera::SetAspectRatio(float aspectRatio)
	{
		_aspectRatio = aspectRatio;
		_dirtyProj = true;
	}

	void PerspectiveCamera::LookAt(XMFLOAT3 target)
	{
		_target = target;
		_dirtyView = true;
	}

	void PerspectiveCamera::RebuildView()
	{
		XMFLOAT3 up(0.0f, 1.0f, 0.0f);
		_viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&_position), XMLoadFloat3(&_target), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
	}

	void PerspectiveCamera::RebuildProjection()
	{
		_projectionMatrix = XMMatrixPerspectiveFovLH(_fov, _aspectRatio, _near, _far);
	}

	//	-	-	-	-	-	-	-	-	-	-	-	Shader	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	Shader::Shader(const char* vertexShaderBytecode, size_t vertexShaderSize, const char* pixelShaderBytecode, size_t pixelShaderSize, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount)
	{
		auto device = _game->_device;

		HRESULT hr;
		hr = device->CreateVertexShader(vertexShaderBytecode, vertexShaderSize, nullptr, &_vertexShader);
		CHECK_HR(hr, "Failed to create Vertex Shader.");
		hr = device->CreatePixelShader(pixelShaderBytecode, pixelShaderSize, nullptr, &_pixelShader);
		CHECK_HR(hr, "Failed to create Pixel Shader.");
		
		hr = device->CreateInputLayout(inputDescs, inputDescCount, vertexShaderBytecode, vertexShaderSize, &_inputLayout);
		CHECK_HR(hr, "Failed to create Input Layout.");
	}

	Shader::Shader(const char* vertexPath, const char* pixelPath, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount)
	{
		auto device = _game->_device;

		size_t vsLength, psLength;
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
			PRINT_ERROR("Could not open Vertex Shader at: " << vertexPath);
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
			PRINT_ERROR("Could not open Pixel Shader at: " << pixelPath);
		}

		HRESULT hr;
		hr = device->CreateVertexShader(vsBytes, vsLength, nullptr, &_vertexShader);
		CHECK_HR(hr, "Failed to create Vertex Shader.");
		hr = device->CreatePixelShader(psBytes, psLength, nullptr, &_pixelShader);
		CHECK_HR(hr, "Failed to create Pixel Shader.");

		hr = device->CreateInputLayout(inputDescs, inputDescCount, vsBytes, vsLength, &_inputLayout);
		CHECK_HR(hr, "Failed to create Input Layout.");

		delete[] vsBytes;
		delete[] psBytes;
	}

	Shader::~Shader()
	{
		SAFE_RELEASE(_vertexShader);
		SAFE_RELEASE(_pixelShader);
		SAFE_RELEASE(_inputLayout);
	}

	//	-	-	-	-	-	-	-	-	-	-	-	Mesh	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	Mesh::Mesh(): _numVertices(0), _vertexBuffer(nullptr)
	{

	}

	Mesh::~Mesh()
	{
		SAFE_RELEASE(_vertexBuffer);

		for (size_t i = 0, l = _parts.size(); i < l; i++) {
			SAFE_RELEASE(_parts[i].indexBuffer);
		}
	}

	void Mesh::SetVertices(VertexStandard* vertices, unsigned int numVertices)
	{
		_numVertices = numVertices;

		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = numVertices * sizeof(VertexStandard);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = NULL;
		bd.MiscFlags = NULL;
		bd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA pData = {};
		pData.pSysMem = vertices;

		HRESULT hr;
		hr = _game->_device->CreateBuffer(&bd, &pData, &_vertexBuffer);
		CHECK_HR(hr, "Failed to create Vertex Buffer.");
	}

	void Mesh::AddIndexBuffer(unsigned int* indices, unsigned int numIndices, unsigned int baseVertex, Material* mat)
	{
		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = numIndices * sizeof(unsigned int);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_INDEX_BUFFER;
		bd.CPUAccessFlags = NULL;
		bd.MiscFlags = NULL;
		bd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA pData = {};
		pData.pSysMem = indices;

		ID3D11Buffer* indexBuffer;

		HRESULT hr;
		hr = _game->_device->CreateBuffer(&bd, &pData, &indexBuffer);
		CHECK_HR(hr, "Failed to create Vertex Buffer.");

		_parts.push_back({ indexBuffer, numIndices, baseVertex, mat });
	}

	//	-	-	-	-	-	-	-	-	-	-	TinyEngineGame	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	void TinyEngineGame::DrawMesh(Mesh* mesh, Camera* camera, const DirectX::XMMATRIX& world, Shader * shader)
	{
		if (shader == nullptr)
		{
			shader = _defaultShader;
		}

		auto context = _immediateContext;

		const unsigned int stride = sizeof(VertexStandard);
		const unsigned int offset = 0;

		context->IASetVertexBuffers(0, 1, &mesh->_vertexBuffer, &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(shader->_inputLayout);

		context->RSSetState(_defaultRasterizerState);

		context->VSSetShader(shader->_vertexShader, nullptr, 0);
		context->PSSetShader(shader->_pixelShader, nullptr, 0);

		PerObjectCb objCb;
		objCb.world = XMMatrixTranspose(world);

		XMVECTOR det = XMMatrixDeterminant(world);
		objCb.worldInverseTranspose = XMMatrixInverse(&det, world);

		objCb.view = XMMatrixTranspose(camera->GetView());
		objCb.projection = XMMatrixTranspose(camera->GetProjection());
		objCb.eyePosW = camera->GetPosition();

		memcpy(objCb.lights, _lights, sizeof(_lights));

		_perObjectConstantBuffer->Upload(objCb);

		context->VSSetConstantBuffers(0, 1, &_perObjectConstantBuffer->_buffer);
		context->PSSetConstantBuffers(0, 1, &_perObjectConstantBuffer->_buffer);

		if (mesh->_parts.size())
		{
			for (int i = 0; i < mesh->_parts.size(); i++)
			{
				auto part = mesh->_parts[i];

				PerMaterialCB matCb;
				matCb.mat = *part.mat;

				_materialConstantBuffer->Upload(matCb);

				context->VSSetConstantBuffers(1, 1, &_materialConstantBuffer->_buffer);
				context->PSSetConstantBuffers(1, 1, &_materialConstantBuffer->_buffer);

				context->IASetIndexBuffer(part.indexBuffer, DXGI_FORMAT_R32_UINT, 0);

				// could maybe use this if the index buffer only draws from vert n. optimize the space slightly.
				context->DrawIndexed(part.size, 0, part.baseVertex);
			}
		}
		else
		{
			context->Draw(mesh->_numVertices, 0);
		}
	}

	LRESULT CALLBACK TinyEngineGame::WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
	{
		static TinyEngineGame* engine;

		switch (uMsg)
		{
		case WM_CREATE:
			engine = reinterpret_cast<TinyEngineGame*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
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

	TinyEngineGame::TinyEngineGame(int width, int height, const char* title)
		: _width(width), _height(height), _title(title)
	{
		InitWin32();

		InitD3D();

		RenderResource::_game = this;
		_lastTime = _startTime = high_resolution_clock::now();
	}

	TinyEngineGame::~TinyEngineGame()
	{
		SAFE_DELETE(_defaultShader);
		SAFE_DELETE(_materialConstantBuffer);
		SAFE_DELETE(_perObjectConstantBuffer);
		
		SAFE_RELEASE(_defaultRasterizerState);
		SAFE_RELEASE(_depthStencilView);
		SAFE_RELEASE(_backBufferView);
		SAFE_RELEASE(_swapChain);
		SAFE_RELEASE(_immediateContext);
		SAFE_RELEASE(_device);
	}

	void TinyEngineGame::InitWin32()
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

	void TinyEngineGame::InitD3D()
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
		// create sound stuff.

		UpdateViewport();
	}

	void TinyEngineGame::OnResize(int width, int height)
	{
		if (!isRunning) {
			return;
		}

		_width = width;
		_height = height;

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

		UpdateViewport();
	}

	void TinyEngineGame::SwapBuffers()
	{
		// Swap buffers
		_swapChain->Present(0, 0);

		_backBufferView->Release();
		_backBufferView = nullptr;

		BindCurrentBackBufferView();

		_immediateContext->OMSetRenderTargets(1, &_backBufferView, _depthStencilView);
	}

	void TinyEngineGame::BindCurrentBackBufferView()
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

	void TinyEngineGame::UpdateViewport()
	{
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(_width);
		viewport.Height = static_cast<float>(_height);
		viewport.MinDepth = 0;
		viewport.MaxDepth = 1.0;

		_immediateContext->RSSetViewports(1, &viewport);
	}

	void TinyEngineGame::Run()
	{
		isRunning = true;

		D3D11_INPUT_ELEMENT_DESC inputDescs[3] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA}
		};

		_defaultShader = new Shader("./assets/shaders/defaultVertexShader.cso", "./assets/shaders/defaultPixelShader.cso", inputDescs, 3);

		_perObjectConstantBuffer = new ConstantBuffer<PerObjectCb>();
		_materialConstantBuffer = new ConstantBuffer<PerMaterialCB>();

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

			auto thisTime = high_resolution_clock::now();
			auto elapsed = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(thisTime - _startTime).count()) / 1000.0f;
			auto delta = static_cast<float>(std::chrono::duration_cast<std::chrono::milliseconds>(thisTime - _lastTime).count()) / 1000.0f;

			isRunning = OnUpdate(elapsed, delta);

			_lastTime = thisTime;
			SwapBuffers();
		}
	}

	float TinyEngineGame::GetWidth() const
	{
		return static_cast<float>(_width);
	}

	float TinyEngineGame::GetHeight() const
	{
		return static_cast<float>(_height);
	}

	void TinyEngineGame::SetClearColor(XMFLOAT4 color)
	{
		_clearColor = color;
	}
}

#endif