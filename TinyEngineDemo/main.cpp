#define TINY_ENGINE_IMPLEMENTATION
#define TINY_ENGINE_LAZY_LIBS

#include "TinyEngine.h"
#include "vendor/OBJ_Loader.h";

#define SAFE_DELETE(ptr) if (ptr) { delete ptr; ptr = nullptr; }

using namespace TinyEngine;
using namespace std::chrono;

class Game
	: public TinyEngineGame
{
private:
	vector<Mesh*> _meshes;
	vector<Material*> _materials;
	PerspectiveCamera _camera;

public:
	Game() : TinyEngineGame(1280, 720, "Game")
	{
		_camera.LookAt({0.0f, 0.0f, 0.0f});
		_camera.SetAspectRatio(GetWidth() / GetHeight());
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
	}

private:
	void OnInit() override
	{
		objl::Loader loader;
		if (loader.LoadFile("./assets/mesh/planet.obj"))
		{
			vector<VertexStandard> vertices;

			auto mesh = new Mesh();
			_meshes.push_back(mesh);

			for (objl::Mesh& meshData : loader.LoadedMeshes)
			{
				unsigned int base = vertices.size();
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
				mat->diffuse = XMFLOAT3(&objlMat.Kd.X);
				mat->specular = XMFLOAT3(&objlMat.Ks.X);
				mat->specularExponent = objlMat.Ns;
				mat->transparency = objlMat.d;

				_materials.push_back(mat);

				mesh->AddIndexBuffer(meshData.Indices.data(), meshData.Indices.size(), base, mat);
			}

			mesh->SetVertices(vertices.data(), vertices.size());
		}
	}

	bool OnUpdate(float time, float delta) override
	{
		_camera.SetPosition({ sinf(time) * 4.0f, 1.0f, cosf(time) * 4.0f });

		for (auto* mesh : _meshes)
		{
			DrawMesh(mesh, &_camera, XMMatrixIdentity());
		}

		return true;
	}
};

int main(int argc, char** argv)
{
#if DEBUG || _DEBUG
	for (auto i = 0, l = argc - 1; i < l; i++)
	{
		if (string(argv[i]) == "/r") {
			cout << "Setting Working Directory to: " << argv[i + 1] << endl;
			SetCurrentDirectory(argv[i + 1]);
			break;
		}
	}

#endif

	Game game;
	game.Run();
}