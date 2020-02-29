#pragma once
#include <dxgi.h>
#include "Subject.h"
#include "Window.h"
#include "Mesh.h"
#include "Shader.h"
#include "Renderer.h"
#include "ConstantBuffer.h"
#include "ICamera.h"
#include <d3d11.h>
#include <wrl/client.h>

namespace TinyEngine
{
	// Represents a light source at infinity. Behaves like the sun.
	struct DirectionLight
	{
		DirectX::XMFLOAT4 color;
		DirectX::XMFLOAT3 direction;
		float _pad;
	};

	// Internal
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

	// Internal
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

	// 3D Renderer. Draws things on the screen.
	class Renderer : 
		public IObserver
	{
	public:
		// Lights which will be used when drawing the scene.
		DirectionLight lights[3];

		// Ambient light color.
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

		// Set the color that the window will be set to at the start of the frame.
		void SetClearColor(DirectX::XMFLOAT4 color);

		// Clear the screen.
		void Clear();

		// Swap the back and front buffer.
		void SwapBuffers();

		// Draw a mesh.
		//	Mesh* mesh: Mesh to draw
		//	std::vector<Material*> materials: Materials to draw the mesh with.
		//		Min 1. One material per parts in the mesh.
		//		If there are too few it will re use the last material in the array.
		//	ICamera* camera: Camera to draw the mesh with.
		//	DirectX::XMMATRIX world: World matrix of the mesh.
		void DrawMesh(Mesh* mesh, std::vector<Material*> materials, ICamera* camera, DirectX::XMMATRIX world);

		// Inherited via IObserver
		virtual void OnNotify(const Event& event) override;

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		Microsoft::WRL::ComPtr<ID3D11Device> GetDevice() const
		{
			return _device;
		}

		Microsoft::WRL::ComPtr<ID3D11DeviceContext> GetImmediateContext() const
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
