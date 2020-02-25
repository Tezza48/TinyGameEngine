#pragma once

#include "Event.h"

namespace TinyEngine
{
	// Object which can observe events on a Subject.
	class IObserver
	{
	public:
		// Called when the subject has been notified.
		//	const Event& event: Event which has happened
		virtual void OnNotify(const Event& event) = 0;
	};
}