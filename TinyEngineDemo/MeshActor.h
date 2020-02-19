#pragma once
#include "Mesh.h"
#include "Actor.h"

class SpaceGame;

class MeshActor :
	public Actor
{
private:
	TinyEngine::Mesh* _mesh;

public:
	MeshActor(SpaceGame* game) : Actor(game) { }
	void SetMesh(TinyEngine::Mesh* mesh);

	virtual void OnDraw(TinyEngine::Renderer* renderer) override;
};

