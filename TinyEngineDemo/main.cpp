#if 0
#define TINY_ENGINE_IMPLEMENTATION
#define TINY_ENGINE_LAZY_LIBS
#include "TinyEngine.h"

#include <DirectXColors.h>

#pragma warning(disable: 4067 4244)
#include "vendor/OBJ_Loader.h";
#pragma warning(default: 4067 4244)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "vendor/stb_image.h"

#include <vector>
#include <map>
#include <string>

#define SAFE_DELETE(ptr) if (ptr) { delete ptr; ptr = nullptr; }

using namespace TinyEngine;
using namespace std::chrono;

using std::vector;
using std::map;
using std::string;

class DemoInput :
	public OLD_InputManager
{
protected:
	// Inherited via InputManager
	virtual void OnKeyPressed(Key key) override
	{
		cout << "Key Pressed " << (char)key << endl;
	}

	virtual void OnKeyReleased(Key key) override
	{
		cout << "Key Released " << (char)key << endl;
	}
};

class DemoGame
	: public OLD_TinyEngineGame
{
private:
	vector<OLD_Mesh*> _meshes;
	vector<OLD_Material*> _materials;
	map<string, OLD_Texture*> _textures;
	OLD_Texture* _nullTexture;

	OLD_PerspectiveCamera _camera;
	objl::Loader* _objLoader;

public:
	DemoGame() : OLD_TinyEngineGame(1280, 720, "Game"), _objLoader(nullptr)
	{
		_camera.SetPosition({ 0.0f, 1.0f, 0.0f });
		_camera.LookAt({ 0.0f, 0.0f, 0.0f });

		_nullTexture = new OLD_Texture();

		_inputManager = new DemoInput();
	}

	~DemoGame()
	{
		for (auto* mesh : _meshes)
		{
			delete mesh;
			mesh = nullptr;
		}

		for (auto* mat : _materials)
		{
			delete mat;
			mat = nullptr;
		}

		for (auto tex : _textures)
		{
			delete tex.second;
			tex.second = nullptr;
		}

		SAFE_DELETE(_objLoader);
		SAFE_DELETE(_nullTexture);
		SAFE_DELETE(_inputManager);
	}

private:
	OLD_Texture* LoadTexture(const char* path)
	{
		if (_textures[path])
		{
			return _textures[path];
		}

		int w, h, bpp;
		unsigned char* texData = stbi_load(path, &w, &h, &bpp, STBI_rgb_alpha);

		auto failure = stbi_failure_reason();
		if (failure)
		{
			PRINT_ERROR("STB Failed to load Image: " << failure);
		}

		OLD_Texture* tex = new OLD_Texture(texData, w, h);
		_textures[path] = tex;

		stbi_image_free(texData);

		return tex;
	}
		
	void OnInit() override
	{
		const auto& sphMesh = LoadMeshes("./assets/mesh/specMapSphere.obj");

		XMStoreFloat3(&_lights[0].direction, XMVector3Normalize(XMVectorSet(-1.0f, -0.5f, -1.0f, 0.0f)));
		_lights[0].color = { 1.0, 1.0, 1.0, 0.5f };

		_ambientLight = { 0.3f, 0.5f, 0.6f, 0.1f };
		SetClearColor(_ambientLight);

		XMStoreFloat3(&_lights[1].direction, XMVector3Normalize(XMVectorSet(1.0f, -0.25f, -1.0f, 0.0f)));
		_lights[1].color = { 0.2f, 0.2f, 0.4f, 0.3f };

		XMStoreFloat3(&_lights[2].direction, XMVector3Normalize(XMVectorSet(1.0f, -0.25f, 1.0f, 0.0f)));
		_lights[2].color = { 0.2f, 0.2f, 0.2f, 0.05f };
	}

	bool OnUpdate(float time, float delta) override
	{
		_camera.SetPosition({ sinf(time / 4) * 2.0f, 1.0f, cosf(time / 4) * 2.0f});
		_camera.SetAspectRatio(GetWidth() / GetHeight());

		for (auto* mesh : _meshes)
		{
			XMMATRIX world = XMMatrixIdentity();
			DrawMesh(mesh, &_camera, world);
		}

		return true;
	}
};

