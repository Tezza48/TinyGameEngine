#pragma once

#include "Game.h"
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

class SpaceGame :
	public TinyEngine::Game
{
private:
	// Asset manager?
	std::vector<TinyEngine::Mesh*> _meshes;
	std::vector<TinyEngine::Material*> _materials;
	std::unordered_map<std::string, TinyEngine::Texture*> _textures;
	TinyEngine::Texture* _nullTexture;

	// Game
	MeshActor* _skyboxActor;
	TinyEngine::Shader* _skyboxShader;

	Actor* _rootActor;

public:
	TinyEngine::ICamera* _activeCamera;

public:
	SpaceGame(int width, int height, const char* title);

	~SpaceGame();

	TinyEngine::Texture* LoadTexture(const char* path);

	std::vector<TinyEngine::Mesh*> LoadMeshes(const char* path);

	// Inherited via Game
	virtual void OnInit() override;

	virtual void OnUpdate(float elapsed, float delta) override;

	virtual Input* GetInput() const override;
};