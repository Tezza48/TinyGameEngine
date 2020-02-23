#pragma once
#include "Actor.h"
#include <DirectXMath.h>
#include "Renderer.h"
#include "Mesh.h"
#include <vector>

class SpaceGame;

class Universe :
	public Actor
{
private:
	TinyEngine::Mesh* _starMesh;

	DirectX::XMFLOAT3 _lastPosition = {};

public:
	Universe(SpaceGame* game, TinyEngine::Mesh* starMesh);

	void Clear();
	void Generate(DirectX::XMFLOAT3 position);

	virtual void OnDraw(TinyEngine::Renderer* renderer) override;
	virtual void OnUpdate(float elapsed, float delta) override;

private:
	static int hashVector3(DirectX::XMINT3 seed);
};

