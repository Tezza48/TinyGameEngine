#pragma once
#include "Actor.h"
#include "ICamera.h"
#include "Subject.h"

class FreeCameraActor :
	public Actor, public TinyEngine::ICamera, public TinyEngine::IObserver
{
private:
	float _aspectRatio = 1.0f;

public:
	FreeCameraActor(SpaceGame* game) : Actor(game) {}

	void SetAspectRatio(float aspectRatio);

	// Inherited via ICamera
	virtual DirectX::XMFLOAT3 GetEyePosition() override;
	virtual DirectX::XMMATRIX GetView() override;
	virtual DirectX::XMMATRIX GetProjection() override;

	// Inherited via Actor
	virtual void OnUpdate(float elapsed, float delta) override;

	// Inherited via IObserver
	virtual void OnNotify(const TinyEngine::Event& event);
};

