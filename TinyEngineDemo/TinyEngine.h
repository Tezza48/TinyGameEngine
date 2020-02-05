#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <d3d11.h>
#include <dxgi.h>
#include <iostream>
#include <comdef.h>
#include <DirectXMath.h>
#include <vector>

#define PRINT_ERROR(x) std::cerr << __FILE__ << ":" << __LINE__ << " " << x << std::endl

#define CHECK_HR(hr, message) if (FAILED(hr)) {_com_error err(hr); PRINT_ERROR(message << "\n\t" << err.ErrorMessage()); }

namespace TinyEngine
{
	class Mesh;
	class Shader;

	class TinyEngineGame
	{
		friend class Mesh;
		friend class Shader;
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

		ID3D11RasterizerState* _defaultRasterizerState;

		Shader* _defaultShader;

		int _width;
		int _height;
		const char* _title;

		DirectX::XMFLOAT4 _clearColor;

	public:
		TinyEngineGame(int width, int height, const char* Title);
		~TinyEngineGame();

		void Run();

	protected:
		virtual void OnInit() = 0;
		virtual bool OnUpdate() = 0;

		void DrawMesh(Mesh* mesh, Shader* shader = nullptr);

	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);

		void InitWin32();
		void InitD3D();

		void OnResize(int width, int height);
		void UpdateViewport();

