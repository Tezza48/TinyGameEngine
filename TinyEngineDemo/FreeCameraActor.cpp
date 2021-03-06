#include "FreeCameraActor.h"
#include "Game.h"

using namespace TinyEngine;
using namespace DirectX;

void FreeCameraActor::SetAspectRatio(float aspectRatio)
{
	_aspectRatio = aspectRatio;
}

DirectX::XMFLOAT3 FreeCameraActor::GetEyePosition()
{
	return _position;
}

DirectX::XMMATRIX FreeCameraActor::GetView()
{
	auto world = GetWorld();
	auto det = XMMatrixDeterminant(world);

	return XMMatrixInverse(&det, world);
}

DirectX::XMMATRIX FreeCameraActor::GetProjection()
{
	return XMMatrixPerspectiveFovLH(XM_PIDIV4, _aspectRatio, 0.01f, 1000.0f);
}

void FreeCameraActor::OnUpdate(float elapsed, float delta)
{
	auto input = _game->GetInput();

	const auto mouseDelta = input->GetMouseDelta();

	auto vecOrientation = XMLoadFloat4(&_orientation);
	XMFLOAT3 localRight = { 1.0f, 0.0f, 0.0f };
	XMFLOAT3 localUp = { 0.0f, 1.0f, 0.0f };
	XMFLOAT3 localFwd = { 0.0f, 0.0f, 1.0f };

	auto vecLocalRight = XMLoadFloat3(&localRight);
	auto vecLocalUp = XMLoadFloat3(&localUp);
	auto vecLocalFwd = XMLoadFloat3(&localFwd);

	XMStoreFloat4(&_orientation, vecOrientation);

	auto vecWorldFwd = XMVector3Rotate(vecLocalFwd, vecOrientation);
	auto vecWorldRight = XMVector3Rotate(vecLocalRight, vecOrientation);

	float sensetivity = 2.0;
	float moveSpeed = 5.0f;

	_yaw += mouseDelta.x / _game->GetWidth() * sensetivity;

	_pitch += mouseDelta.y / _game->GetHeight() * sensetivity;
	_pitch = fminf(XM_PIDIV2, _pitch);
	_pitch = fmaxf(-XM_PIDIV2, _pitch);

	XMStoreFloat4(&_orientation, XMQuaternionRotationRollPitchYaw(_pitch, _yaw, 0.0f));

	float fwdInput = 0.0f;
	if (input->GetKey(Key::W)) fwdInput++;
	if (input->GetKey(Key::S)) fwdInput--;

	float rightInput = 0.0f;
	if (input->GetKey(Key::D)) rightInput++;
	if (input->GetKey(Key::A)) rightInput--;

	float upInput = 0.0f;
	if (input->GetKey(Key::R)) upInput++;
	if (input->GetKey(Key::F)) upInput--;


	auto moveDelta = XMVector3Normalize(vecWorldFwd * fwdInput + vecWorldRight * rightInput + vecLocalUp * upInput) * delta * moveSpeed;

	XMStoreFloat3(&_position, XMVectorAdd(XMLoadFloat3(&_position), moveDelta));

	Actor::OnUpdate(elapsed, delta);
}

void FreeCameraActor::OnNotify(const Event& event)
{
	switch (static_cast<TinyEngine::EngineEventType>(event.GetType()))
	{
	case TinyEngine::EngineEventType::WINDOW_RESIZE:
	{
		const auto& resizeEvent = static_cast<const TinyEngine::ResizeEvent&>(event);
		SetAspectRatio(static_cast<float>(resizeEvent.x) / static_cast<float>(resizeEvent.y));

		break;
	}
	default:
		break;
	}
}
