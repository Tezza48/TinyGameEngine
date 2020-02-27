#pragma once

#include "TinyEngineGame.h"
#include <vector>
#include <unordered_map>
#include "Input.h"
#include <string>
#include "Actor.h"
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"
#include "FreeCameraActor.h"
#include "MeshActor.h"

// OBJL is a bit rubbish, since i cant indlude the header i've prototyped what i need here.
namespace objl
{
	struct Material;
}

class Game :
	public TinyEngine::TinyEngineGame
{
public:
	struct MeshAsset
	{
		TinyEngine::Mesh* mesh;
		std::vector<TinyEngine::Material*> materials;
	};

private:
	// Game
	Actor* _rootActor;
	Input _inputHandler;

public:
	// Asset manager?
	// TODO WT: should all be maps so assets can be requested by name, far easier to work with.
	std::vector<MeshAsset> _meshes;
	std::unordered_map<std::string, TinyEngine::Texture*> _textures;
	TinyEngine::Texture* _nullTexture;

	// Game
	TinyEngine::ICamera* _activeCamera;

public:
	Game(int width, int height, const char* title);
	~Game();

	TinyEngine::Texture* LoadTexture(const char* path);

	MeshAsset LoadMesh(const char* path);

	TinyEngine::Material* ConvertMaterial(const char* path, const objl::Material& objlMat);

	// Inherited via Game
	virtual void OnInit() override;

	virtual void OnUpdate(float elapsed, float delta) override;

	virtual Input* GetInput() const override;
};