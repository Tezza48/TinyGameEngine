#include "BaseInput.h"
#include "EngineEventType.h"

void TinyEngine::BaseInput::OnNotify(const Event& event)
{
	const auto type = static_cast<EngineEventType>(event.GetType());


	switch (type)
	{
	case EngineEventType::WINDOW_KEY_DOWN:
	{
		const auto& keyboardEvent = static_cast<const KeyboardEvent&>(event);
		OnKeyDown(keyboardEvent.key);

		break;

	}
	case EngineEventType::WINDOW_KEY_UP:
	{
		const auto& keyboardEvent = static_cast<const KeyboardEvent&>(event);
		OnKeyUp(keyboardEvent.key);

		break;
	}
	case EngineEventType::WINDOW_MOUSE_MOVE:
	{
		const auto& mouseEvent = static_cast<const Vec2Event&>(event);
		OnMouseMove(mouseEvent.x, mouseEvent.y);

		break;
	}
	default:
		break;
	}
}
