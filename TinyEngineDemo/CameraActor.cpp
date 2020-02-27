#include "CameraActor.h"
#include <DirectXMath.h>
#include "Event.h"

using namespace DirectX;
using namespace TinyEngine;

void CameraActor::SetAspectRatio(float aspectRatio)
{
	_aspectRatio = aspectRatio;
}

DirectX::XMFLOAT3 CameraActor::GetEyePosition()
{
	return this->_position;
}

DirectX::XMMATRIX CameraActor::GetView()
{
	const auto world = GetWorld();
	auto det = XMMatrixDeterminant(world);
	return XMMatrixInverse(&det, world);
}

DirectX::XMMATRIX CameraActor::GetProjection()
{
	return XMMatrixPerspectiveFovLH(XM_PIDIV4, _aspectRatio, 0.01f, 1000.0f);
}

void CameraActor::OnNotify(const Event& event)
{
	switch (static_cast<EngineEventType>(event.GetType()))
	{
	case EngineEventType::WINDOW_RESIZE:
	{
		const auto& resizeEvent = static_cast<const ResizeEvent&>(event);
		SetAspectRatio(static_cast<float>(resizeEvent.x) / static_cast<float>(resizeEvent.y));

		break;
	}
	default:
		break;
	}
}
