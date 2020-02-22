#pragma once

#include <DirectXMath.h>
#include <list>
#include "Renderer.h"

class SpaceGame;

class Actor
{
private:
	Actor* _parent = nullptr;
	std::list<Actor*> _children;

protected:
	DirectX::XMFLOAT3 _position = { 0.0f, 0.0f, 0.0f };
	DirectX::XMFLOAT4 _orientation = { 0.0f, 0.0f, 0.0f, 1.0f };
	DirectX::XMFLOAT3 _scale = { 1.0f, 1.0f, 1.0f };

	SpaceGame* _game = nullptr;

public:
	Actor(SpaceGame* game);
	~Actor();

	DirectX::XMFLOAT3 GetPosition() const;
	void SetPosition(DirectX::XMFLOAT3 position);

	DirectX::XMFLOAT3 GetScale() const;
	void SetScale(DirectX::XMFLOAT3 scale);

	DirectX::XMMATRIX GetWorld() const;
	DirectX::XMMATRIX GetLocalTransform() const;

	virtual void AddChild(Actor* child);
	virtual void RemoveChild(Actor* child);
	virtual void SetParent(Actor* parent);

	virtual void OnUpdate(float elapsed, float delta);
	virtual void OnDraw(TinyEngine::Renderer* renderer);
};

