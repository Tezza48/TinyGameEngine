#pragma once

#include "Subject.h"
#include "Key.h"

namespace TinyEngine
{
	// Base class for an input handler. Acts as a null object if not overridden.
	class BaseInput :
		public IObserver
	{
	public:
		// Called every frame.
		virtual void OnUpdate() {}

		// Inherited via Observer

		// Called when the subject has been notified.
		//	const Event& event: Event which has happened
		virtual void OnNotify(const Event& event) override;

	private:
		// Called when a key has been pressed down.
		//	Key key: Key which has been pressed
		virtual void OnKeyDown(Key key) {};

		// Called when a key has been released.
		//	Key key: Key which has been released
		virtual void OnKeyUp(Key key) {};

		// Called when the mouse has moved. Screen Space.
		//	float x: Change in horizontal direction
		//	float y: Change in vertical direction
		virtual void OnMouseMove(float x, float y) {};
	};
}

