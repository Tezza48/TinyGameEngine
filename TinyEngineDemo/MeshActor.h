#pragma once
#include "Mesh.h"
#include "Actor.h"

class SpaceGame;

class MeshActor :
	public Actor
{
private:
	TinyEngine::Mesh* _mesh;
	std::vector<TinyEngine::Material*> _materials;

public:
	MeshActor(SpaceGame* game) : Actor(game), _mesh(nullptr) { }

	void SetMesh(TinyEngine::Mesh* mesh);
	void SetMaterials(std::vector<TinyEngine::Material*> materials);

	virtual void OnDraw(TinyEngine::Renderer* renderer) override;
};