int main(int argc, char** argv)
{
	for (auto i = 0, l = argc - 1; i < l; i++)
	{
		if (string(argv[i]) == "/r") {
			cout << "Setting Working Directory to: " << argv[i + 1] << endl;
			SetCurrentDirectory(argv[i + 1]);
			break;
		}
	}

	DemoGame game;
	game.Run();
}
#endif


#pragma warning(disable: 4067 4244)
#include "vendor/OBJ_Loader.h";
#pragma warning(default: 4067 4244)

#include "Game.h"
#include "Key.h"
#include "Mesh.h"
#include <vector>
#include <DirectXMath.h>
#include <filesystem>
#include <string>
#include "ICamera.h"
#include "EngineEventType.h"

#ifdef TINY_ENGINE_EXPOSE_NATIVE
#error "TINY_ENGINE_EXPOSE_NATIVE has leaked to main."
#endif

using std::vector;
using TinyEngine::Mesh;
using TinyEngine::Material;
using TinyEngine::VertexStandard;

using namespace DirectX;

class Camera :
	public TinyEngine::ICamera, public TinyEngine::IObserver
{
private:
	XMFLOAT3 _position = {};
	float _aspectRatio = 1.0f;

public:
	void SetEyePosition(XMFLOAT3 position) { _position = position; }
	virtual DirectX::XMFLOAT3 GetEyePosition() { return _position; }

	virtual DirectX::XMMATRIX GetView()
	{
		XMFLOAT3 target = {};
		XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
		return XMMatrixLookAtLH(XMLoadFloat3(&_position), XMLoadFloat3(&target), XMLoadFloat3(&up));
	}

	virtual DirectX::XMMATRIX GetProjection()
	{
		return XMMatrixPerspectiveFovLH(XM_PIDIV2, _aspectRatio, 0.01f, 100.0f);
	}

	void SetAspectRatio(float aspectRatio)
	{
		_aspectRatio = aspectRatio;
	}

	virtual void OnNotify(const TinyEngine::Event& event)
	{
		switch (static_cast<TinyEngine::EngineEventType>(event.GetType()))
		{
		case TinyEngine::EngineEventType::WINDOW_RESIZE:
		{
			const auto& resizeEvent = static_cast<const TinyEngine::ResizeEvent&>(event);
			SetAspectRatio(static_cast<float>(resizeEvent.x) / static_cast<float>(resizeEvent.y));

			break;
		}
		default:
			break;
		}
	}
};

class Input :
	public TinyEngine::BaseInput
{
private:
	// Inherited via BaseInput
	virtual void OnKeyDown(TinyEngine::Key key)
	{

	}

	virtual void OnKeyUp(TinyEngine::Key key)
	{

	}
};

