#pragma once
#include "ICamera.h"
#include "Actor.h"
#include "IObserver.h"

class Game;

class CameraActor :
	public Actor, public TinyEngine::ICamera, public TinyEngine::IObserver
{
private:
	float _aspectRatio = 1.0f;

public:
	CameraActor(Game* game) : Actor(game) {}

	void SetAspectRatio(float aspectRatio);

	// Inherited via ICamera
	virtual DirectX::XMFLOAT3 GetEyePosition() override;
	virtual DirectX::XMMATRIX GetView() override;
	virtual DirectX::XMMATRIX GetProjection() override;

	// Inherited via IObserver
	virtual void OnNotify(const TinyEngine::Event& event) override;
};

