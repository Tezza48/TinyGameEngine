#include "SpaceGame.h"

#pragma warning(disable: 4067 4244)
#include "vendor/OBJ_Loader.h";
#pragma warning(default: 4067 4244)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "vendor\stb_image.h"

#include <iostream>
#include <DirectXMath.h>
#include <filesystem>
#include "FreeCameraActor.h"
								
using namespace DirectX;
using namespace TinyEngine;

using std::cout;
using std::endl;
using std::vector;

SpaceGame::SpaceGame(int width, int height, const char* title) : TinyEngine::Game(width, height, title), _skyboxShader(nullptr)
{
	SetInputHandler(new Input());

	_nullTexture = new Texture(GetRenderer());

	_rootActor = new Actor(this);
}

SpaceGame::~SpaceGame()
{
	delete _rootActor;
	_rootActor = nullptr;

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

	delete _skyboxShader;
	_skyboxShader = nullptr;
}

Texture* SpaceGame::LoadTexture(const char* path)
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

std::vector<Mesh*> SpaceGame::LoadMeshes(const char* path)
{
	objl::Loader loader;

	vector<Mesh*> meshes;

	auto* renderer = GetRenderer();

	if (loader.LoadFile(path))
	{
		vector<VertexStandard> vertices;

		auto mesh = new Mesh(renderer);
		meshes.push_back(mesh);

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

			_materials.push_back(mat);

			mesh->AddIndexBuffer(meshData.Indices.data(), static_cast<unsigned int>(meshData.Indices.size()), static_cast<unsigned int>(base), mat);
		}

		mesh->SetVertices(vertices.data(), static_cast<unsigned int>(vertices.size()));
	}

	_meshes.insert(_meshes.end(), meshes.begin(), meshes.end());
	return meshes;
}

// Inherited via Game

void SpaceGame::OnInit()
{
	auto renderer = GetRenderer();

	auto* freeCamera = new FreeCameraActor(this);
	freeCamera->SetParent(_rootActor);
	freeCamera->SetPosition({ 0.0f, 0.0f, -20.0f });
	freeCamera->SetAspectRatio(static_cast<float>(this->GetWidth()) / static_cast<float>(this->GetHeight()));

	AddObserver(*freeCamera);

	_activeCamera = freeCamera;

	D3D11_INPUT_ELEMENT_DESC inputDescs[3] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA }
	};

	_skyboxShader = new Shader(renderer, "./assets/shader/DefaultVertexShader.cso", "./assets/shader/SkyboxPixelShader.cso", inputDescs, 3);

	auto skyboxMesh = LoadMeshes("./assets/mesh/nightSkySphere.obj");
	skyboxMesh[0]->GetPartMaterial(0)->shader = _skyboxShader;

	auto* skybox = new MeshActor(this);
	skybox->SetMesh(skyboxMesh[0]);
	skybox->SetParent(freeCamera);

	auto venusMeshes = LoadMeshes("./assets/mesh/planets/venus.obj");

	auto* meshActor = new MeshActor(this);
	meshActor->SetMesh(venusMeshes[0]);
	meshActor->SetParent(_rootActor);

	XMStoreFloat3(&renderer->lights[0].direction, XMVector3Normalize(XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f)));
	renderer->lights[0].color = { 1.0, 1.0, 1.0, 0.5f };

	renderer->ambientLight = { 0.0f, 0.0f, 0.0f, 1.0f };
	renderer->SetClearColor(renderer->ambientLight);

	renderer->lights[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderer->lights[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };
}

void SpaceGame::OnUpdate(float elapsed, float delta)
{
	const auto input = GetInput();
	input->OnUpdate();

	_rootActor->OnUpdate(elapsed, delta);

	_rootActor->OnDraw(GetRenderer());
}

Input* SpaceGame::GetInput() const
{
	return static_cast<Input*>(TinyEngine::Game::GetInput());
}
