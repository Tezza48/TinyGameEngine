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
#include <random>
								
using namespace DirectX;
using namespace TinyEngine;

using std::cout;
using std::endl;
using std::vector;
using std::linear_congruential_engine;

SpaceGame::SpaceGame(int width, int height, const char* title) : TinyEngine::Game(width, height, title)
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
	freeCamera->SetPosition({ 0.0f, 0.0f, -50.0f });
	freeCamera->SetAspectRatio(static_cast<float>(this->GetWidth()) / static_cast<float>(this->GetHeight()));

	AddObserver(*freeCamera);

	_activeCamera = freeCamera;

	D3D11_INPUT_ELEMENT_DESC inputDescs[3] = {
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
	{ "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA },
	{ "NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D11_APPEND_ALIGNED_ELEMENT, D3D11_INPUT_PER_VERTEX_DATA }
	};

	auto sphereMesh = LoadMeshes("./assets/mesh/sphere_1u.obj")[0];

	XMStoreFloat3(&renderer->lights[0].direction, XMVector3Normalize(XMVectorSet(-1.0f, 0.0f, 0.0f, 0.0f)));
	renderer->lights[0].color = { 1.0, 1.0, 1.0, 0.0f };

	renderer->ambientLight = { 1.0f, 1.0f, 1.0f, 1.0f };
	renderer->SetClearColor({ 0.0f, 0.0f, 0.0f, 1.0f });

	renderer->lights[1].color = { 0.0f, 0.0f, 0.0f, 0.0f };
	renderer->lights[2].color = { 0.0f, 0.0f, 0.0f, 0.0f };

	// Universe map scene...
	auto planetHolder = new Actor(this);
	planetHolder->SetParent(_rootActor);

	MeshActor* planets[16 * 16 * 16];

	for (int z = 0; z < 16; z++)
	{
		for (int y = 0; y < 16; y++)
		{
			for (int x = 0; x < 16; x++)
			{
				//const r = rand();
				if (0.2f < (static_cast<float>(rand()) / RAND_MAX))
				{
					continue;
				}

				// TODO WT: Meshes and materials should not exist together cos it makes this stuff confusing and inefficent.
				// Suggesting either a MeshRenderer component class or the MeshActor maintains a list of material and the mesh itself.

				// Grab a copy of the mesh, will have the same internal buffers
				auto meshCpy = new Mesh(*sphereMesh);
				_meshes.push_back(meshCpy);

				// Copy the mesh's material
				auto mat = new Material(*meshCpy->GetPartMaterial(0));
				// Set the mat's ambient to a random color
				mat->ambient = {
					(static_cast<float>(rand()) / RAND_MAX),
					(static_cast<float>(rand()) / RAND_MAX),
					(static_cast<float>(rand()) / RAND_MAX)
				};
				_materials.push_back(mat);

				meshCpy->SetPartMaterial(0, mat);

				auto newPlanet = new MeshActor(this);
				newPlanet->SetMesh(meshCpy);
				newPlanet->SetParent(planetHolder);
				planets[z * 16 * 16 + y * 16 + x] = newPlanet;

				float posScale = 16.0f;

				float xPos = ((static_cast<float>(x) - 8.0f) + (float)rand() / RAND_MAX)* posScale;
				float yPos = ((static_cast<float>(y) - 8.0f) + (float)rand() / RAND_MAX)* posScale;
				float zPos = ((static_cast<float>(z) - 8.0f) + (float)rand() / RAND_MAX)* posScale;

				newPlanet->SetPosition({ xPos, yPos, zPos });

				auto scale = static_cast<float>(rand()) / RAND_MAX;
				scale += 0.5f;

				newPlanet->SetScale({ scale, scale, scale });
			}
		}
	}
}

void SpaceGame::OnUpdate(float elapsed, float delta)
{
	auto input = GetInput();
	input->OnUpdate();

	if (input->GetKeyDown(Key::ESC))
	{
		auto window = GetWindow();
		window->SetCaptureMouse(!window->GetCaptureMouse());
		window->SetMouseVisible(!window->GetMouseVisible());
	}

	_rootActor->OnUpdate(elapsed, delta);

	_rootActor->OnDraw(GetRenderer());
}

Input* SpaceGame::GetInput() const
{
	return static_cast<Input*>(TinyEngine::Game::GetInput());
}
