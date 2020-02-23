#pragma once
#include <d3d11.h>
#include <dxgi.h>
#include "Subject.h"
#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "IRenderer.h"
#include "ConstantBuffer.h"
#include "ICamera.h"
#include <wrl\client.h>

namespace TinyEngine
{
	struct DirectionLight
	{
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT3 direction;
		float _pad;
	};

	struct PerObjectCBData
	{
	public:
		DirectionLight lights[3];
		DirectX::XMFLOAT4 ambientLight;
		DirectX::XMMATRIX world;
		DirectX::XMMATRIX worldInverseTranspose;
		DirectX::XMMATRIX view;
		DirectX::XMMATRIX projection;
		DirectX::XMFLOAT3 eyePosW;
		float _pad = 0.0f;
	};

	struct PerMaterialCBData
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

	class Renderer : 
		public IObserver, public IRenderer
	{
	public:
		DirectionLight lights[3];
		DirectX::XMFLOAT4 ambientLight;

	private:
		Microsoft::WRL::ComPtr<ID3D11Device> _device;
		Microsoft::WRL::ComPtr<ID3D11DeviceContext> _immediateContext;

		Microsoft::WRL::ComPtr<IDXGISwapChain> _swapChain;

		Microsoft::WRL::ComPtr<ID3D11RenderTargetView> _backBufferView;
		Microsoft::WRL::ComPtr<ID3D11DepthStencilView> _depthStencilView;

		Microsoft::WRL::ComPtr<ID3D11RasterizerState> _defaultRasterizerState;

		Microsoft::WRL::ComPtr<ID3D11SamplerState> _defaultSamplerState;

		Shader* _defaultShader;

		ConstantBuffer<PerObjectCBData>* _perObjectCB;
		ConstantBuffer<PerMaterialCBData>* _perMaterialCB;

		DirectX::XMFLOAT4 _clearColor;

	public:
		Renderer(int width, int height, Window& window);
		virtual ~Renderer();

		Renderer(const Renderer&) = delete;
		//void DrawMesh(Mesh* mesh, )

		void SetClearColor(DirectX::XMFLOAT4 color);

		void Clear();
		void SwapBuffers();

		void DrawMesh(Mesh* mesh, std::vector<Material*> materials, ICamera* camera, DirectX::XMMATRIX world);

		// Inherited via IObserver
		virtual void OnNotify(const Event& event) override;

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() const override
		{
			return _device;
		}

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetImmediateContext() const override
		{
			return _immediateContext;
		}
#endif

	private:
		void BindCurrentBackBufferView();
		void UpdateViewport(int x, int y, int width, int height);

		void OnResize(int width, int height);
	};
}
