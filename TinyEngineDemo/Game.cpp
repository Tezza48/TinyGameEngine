#include "Game.h"

#pragma warning(disable: 4067 4244)
#include "vendor/OBJ_Loader.h";
#pragma warning(default: 4067 4244)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "vendor\stb_image.h"

#include <iostream>
#include <DirectXMath.h>
#include <filesystem>
#include "CameraActor.h"

using namespace DirectX;
using namespace TinyEngine;

using std::cout;
using std::endl;
using std::vector;

Game::Game(int width, int height, const char* title) : TinyEngine::TinyEngineGame(width, height, title), _activeCamera(nullptr)
{
	SetInputHandler(&_inputHandler);

	_nullTexture = new Texture(GetRenderer());

	_rootActor = new Actor(this);
}

Game::~Game()
{
	delete _rootActor;
	_rootActor = nullptr;

	delete _nullTexture;
	_nullTexture = nullptr;

	for (auto mesh : _meshes)
	{
		delete mesh.mesh;
		
		for (auto mat : mesh.materials)
		{
			delete mat;
		}
	}

	for (auto textureKeyVal : _textures)
	{
		delete textureKeyVal.second;
		textureKeyVal.second = nullptr;
	}
}

Texture* Game::LoadTexture(const char* path)
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

Game::MeshAsset Game::LoadMesh(const char* path)
{
	objl::Loader loader;

	auto* renderer = GetRenderer();

	if (loader.LoadFile(path))
	{
		vector<VertexStandard> vertices;

		MeshAsset asset;
		asset.mesh = new Mesh(renderer);

		for (objl::Mesh& meshData : loader.LoadedMeshes)
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

			auto* mat = ConvertMaterial(path, objlMat);

			asset.materials.push_back(mat);
			asset.mesh->AddIndexBuffer(meshData.Indices.data(), static_cast<unsigned int>(meshData.Indices.size()), static_cast<unsigned int>(base));
		}

		asset.mesh->SetVertices(vertices.data(), static_cast<unsigned int>(vertices.size()));
		_meshes.push_back(asset);

		return asset;
	}
	else
	{
		cout << "Could not load mesh: " << path << endl;

		return MeshAsset();
	}
}

Material* Game::ConvertMaterial(const char* path, const objl::Material& objlMat)
{
	auto* mat = new Material{};
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

	return mat;
}

// Inherited via Game

void Game::OnInit()
{
	auto renderer = GetRenderer();

	auto* camera = new CameraActor(this);
	camera->SetParent(_rootActor);
	camera->SetPosition({ 0.0f, 0.0f, -4.0f });
	camera->SetAspectRatio(static_cast<float>(this->GetWidth()) / static_cast<float>(this->GetHeight()));
	AddObserver(*camera);
	_activeCamera = camera;

	auto sphereMesh = LoadMesh("./assets/mesh/sphere_1u.obj");
	auto cubeMesh = LoadMesh("./assets/mesh/cube_1u.obj");

	auto* sphereActor = new MeshActor(this);
	sphereActor->SetMesh(sphereMesh.mesh);
	sphereActor->SetMaterials(sphereMesh.materials);
	sphereActor->SetParent(_rootActor);

	auto* paddleActor = new MeshActor(this);
	paddleActor->SetMesh(cubeMesh.mesh);
	paddleActor->SetMaterials(cubeMesh.materials);
	paddleActor->SetParent(_rootActor);

	XMStoreFloat3(&renderer->lights[0].direction, XMVector3Normalize(XMVectorSet(-1.0f, -1.0f, 0.0f, 0.0f)));
	renderer->lights[0].color = { 1.0, 1.0, 1.0, 1.0f };

	XMStoreFloat3(&renderer->lights[1].direction, XMVector3Normalize(XMVectorSet(1.0f, -1.0f, 1.0f, 0.0f)));
	renderer->lights[1].color = { 0.0, 1.0, 1.0, 1.0f };

	XMStoreFloat3(&renderer->lights[2].direction, XMVector3Normalize(XMVectorSet(-1.0f, -2.0f, -1.0f, 0.0f)));
	renderer->lights[2].color = { 1.0, 0.0, 1.0, 1.0f };

	renderer->ambientLight = { 0.1f, 0.1f, 0.2f, 0.5f };
	renderer->SetClearColor({ 0.1f, 0.1f, 0.2f, 1.0f });
}

void Game::OnUpdate(float elapsed, float delta)
{
	auto input = GetInput();

	if (input->GetKeyDown(Key::ESC))
	{
		auto window = GetWindow();
		window->SetCaptureMouse(!window->GetCaptureMouse());
		window->SetMouseVisible(!window->GetMouseVisible());
	}

	_rootActor->OnUpdate(elapsed, delta);
	_rootActor->OnDraw(GetRenderer());
}

Input* Game::GetInput() const
{
	return static_cast<Input*>(TinyEngineGame::GetInput());
}
