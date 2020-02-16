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
	class OLD_Mesh;
	class OLD_Shader;
	class OLD_Camera;
	class OLD_TinyEngineGame;
	class OLD_Texture;

	template<typename T>
	class OLD_ConstantBuffer;

	class OLD_RenderResource
	{
		friend class OLD_TinyEngineGame;
	protected:
		static OLD_TinyEngineGame* _game;
	};

	struct OLD_Material {
	public:
		DirectX::XMFLOAT3 specular;
		float specularExponent;
		DirectX::XMFLOAT3 ambient;
		float transparency;
		DirectX::XMFLOAT3 diffuse;
		OLD_Texture* diffuseTexture;
		OLD_Texture* specularTexture;

		OLD_Material();
		OLD_Material(DirectX::XMFLOAT3 ambient, DirectX::XMFLOAT3 diffuse, DirectX::XMFLOAT3 specular, float specularExponent, float transparency, OLD_Texture* diffuseTexture = nullptr);
	};

	struct OLD_DirectionLight
	{
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT3 direction;
		float _pad;
	};

	struct OLD_PerObjectCb
	{
	public:
		OLD_DirectionLight lights[3];
		DirectX::XMFLOAT4 ambientLight;
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInverseTranspose;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
		DirectX::XMFLOAT3 eyePosW;
		float _pad = 0.0f;
	};

	struct OLD_PerMaterialCB
	{
		struct
		{
			DirectX::XMFLOAT3 specular;
			float specularExponent;
			DirectX::XMFLOAT3 ambient;
			float transparency;
			DirectX::XMFLOAT3 diffuse;
		} mat;
		float _pad = 0.0f;
	};

	class OLD_InputManager
	{
	public:
		enum class Key
		{
			N0 = 0x30,
			N1,
			N2,
			N3,
			N4,
			N5,
			N6,
			N7,
			N8,
			N9,
			A = 0x41,
			B,
			C,
			D,
			E,
			F,
			G,
			H,
			I,
			J,
			K,
			L,
			M,
			N,
			O,
			P,
			Q,
			R,
			S,
			T,
			U,
			V,
			W,
			X,
			Y,
			Z,
		};
	public:
		virtual void OnKeyPressed(Key key) = 0;
		virtual void OnKeyReleased(Key key) = 0;
	};

	class OLD_TinyEngineGame
	{
	public:
	protected:
		OLD_DirectionLight _lights[3];
		DirectX::XMFLOAT4 _ambientLight;

		OLD_InputManager* _inputManager = nullptr;

	private:
		HWND _window = 0;
		bool isRunning;

		ID3D11Device* _device;
		ID3D11DeviceContext* _immediateContext;
		IDXGISwapChain* _swapChain;

		ID3D11RenderTargetView* _backBufferView;
		ID3D11DepthStencilView* _depthStencilView;

		ID3D11RasterizerState* _defaultRasterizerState;

		ID3D11SamplerState* _defaultSamplerState;

		OLD_ConstantBuffer<OLD_PerObjectCb>* _perObjectConstantBuffer;
		OLD_ConstantBuffer<OLD_PerMaterialCB>* _materialConstantBuffer;

		OLD_Shader* _defaultShader;

		int _width;
		int _height;
		const char* _title;

		DirectX::XMFLOAT4 _clearColor;

		std::chrono::high_resolution_clock::time_point _startTime;
		std::chrono::high_resolution_clock::time_point _lastTime;

	public:
		OLD_TinyEngineGame(int width, int height, const char* Title);
		~OLD_TinyEngineGame();

		ID3D11Device* GetDevice();
		ID3D11DeviceContext* GetImmediateContext();

		void Run();

	protected:
		float GetWidth() const;
		float GetHeight() const;

		void SetClearColor(DirectX::XMFLOAT4 color);

		virtual void OnInit() = 0;
		virtual bool OnUpdate(float time, float delta) = 0;

		void DrawMesh(OLD_Mesh* mesh, OLD_Camera* camera, const DirectX::XMMATRIX& world, OLD_Shader* shader = nullptr);
		// TODO WT: void DrawTexture(Texture* texture, DirectX::XMFLOAT4 rect, Shader* shader = nullptr);
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
	class OLD_ConstantBuffer :
		public OLD_RenderResource
	{
		friend class OLD_TinyEngineGame;
	private:
		ID3D11Buffer* _buffer;

	public:
		OLD_ConstantBuffer();
		~OLD_ConstantBuffer();

		void Upload(const T& data);
	};

	template<typename T>
	inline OLD_ConstantBuffer<T>::OLD_ConstantBuffer()
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

		HRESULT hr = _game->GetDevice()->CreateBuffer(&desc, &initialData, &_buffer);
		CHECK_HR(hr, "Failed to create Constant Buffer");
	}

	template<typename T>
	inline OLD_ConstantBuffer<T>::~OLD_ConstantBuffer()
	{
		if (_buffer) {
			_buffer->Release();
			_buffer = nullptr;
		}
	}

	template<typename T>
	inline void OLD_ConstantBuffer<T>::Upload(const T& data)
	{
		D3D11_MAPPED_SUBRESOURCE mappedData;

		auto context = _game->GetImmediateContext();

		context->Map(_buffer, 0, D3D11_MAP_WRITE_DISCARD, NULL, &mappedData);

		memcpy(mappedData.pData, &data, sizeof(T));

		context->Unmap(_buffer, 0);
	}

	struct OLD_VertexStandard {
	public:
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT2 texcoord;
		DirectX::XMFLOAT3 normal;

	public:
		OLD_VertexStandard(DirectX::XMFLOAT3 position, DirectX::XMFLOAT2 texcoord, DirectX::XMFLOAT3 normal);
	};

	class OLD_Camera
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

	class OLD_PerspectiveCamera :
		public OLD_Camera
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

	class OLD_Mesh :
		public OLD_RenderResource
	{
		friend class OLD_TinyEngineGame;
	private:
		struct MeshPart {
			ID3D11Buffer* indexBuffer;
			unsigned int size;
			unsigned int baseVertex;
			OLD_Material* mat;
		};

		ID3D11Buffer* _vertexBuffer;
		unsigned int _numVertices;

		std::vector<MeshPart> _parts;

	public:
		OLD_Mesh();
		~OLD_Mesh();

		OLD_Mesh(const OLD_Mesh&) = delete;

		void SetVertices(OLD_VertexStandard* vertices, unsigned int numVertices);
		void AddIndexBuffer(unsigned int* indices, unsigned int numIndices, unsigned int baseVertex, OLD_Material* mat = nullptr);
	};

	class OLD_Shader :
		public OLD_RenderResource
	{
		friend class OLD_TinyEngineGame;
	private:
		ID3D11VertexShader* _vertexShader;
		ID3D11PixelShader* _pixelShader;
		ID3D11InputLayout* _inputLayout;

	public:
		OLD_Shader(const char* vertexShaderBytecode, size_t vertexShaderSize, const char* pixelShaderBytecode, size_t pixelShaderSize, D3D11_INPUT_ELEMENT_DESC* inputDesc, unsigned int inputDescCount);
		OLD_Shader(const char* vertexPath, const char* fragmentPath, D3D11_INPUT_ELEMENT_DESC* inputDesc, unsigned int inputDescCount);
		~OLD_Shader();

		OLD_Shader(const OLD_Shader&) = delete;
	};

	class OLD_Texture :
		OLD_RenderResource
	{
		friend class OLD_TinyEngineGame;
	private:
		ID3D11ShaderResourceView* _textureView;

	public:
		OLD_Texture();
		OLD_Texture(const unsigned char* data, int width, int height);

		OLD_Texture(const OLD_Texture&) = delete;
		~OLD_Texture();
	};

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
	OLD_TinyEngineGame* OLD_RenderResource::_game;

	OLD_Texture::OLD_Texture(): _textureView(nullptr)
	{
		
	}

	OLD_Texture::OLD_Texture(const unsigned char* data, int width, int height)
	{
		D3D11_TEXTURE2D_DESC desc = {};
		desc.Width = width;
		desc.Height = height;
		desc.MipLevels = 0;
		desc.ArraySize = 1;
		desc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		desc.SampleDesc.Count = 1;
		desc.SampleDesc.Quality = 0;
		desc.Usage = D3D11_USAGE_DEFAULT;
		desc.BindFlags = D3D11_BIND_SHADER_RESOURCE | D3D11_BIND_RENDER_TARGET;
		desc.CPUAccessFlags = DXGI_CPU_ACCESS_NONE;
		desc.MiscFlags = D3D11_RESOURCE_MISC_GENERATE_MIPS;

		auto device = _game->GetDevice();
		auto context = _game->GetImmediateContext();

		ID3D11Texture2D* texture;

		HRESULT hr = device->CreateTexture2D(&desc, nullptr, &texture);
		CHECK_HR(hr, "Failed to Create Texture2D");

		unsigned int rowPitch = width * 4 * sizeof(unsigned char);

		context->UpdateSubresource(texture, 0, nullptr, data, rowPitch, 0);
		
		D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc;
		srvDesc.Format = desc.Format;
		srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Texture2D.MipLevels = -1;

		hr = device->CreateShaderResourceView(texture, &srvDesc, &_textureView);

		context->GenerateMips(_textureView);

		SAFE_RELEASE(texture);
	}

	OLD_Texture::~OLD_Texture()
	{
		SAFE_RELEASE(_textureView);
	}


	//	-	-	-	-	-	-	-	-	-	-	-	-	Material	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	OLD_Material::OLD_Material()
	{
		ambient = {};
		diffuse = {};
		specular = {};
		specularExponent = {};
		transparency = {};
		diffuseTexture = nullptr;
	}

	OLD_Material::OLD_Material(DirectX::XMFLOAT3 ambient, DirectX::XMFLOAT3 diffuse, DirectX::XMFLOAT3 specular, float specularExponent, float transparency, OLD_Texture* diffuseTexture)
	{
		this->ambient = ambient;
		this->diffuse = diffuse;
		this->diffuseTexture = diffuseTexture;
		this->specular = specular;
		this->specularExponent = specularExponent;
		this->transparency = transparency;
	}

	//	-	-	-	-	-	-	-	-	-	-	-	-	VertexStandard	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	
	TinyEngine::OLD_VertexStandard::OLD_VertexStandard(XMFLOAT3 position, XMFLOAT2 texcoord, XMFLOAT3 normal)
	{
		this->position = position;
		this->texcoord = texcoord;
		this->normal = normal;
	}

	//	-	-	-	-	-	-	-	-	-	-	-	-	Camera	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	XMMATRIX OLD_Camera::GetView()
	{
		if (_dirtyView)
		{
			RebuildView();
			_dirtyView = false;
		}

		return _viewMatrix;
	}

	XMMATRIX OLD_Camera::GetProjection()
	{
		if (_dirtyProj)
		{
			RebuildProjection();
			_dirtyProj = false;
		}

		return _projectionMatrix;
	}

	void TinyEngine::OLD_Camera::SetPosition(XMFLOAT3 position)
	{
		_position = position;
		_dirtyView = true;
	}

	DirectX::XMFLOAT3 OLD_Camera::GetPosition()
	{
		return _position;
	}

	//	-	-	-	-	-	-	-	-	-	-	-	PerspectiveCamera	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-
	
	void OLD_PerspectiveCamera::SetFov(float fov)
	{
		_fov = fov;
		_dirtyProj = true;
	}

	void OLD_PerspectiveCamera::SetAspectRatio(float aspectRatio)
	{
		_aspectRatio = aspectRatio;
		_dirtyProj = true;
	}

	void OLD_PerspectiveCamera::LookAt(XMFLOAT3 target)
	{
		_target = target;
		_dirtyView = true;
	}

	void OLD_PerspectiveCamera::RebuildView()
	{
		XMFLOAT3 up(0.0f, 1.0f, 0.0f);
		_viewMatrix = XMMatrixLookAtLH(XMLoadFloat3(&_position), XMLoadFloat3(&_target), XMVectorSet(0.0f, 1.0f, 0.0f, 1.0f));
	}

	void OLD_PerspectiveCamera::RebuildProjection()
	{
		_projectionMatrix = XMMatrixPerspectiveFovLH(_fov, _aspectRatio, _near, _far);
	}

	//	-	-	-	-	-	-	-	-	-	-	-	Shader	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	OLD_Shader::OLD_Shader(const char* vertexShaderBytecode, size_t vertexShaderSize, const char* pixelShaderBytecode, size_t pixelShaderSize, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount)
	{
		auto device = _game->GetDevice();

		HRESULT hr;
		hr = device->CreateVertexShader(vertexShaderBytecode, vertexShaderSize, nullptr, &_vertexShader);
		CHECK_HR(hr, "Failed to create Vertex Shader.");
		hr = device->CreatePixelShader(pixelShaderBytecode, pixelShaderSize, nullptr, &_pixelShader);
		CHECK_HR(hr, "Failed to create Pixel Shader.");
		
		hr = device->CreateInputLayout(inputDescs, inputDescCount, vertexShaderBytecode, vertexShaderSize, &_inputLayout);
		CHECK_HR(hr, "Failed to create Input Layout.");
	}

	OLD_Shader::OLD_Shader(const char* vertexPath, const char* pixelPath, D3D11_INPUT_ELEMENT_DESC* inputDescs, unsigned int inputDescCount)
	{
		auto device = _game->GetDevice();

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

	OLD_Shader::~OLD_Shader()
	{
		SAFE_RELEASE(_vertexShader);
		SAFE_RELEASE(_pixelShader);
		SAFE_RELEASE(_inputLayout);
	}

	//	-	-	-	-	-	-	-	-	-	-	-	Mesh	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	OLD_Mesh::OLD_Mesh(): _numVertices(0), _vertexBuffer(nullptr)
	{

	}

	OLD_Mesh::~OLD_Mesh()
	{
		SAFE_RELEASE(_vertexBuffer);

		for (size_t i = 0, l = _parts.size(); i < l; i++) {
			SAFE_RELEASE(_parts[i].indexBuffer);
		}
	}

	void OLD_Mesh::SetVertices(OLD_VertexStandard* vertices, unsigned int numVertices)
	{
		_numVertices = numVertices;

		D3D11_BUFFER_DESC bd;
		bd.ByteWidth = numVertices * sizeof(OLD_VertexStandard);
		bd.Usage = D3D11_USAGE_IMMUTABLE;
		bd.BindFlags = D3D11_BIND_VERTEX_BUFFER;
		bd.CPUAccessFlags = NULL;
		bd.MiscFlags = NULL;
		bd.StructureByteStride = 0;

		D3D11_SUBRESOURCE_DATA pData = {};
		pData.pSysMem = vertices;

		HRESULT hr;
		hr = _game->GetDevice()->CreateBuffer(&bd, &pData, &_vertexBuffer);
		CHECK_HR(hr, "Failed to create Vertex Buffer.");
	}

	void OLD_Mesh::AddIndexBuffer(unsigned int* indices, unsigned int numIndices, unsigned int baseVertex, OLD_Material* mat)
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
		hr = _game->GetDevice()->CreateBuffer(&bd, &pData, &indexBuffer);
		CHECK_HR(hr, "Failed to create Vertex Buffer.");

		_parts.push_back({ indexBuffer, numIndices, baseVertex, mat });
	}

	//	-	-	-	-	-	-	-	-	-	-	TinyEngineGame	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-	-

	void OLD_TinyEngineGame::DrawMesh(OLD_Mesh* mesh, OLD_Camera* camera, const DirectX::XMMATRIX& world, OLD_Shader * shader)
	{
		if (shader == nullptr)
		{
			shader = _defaultShader;
		}

		auto context = _immediateContext;

		const unsigned int stride = sizeof(OLD_VertexStandard);
		const unsigned int offset = 0;

		context->IASetVertexBuffers(0, 1, &mesh->_vertexBuffer, &stride, &offset);
		context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
		context->IASetInputLayout(shader->_inputLayout);

		context->RSSetState(_defaultRasterizerState);

		context->VSSetShader(shader->_vertexShader, nullptr, 0);
		context->PSSetShader(shader->_pixelShader, nullptr, 0);

		context->PSSetSamplers(0, 1, &_defaultSamplerState);

		OLD_PerObjectCb objCb;
		memcpy(objCb.lights, _lights, sizeof(_lights));
		objCb.ambientLight = _ambientLight;
		objCb.world = XMMatrixTranspose(world);

		XMVECTOR det = XMMatrixDeterminant(world);
		objCb.worldInverseTranspose = XMMatrixInverse(&det, world);

		objCb.view = XMMatrixTranspose(camera->GetView());
		objCb.projection = XMMatrixTranspose(camera->GetProjection());
		objCb.eyePosW = camera->GetPosition();


		_perObjectConstantBuffer->Upload(objCb);

		context->VSSetConstantBuffers(0, 1, &_perObjectConstantBuffer->_buffer);
		context->PSSetConstantBuffers(0, 1, &_perObjectConstantBuffer->_buffer);

		if (mesh->_parts.size())
		{
			for (int i = 0; i < mesh->_parts.size(); i++)
			{
				auto part = mesh->_parts[i];

				OLD_PerMaterialCB matCb;
				matCb.mat.diffuse = part.mat->diffuse;
				matCb.mat.ambient = part.mat->ambient;
				matCb.mat.specular = part.mat->specular;
				matCb.mat.specularExponent = part.mat->specularExponent;
				matCb.mat.transparency = part.mat->transparency;

				_materialConstantBuffer->Upload(matCb);

				ID3D11ShaderResourceView* textureViews[2] = {
					part.mat->diffuseTexture->_textureView,
					part.mat->specularTexture->_textureView
				};

				context->PSSetShaderResources(0, 2, textureViews);

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

	LRESULT CALLBACK OLD_TinyEngineGame::WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
	{
		static OLD_TinyEngineGame* engine;

		switch (uMsg)
		{
		case WM_CREATE:
		{
			engine = reinterpret_cast<OLD_TinyEngineGame*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);

			return 0;
		}

		case WM_CLOSE:
		{
			DestroyWindow(hwnd);
			engine->isRunning = false;

			return 0;
		}

		case WM_DESTROY:
		{
			PostQuitMessage(0);

			return 0;
		}

		case WM_SIZE:
		{
			const auto width = LOWORD(lparam);
			const auto height = HIWORD(lparam);
			engine->OnResize(width, height);
			return 0;
		}

		case WM_KEYDOWN:
		{
			if (engine->_inputManager && !((lparam >> 30) & 1)) // bit 30 indicates that the key was already pressed
			{
				engine->_inputManager->OnKeyPressed(static_cast<OLD_InputManager::Key>(wparam));

				return 0;
			}

			break;
		}
			
		case WM_KEYUP:
		{
			if (engine->_inputManager)
			{
				engine->_inputManager->OnKeyReleased(static_cast<OLD_InputManager::Key>(wparam));
				return 0;
			}

			break;
		}
		}

		return DefWindowProc(hwnd, uMsg, wparam, lparam);;
	}

	OLD_TinyEngineGame::OLD_TinyEngineGame(int width, int height, const char* title)
		: _width(width), _height(height), _title(title)
	{
		InitWin32();

		InitD3D();

		OLD_RenderResource::_game = this;
		_lastTime = _startTime = high_resolution_clock::now();
	}

	OLD_TinyEngineGame::~OLD_TinyEngineGame()
	{
		SAFE_DELETE(_defaultShader);
		SAFE_DELETE(_materialConstantBuffer);
		SAFE_DELETE(_perObjectConstantBuffer);
		
		SAFE_RELEASE(_defaultSamplerState);
		SAFE_RELEASE(_defaultRasterizerState);
		SAFE_RELEASE(_depthStencilView);
		SAFE_RELEASE(_backBufferView);
		SAFE_RELEASE(_swapChain);
		SAFE_RELEASE(_immediateContext);
		SAFE_RELEASE(_device);
	}

	void OLD_TinyEngineGame::InitWin32()
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

	void OLD_TinyEngineGame::InitD3D()
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

		UpdateViewport();
	}

	void OLD_TinyEngineGame::OnResize(int width, int height)
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

	void OLD_TinyEngineGame::SwapBuffers()
	{
		// Swap buffers
		_swapChain->Present(0, 0);

		_backBufferView->Release();
		_backBufferView = nullptr;

		BindCurrentBackBufferView();

		_immediateContext->OMSetRenderTargets(1, &_backBufferView, _depthStencilView);
	}

	void OLD_TinyEngineGame::BindCurrentBackBufferView()
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

	void OLD_TinyEngineGame::UpdateViewport()
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

	void OLD_TinyEngineGame::Run()
	{
		isRunning = true;

		D3D11_INPUT_ELEMENT_DESC inputDescs[3] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA}
		};

		_defaultShader = new OLD_Shader("./assets/shader/defaultVertexShader.cso", "./assets/shader/defaultPixelShader.cso", inputDescs, 3);

		_perObjectConstantBuffer = new OLD_ConstantBuffer<OLD_PerObjectCb>();
		_materialConstantBuffer = new OLD_ConstantBuffer<OLD_PerMaterialCB>();

		OnInit();

		MSG message = {};

		while (message.message != WM_QUIT)
		{
			while (PeekMessage(&message, _window, 0, 0, PM_REMOVE))
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

	inline ID3D11Device* OLD_TinyEngineGame::GetDevice()
	{
		return _device;
	}

	inline ID3D11DeviceContext* OLD_TinyEngineGame::GetImmediateContext()
	{
		return _immediateContext;
	}

	float OLD_TinyEngineGame::GetWidth() const
	{
		return static_cast<float>(_width);
	}

	float OLD_TinyEngineGame::GetHeight() const
	{
		return static_cast<float>(_height);
	}

	void OLD_TinyEngineGame::SetClearColor(XMFLOAT4 color)
	{
		_clearColor = color;
	}
}

#endif