		void SwapBuffers();
		void BindCurrentBackBufferView();
	};

	struct VertexStandard {
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
		DirectX::XMFLOAT3 normal;
	};

	class RenderResource
	{
		friend class TinyEngineGame;
	protected:
		static TinyEngineGame* _game;
	};

	class Mesh :
		public RenderResource
	{
		friend class TinyEngineGame;
	private:

		ID3D11Buffer* _vertexBuffer;
		unsigned int _numVertices;

		std::vector<ID3D11Buffer*> _indexBuffers;
		std::vector<unsigned int> _indexBufferSizes;

	public:
		Mesh();
		Mesh(const char* path);
		~Mesh();

		Mesh(const Mesh&) = delete;

		void SetVertices(VertexStandard* vertices, unsigned int numVertices);
		void AddIndexBuffer(unsigned int* indices, unsigned int numIndices);

	private:
		void ParseObjFace(const char* face, int* p, int* t, int* n);
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
#include <map>
#include <vector>

using std::cerr;
using std::endl;
using std::ifstream;
using std::string;
using std::cout;
using std::stringstream;
using std::map;
using std::vector;
using std::getline;

using namespace DirectX;

namespace TinyEngine
{
	TinyEngineGame* RenderResource::_game;

	TinyEngine::Shader::Shader(const char* vertexShaderBytecode, size_t vertexShaderSize, const char* pixelShaderBytecode, size_t pixelShaderSize, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount)
	{
		auto device = _game->_device;
		device->CreateVertexShader(vertexShaderBytecode, vertexShaderSize, nullptr, &_vertexShader);
		device->CreatePixelShader(pixelShaderBytecode, pixelShaderSize, nullptr, &_pixelShader);

		device->CreateInputLayout(inputDescs, inputDescCount, vertexShaderBytecode, vertexShaderSize, &_inputLayout);
	}

	inline Shader::~Shader()
	{
		SAFE_RELEASE(_vertexShader);
		SAFE_RELEASE(_pixelShader);
		SAFE_RELEASE(_inputLayout);
	}

	inline Mesh::Mesh(): _numVertices(0), _vertexBuffer(nullptr)
	{

	}

	// This should absolutely not be the responsibility of the engine. This should go in a TinyEngineAssets extension library.
	inline Mesh::Mesh(const char* path): _numVertices(0), _vertexBuffer(nullptr)
	{
		struct ObjData {
			vector<XMFLOAT3> positions;
			vector<XMFLOAT2> texcoords;
			vector<XMFLOAT3> normals;
			vector<vector<VertexStandard>> faces;
		};

		map<string, ObjData*> objMap;

		// Load mesh at this location.
		ifstream file(path);

		if (file.is_open())
		{
			string line;
			ObjData* currentObj = nullptr;
			while (std::getline(file, line))
			{
				stringstream lineStream(line);
				string instruction;

				lineStream >> instruction;
				if (instruction == "#")
				{
					continue;
				}
				else if (instruction == "o")
				{
					string name;
					lineStream >> name;
					objMap[name] = new ObjData();
					currentObj = objMap[name];
				}
				else if (instruction == "v")
				{
					float x, y, z;
					lineStream >> x >> y >> z;
					currentObj->positions.emplace_back(x, y, z);
				}
				else if (instruction == "vt")
				{
					float u, v, w;
					lineStream >> u >> v >> w;

					if (!w) PRINT_ERROR("Mesh with 3 component texcoords is not supported.");

					currentObj->texcoords.emplace_back(u, v);
				}
				else if (instruction == "vn")
				{
					float i, j, k;
					lineStream >> i >> j >> k;
					currentObj->normals.emplace_back(i, j, k);
				}
				else if (instruction == "f")
				{
					int positions[4];
					int texcoords[4];
					int normals[4];

					// Populate pos, tex, norm with data from face slash separated values.
					string faceString;
					int index = 0;
					while (index < 4)
					{
						lineStream >> faceString;
						if (faceString == "") break;
						ParseObjFace(faceString.c_str(), &positions[index], &texcoords[index], &normals[index]);
						index++;
					}

					std::vector<VertexStandard> face;
					face.push_back({currentObj->positions[positions[0] - 1], currentObj->texcoords[texcoords[0] - 1], currentObj->normals[normals[0] - 1]});
					face.push_back({currentObj->positions[positions[1] - 1], currentObj->texcoords[texcoords[1] - 1], currentObj->normals[normals[1] - 1]});
					face.push_back({currentObj->positions[positions[2] - 1], currentObj->texcoords[texcoords[2] - 1], currentObj->normals[normals[2] - 1]});

					if (index > 3)
					{
						face.push_back({currentObj->positions[positions[3] - 1], currentObj->texcoords[texcoords[3] - 1], currentObj->normals[normals[3] - 1]});
					}

					currentObj->faces.push_back(face);
				}
			}
		}
		else
		{
			PRINT_ERROR("Could not open file: " << path);
		}

		if (objMap.size())
		{
			std::vector<VertexStandard> vertices;
			unsigned int lastVertex = 0;

			// Build Vbuffer and Ibuffers
			for (auto it = objMap.begin(); it != objMap.end(); it++)
			{
				auto obj = it->second;

				vector<unsigned int> indices;

				for (const auto& face : obj->faces)
				{
					vertices.insert(vertices.begin(), face.begin(), face.end());

					indices.push_back(lastVertex);
					indices.push_back(lastVertex + 1);
					indices.push_back(lastVertex + 2);

					unsigned int increment = 3;

					if (face.size() == 4)
					{
						indices.push_back(lastVertex + 0);
						indices.push_back(lastVertex + 2);
						indices.push_back(lastVertex + 3);

						increment++;
					}

					lastVertex += increment;
				}

				AddIndexBuffer(indices.data(), static_cast<unsigned int>(indices.size()));

				delete objMap[it->first];
			}

			SetVertices(vertices.data(), static_cast<unsigned int>(vertices.size()));
		}
	}

	inline Mesh::~Mesh()
	{
		SAFE_RELEASE(_vertexBuffer);

		for (size_t i = 0, l = _indexBuffers.size(); i < l; i++) {
			SAFE_RELEASE(_indexBuffers[i]);
		}
	}

	inline void Mesh::SetVertices(VertexStandard* vertices, unsigned int numVertices)
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

	inline void Mesh::AddIndexBuffer(unsigned int* indices, unsigned int numIndices)
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

		_indexBuffers.push_back(indexBuffer);
		_indexBufferSizes.push_back(numIndices);
	}

	inline void Mesh::ParseObjFace(const char* face, int* p, int* t, int* n)
	{
		stringstream faceStream(face);
		string elem;
		if (getline(faceStream, elem, '/')) *p = std::stoi(elem);
		if (getline(faceStream, elem, '/')) *t = std::stoi(elem);
		faceStream >> *n;
	}

	inline void TinyEngineGame::DrawMesh(Mesh* mesh, Shader * shader)
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

		if (mesh->_indexBuffers.size())
		{
			for (int i = 0; i < mesh->_indexBuffers.size(); i++)
			{
				context->IASetIndexBuffer(mesh->_indexBuffers[i], DXGI_FORMAT_R32_UINT, 0);

				// could maybe use this if the index buffer only draws from vert n. optimize the space slightly.
				context->DrawIndexed(mesh->_indexBufferSizes[i], 0, 0);
			}
		}
		else
		{
			context->Draw(mesh->_numVertices, 0);
		}
	}

	inline LRESULT CALLBACK TinyEngineGame::WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
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

	inline TinyEngineGame::TinyEngineGame(int width, int height, const char* title)
		: _width(width), _height(height), _title(title)
	{
		InitWin32();

		InitD3D();

		RenderResource::_game = this;
	}

	inline TinyEngineGame::~TinyEngineGame()
	{
		SAFE_DELETE(_defaultShader);
		
		SAFE_RELEASE(_defaultRasterizerState);
		SAFE_RELEASE(_depthStencilView);
		SAFE_RELEASE(_backBufferView);
		SAFE_RELEASE(_swapChain);
		SAFE_RELEASE(_immediateContext);
		SAFE_RELEASE(_device);
	}

	inline void TinyEngineGame::InitWin32()
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

	inline void TinyEngineGame::InitD3D()
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
		rd.CullMode = D3D11_CULL_NONE;
		rd.FrontCounterClockwise = false;
		rd.DepthBias = 0;
		rd.DepthBiasClamp = 1.0f;
		rd.SlopeScaledDepthBias = 0;
		rd.DepthClipEnable = true;
		rd.ScissorEnable = false;
		rd.MultisampleEnable = false;
		rd.AntialiasedLineEnable = true;

		hr = _device->CreateRasterizerState(&rd, &_defaultRasterizerState);
		// create sound stuff.

		UpdateViewport();
	}

	inline void TinyEngineGame::OnResize(int width, int height)
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

		UpdateViewport();
	}

	inline void TinyEngineGame::SwapBuffers()
	{
		// Swap buffers
		_swapChain->Present(0, 0);

		_backBufferView->Release();
		_backBufferView = nullptr;

		BindCurrentBackBufferView();

		_immediateContext->OMSetRenderTargets(1, &_backBufferView, _depthStencilView);
	}

	inline void TinyEngineGame::BindCurrentBackBufferView()
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

	inline void TinyEngineGame::UpdateViewport()
	{
		D3D11_VIEWPORT viewport = {};
		viewport.TopLeftX = 0;
		viewport.TopLeftY = 0;
		viewport.Width = static_cast<float>(_width);
		viewport.Height = static_cast<float>(_height);
		viewport.MinDepth = 0;
		viewport.MaxDepth = 0;

		_immediateContext->RSSetViewports(1, &viewport);
	}

	inline void TinyEngineGame::Run()
	{
		isRunning = true;

		const auto vSource = ""
			"struct VS_IN { float3 position: POSITION; float2 texcoord: TEXCOORD; float3 normal: NORMAL; };\n"
			"struct VS_OUT { float4 position: SV_POSITION; float3 normal: NORMAL; };\n"
			"VS_OUT main(VS_IN i)\n"
			"{"
			"	VS_OUT o;\n"
			"	o.normal = i.normal;\n"
			"	o.position = float4(i.position, 1.0);\n"
			"	return o;\n"
			"}\n";

		const auto pSource = ""
			"struct VS_OUT { float4 position: SV_POSITION; float3 normal: NORMAL; };\n"
			"float4 main(VS_OUT i): SV_TARGET\n"
			"{\n"
			"	return float4(i.normal.xyz, 1.0);\n"
			"}\n";

		D3D11_INPUT_ELEMENT_DESC inputDescs[3] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA}
		};

		ID3D10Blob* errors[2];
		ID3D10Blob* vsBlob = nullptr;
		ID3D10Blob* psBlob = nullptr;

		unsigned int compileFlags = 0;

