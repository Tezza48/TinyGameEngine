#include "BaseInput.h"
#include "EngineEventType.h"

void TinyEngine::BaseInput::OnNotify(const Event& event)
{
	const auto type = static_cast<EngineEventType>(event.GetType());

	if (type != EngineEventType::WINDOW_KEY_DOWN && type != EngineEventType::WINDOW_KEY_UP)
	{
		return;
	}

	const auto& keyboardEvent = static_cast<const KeyboardEvent&>(event);

	switch (type)
	{
	case EngineEventType::WINDOW_KEY_DOWN:
		OnKeyDown(keyboardEvent.key);

		break;
	case EngineEventType::WINDOW_KEY_UP:
		OnKeyUp(keyboardEvent.key);

		break;
	default:
		break;
	}
}