class DemoGame :
	public TinyEngine::Game
{
private:
	objl::Loader* _objLoader;
	vector<Mesh*> _meshes;
	vector<Material*> _materials;
	Camera _camera;

public:
	DemoGame(int width, int height, const char* title) : TinyEngine::Game(width, height, title), _objLoader(nullptr)
	{
		SetInputHandler(new Input());
		_camera.SetEyePosition({ 0.0f, 0.0f, -2.0f });
		_camera.SetAspectRatio(static_cast<float>(GetWidth()) / static_cast<float>(GetHeight()));

		AddObserver(_camera);
	}

	~DemoGame()
	{
		for (auto* mesh : _meshes)
		{
			delete mesh;
			mesh = nullptr;
		}

		for (auto* mat : _materials)
		{
			delete mat;
			mat = nullptr;
		}

		// Lazy initialized so might not exist here if it's never used.
		if (_objLoader)
		{
			delete _objLoader;
			_objLoader = nullptr;
		}
	}

	vector<Mesh*> LoadMeshes(const char* path)
	{
		if (!_objLoader)
		{
			_objLoader = new objl::Loader();
		}

		vector<Mesh*> meshes;

		auto* renderer = GetRenderer();

		if (_objLoader->LoadFile(path))
		{
			vector<VertexStandard> vertices;

			auto mesh = new Mesh(renderer);
			meshes.push_back(mesh);

			for (objl::Mesh& meshData : _objLoader->LoadedMeshes)
			{
				auto base = vertices.size();
				for (const auto& objlVertex : meshData.Vertices)
				{
					XMFLOAT3 pos(&objlVertex.Position.X);
					XMFLOAT2 tex(&objlVertex.TextureCoordinate.X);
					XMFLOAT3 norm(&objlVertex.Normal.X);

					vertices.emplace_back(pos, tex, norm);
				}

				const auto& objlMat = meshData.MeshMaterial;

				auto* mat = new Material();
				mat->ambient = XMFLOAT3(&objlMat.Ka.X);
				mat->transparency = objlMat.d;

				if (objlMat.map_Kd != "" && false)
				{
					std::filesystem::path texPath(path);
					texPath.remove_filename().append(objlMat.map_Kd);
					//mat->diffuseTexture = LoadTexture(texPath.string().c_str());
				}
				else
				{
					//mat->diffuseTexture = _nullTexture;
					mat->diffuse = XMFLOAT3(&objlMat.Kd.X);
				}

				if (objlMat.map_Ks != "" && false)
				{
					std::filesystem::path texPath(path);
					texPath.remove_filename().append(objlMat.map_Ks);
					//mat->specularTexture = LoadTexture(texPath.string().c_str());
				}
				else
				{
					//mat->specularTexture = _nullTexture;
					mat->specular = XMFLOAT3(&objlMat.Ks.X);
					mat->specularExponent = objlMat.Ns;
				}

				_materials.push_back(mat);

				mesh->AddIndexBuffer(meshData.Indices.data(), static_cast<unsigned int>(meshData.Indices.size()), static_cast<unsigned int>(base), mat);
			}

			mesh->SetVertices(vertices.data(), static_cast<unsigned int>(vertices.size()));
		}

		_meshes.insert(_meshes.end(), meshes.begin(), meshes.end());
		return meshes;
	}

	// Inherited via Game
	virtual void OnInit() override
	{
		LoadMeshes("./assets/mesh/specMapSphere.obj");

		auto renderer = GetRenderer();

		XMStoreFloat3(&renderer->lights[0].direction, XMVector3Normalize(XMVectorSet(-1.0f, -0.5f, -1.0f, 0.0f)));
		renderer->lights[0].color = { 1.0, 1.0, 1.0, 0.5f };

		renderer->ambientLight = { 0.3f, 0.5f, 0.6f, 0.1f };
		renderer->SetClearColor(renderer->ambientLight);

		XMStoreFloat3(&renderer->lights[1].direction, XMVector3Normalize(XMVectorSet(1.0f, -0.25f, -1.0f, 0.0f)));
		renderer->lights[1].color = { 0.2f, 0.2f, 0.4f, 0.3f };

		XMStoreFloat3(&renderer->lights[2].direction, XMVector3Normalize(XMVectorSet(1.0f, -0.25f, 1.0f, 0.0f)));
		renderer->lights[2].color = { 0.2f, 0.2f, 0.2f, 0.05f };
	}

	virtual void OnUpdate(float elapsed, float delta) override
	{
		_camera.SetEyePosition({ sinf(elapsed) * 2.0f, 1.0f, cosf(elapsed) * 2.0f });

		auto* renderer = GetRenderer();

		for (auto* mesh : _meshes)
		{
			renderer->DrawMesh(mesh, &_camera, XMMatrixIdentity());
		}
	}

};

int main(int argc, char** argv)
{
	for (auto i = 0, l = argc - 1; i < l; i++)
	{
		if (std::string(argv[i]) == "/r") {
			std::cout << "Setting Working Directory to: " << argv[i + 1] << std::endl;
			SetCurrentDirectory(argv[i + 1]);
			break;
		}
	}

	auto game = DemoGame(800, 600, "Game");
	game.Run();

	return 0;
}