#if DEBUG || _DEBUG
		compileFlags |= D3DCOMPILE_DEBUG;
#endif

		auto include = D3D_COMPILE_STANDARD_FILE_INCLUDE;
		HRESULT hr = D3DCompile(vSource, strlen(vSource), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "vs_5_0", compileFlags, NULL, &vsBlob, &errors[0]);
		CHECK_HR(hr, "VS Could not be compiled.")
		hr = D3DCompile(pSource, strlen(pSource), nullptr, nullptr, D3D_COMPILE_STANDARD_FILE_INCLUDE, "main", "ps_5_0", compileFlags, NULL, &psBlob, &errors[1]);
		CHECK_HR(hr, "PS Could not be compiled.")

		for (int i = 0; i < 2; i++)
		{
			auto error = errors[i];
			if (error)
			{
				PRINT_ERROR("Shader compilation failed");
				PRINT_ERROR(static_cast<LPCSTR>(error->GetBufferPointer()));

				error->Release();
				error = nullptr;
			}
		}

		_defaultShader = new Shader((char*)vsBlob->GetBufferPointer(), vsBlob->GetBufferSize(), (char*)psBlob->GetBufferPointer(), psBlob->GetBufferSize(), inputDescs, 3);

		SAFE_RELEASE(vsBlob);
		SAFE_RELEASE(psBlob);

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
}

#endif