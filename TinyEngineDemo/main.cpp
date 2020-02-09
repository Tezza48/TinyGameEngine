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

class Game
	: public TinyEngineGame
{
private:
	vector<Mesh*> _meshes;
	vector<Material*> _materials;
	map<string, Texture*> _textures;
	Texture* _nullTexture;

	PerspectiveCamera _camera;
	objl::Loader* _objLoader;

public:
	Game() : TinyEngineGame(1280, 720, "Game"), _objLoader(nullptr)
	{
		_camera.SetPosition({ 0.0f, 1.0f, 0.0f });
		_camera.LookAt({ 0.0f, 0.0f, 0.0f });

		_nullTexture = new Texture();
	}

	~Game()
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
	}

private:
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
			PRINT_ERROR("STB Failed to load Image: " << failure);
		}

		Texture* tex = new Texture(texData, w, h);
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

		if (_objLoader->LoadFile(path))
		{
			vector<VertexStandard> vertices;

			auto mesh = new Mesh();
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

				if (objlMat.map_Kd != "") {
					std::filesystem::path texPath(path);
					texPath.remove_filename().append(objlMat.map_Kd);
					mat->diffuseTexture = LoadTexture(texPath.string().c_str());
				}
				else
				{
					mat->diffuseTexture = _nullTexture;
					mat->diffuse = XMFLOAT3(&objlMat.Kd.X);
				}

				if (objlMat.map_Ks != "") {
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

	Game game;
	game.Run();
}