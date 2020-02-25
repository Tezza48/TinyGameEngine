#pragma once

namespace TinyEngine
{
	// Events emitted by the engine.
	enum class EngineEventType
	{
		WINDOW_CREATED,
		WINDOW_CLOSE,
		WINDOW_RESIZE,
		WINDOW_KEY_DOWN,
		WINDOW_KEY_UP,
		WINDOW_MOUSE_MOVE,
		_NUM_ENGINE_EVENTS // Number of Engine events.
	};
}