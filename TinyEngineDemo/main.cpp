#pragma warning(disable: 4067 4244)
#include "vendor/OBJ_Loader.h";
#pragma warning(default: 4067 4244)

#include <DirectXMath.h>
#include <filesystem>
#include <unordered_map>
#include <iostream>
#include <vector>
#include <string>
#include "Game.h"
#include "Key.h"
#include "Mesh.h"
#include "ICamera.h"
#include "EngineEventType.h"
#include "Texture.h"
#include "Shader.h"

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "vendor\stb_image.h"

#ifdef TINY_ENGINE_EXPOSE_NATIVE
#error "TINY_ENGINE_EXPOSE_NATIVE has leaked to main."
#endif

using std::vector;
using TinyEngine::Mesh;
using TinyEngine::Material;
using TinyEngine::VertexStandard;
using TinyEngine::Texture;
using TinyEngine::Shader;
using std::unordered_map;
using std::string;
using std::cout;
using std::endl;

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
		return XMMatrixPerspectiveFovLH(XM_PIDIV4, _aspectRatio, 0.01f, 1000.0f);
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
	unordered_map<TinyEngine::Key, bool> lastKeyboard;
	unordered_map<TinyEngine::Key, bool> thisKeyboard;
	unordered_map<TinyEngine::Key, bool> liveKeyboard;

public:
	void OnUpdate()
	{
		lastKeyboard = thisKeyboard;
		thisKeyboard = liveKeyboard;
	}

	bool GetKey(TinyEngine::Key key)
	{
		return thisKeyboard[key];
	}

	bool GetKeyDown(TinyEngine::Key key)
	{
		return !lastKeyboard[key] && thisKeyboard[key];
	}

	bool GetKeyUp(TinyEngine::Key key)
	{
		return lastKeyboard[key] && !thisKeyboard[key];
	}

private:
	// Inherited via BaseInput
	virtual void OnKeyDown(TinyEngine::Key key)
	{
		liveKeyboard[key] = true;
	}

	virtual void OnKeyUp(TinyEngine::Key key)
	{
		liveKeyboard[key] = false;
	}
};

class DemoGame :
	public TinyEngine::Game
{
private:
	objl::Loader* _objLoader;
	vector<Mesh*> _meshes;
	vector<Material*> _materials;
	unordered_map<string, Texture*> _textures;
	Texture* _nullTexture;

	Camera _camera;
	Shader* _skyboxShader;

public:
	DemoGame(int width, int height, const char* title) : TinyEngine::Game(width, height, title), _objLoader(nullptr)
	{
		SetInputHandler(new Input());
		_camera.SetEyePosition({ 0.0f, 0.0f, -2.0f });
		_camera.SetAspectRatio(static_cast<float>(GetWidth()) / static_cast<float>(GetHeight()));

		_nullTexture = new Texture(GetRenderer());

		AddObserver(_camera);
	}

	~DemoGame()
	{
		delete _nullTexture;
		_nullTexture = nullptr;

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

		for (auto textureKeyVal : _textures)
		{
			delete textureKeyVal.second;
			textureKeyVal.second = nullptr;
		}

		// Lazy initialized so might not exist here if it's never used.
		if (_objLoader)
		{
			delete _objLoader;
			_objLoader = nullptr;
		}

		delete _skyboxShader;
		_skyboxShader = nullptr;
	}
	
	Texture* LoadTexture(const char* path)
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
			cout << "STB Failed to load Image: " << failure << endl;
		}

		Texture* tex = new Texture(GetRenderer(), texData, w, h);
		_textures[path] = tex;

		stbi_image_free(texData);

		return tex;
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

				auto* mat = new Material {};
				mat->transparency = objlMat.d;

				if (objlMat.map_Ka != "")
				{
					std::filesystem::path texPath(path);
					texPath.remove_filename().append(objlMat.map_Ka);
					mat->ambientTexture = LoadTexture(texPath.string().c_str());
				}
				else
				{
					mat->ambientTexture = _nullTexture;
					mat->ambient = XMFLOAT3(&objlMat.Ka.X);
				}

				if (objlMat.map_Kd != "")
				{
					std::filesystem::path texPath(path);
					texPath.remove_filename().append(objlMat.map_Kd);
					mat->diffuseTexture = LoadTexture(texPath.string().c_str());
				}
				else
				{
					mat->diffuseTexture = _nullTexture;
					mat->diffuse = XMFLOAT3(&objlMat.Kd.X);
				}

				if (objlMat.map_Ks != "")
				{
					std::filesystem::path texPath(path);
					texPath.remove_filename().append(objlMat.map_Ks);
					mat->specularTexture = LoadTexture(texPath.string().c_str());
				}
				else
				{
					mat->specularTexture = _nullTexture;
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
		auto renderer = GetRenderer();

		D3D11_INPUT_ELEMENT_DESC inputDescs[3] = {
			{"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
			{"TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA},
			{"NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA}
		};

		_skyboxShader = new Shader(
			renderer,
			"./assets/shader/DefaultVertexShader.cso",
			"./assets/shader/SkyboxPixelShader.cso",
			inputDescs, 3
		);

		auto skyboxMesh = LoadMeshes("./assets/mesh/nightSkySphere.obj");
		skyboxMesh[0]->GetPartMaterial(0)->shader = _skyboxShader;

		auto venusMeshes = LoadMeshes("./assets/mesh/planets/venus.obj");

		XMStoreFloat3(&renderer->lights[0].direction, XMVector3Normalize(XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f)));
		renderer->lights[0].color = { 1.0, 1.0, 1.0, 0.5f };

		renderer->ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
		renderer->SetClearColor(renderer->ambientLight);

		renderer->lights[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
		renderer->lights[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	}

	virtual void OnUpdate(float elapsed, float delta) override
	{
		const auto input = GetInput();
		input->OnUpdate();

		float camYPos = _camera.GetEyePosition().y;

		float camYTarget = 0.0f;
		if (input->GetKey(TinyEngine::Key::W))
		{
			camYTarget += 1.0f;
		}

		if (input->GetKey(TinyEngine::Key::S))
		{
			camYTarget -= 1.0f;
		}

		camYPos = fminf(fmaxf(camYPos + camYTarget * delta, -1.0f), 3.0f);

		float eyeDistance = 20.0f;
		float time = elapsed * 0.125f;

		_camera.SetEyePosition({ cosf(time) * eyeDistance , camYPos, sinf(time) * eyeDistance });

		auto* renderer = GetRenderer();

		for (auto* mesh : _meshes)
		{
			renderer->DrawMesh(mesh, &_camera, XMMatrixIdentity());
		}
	}

	virtual Input* GetInput() const override
	{
		return static_cast<Input*>(TinyEngine::Game::GetInput());
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

	auto game = DemoGame(1600, 900, "Game");
	game.Run();

	return 0;
}
	