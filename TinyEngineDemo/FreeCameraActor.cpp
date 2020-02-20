#include "FreeCameraActor.h"
#include "SpaceGame.h"

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
	//XMFLOAT3 target = {};
	//XMFLOAT3 up = { 0.0f, 1.0f, 0.0f };
	//return XMMatrixLookAtLH(XMLoadFloat3(&_position), XMLoadFloat3(&target), XMLoadFloat3(&up));

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

	//XMQuaternionAngles

	//if (input->GetKey(Key::W))
	//{
	//	_position.z += delta * 10.0f;
	//}

	//if (input->GetKey(Key::S))
	//{
	//	_position.z -= delta * 10.0f;
	//}

	_position.z += input->GetMouseDelta().y / _game->GetHeight() * delta * 10.0f;

	//XMQuaternionRotationRollPitchYaw()

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
