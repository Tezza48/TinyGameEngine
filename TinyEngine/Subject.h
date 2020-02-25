#pragma once

#include <vector>
#include "Event.h"
#include "IObserver.h"

namespace TinyEngine
{
	// Object which can be observed.
	// Emits events to all observers which have been added.
	class Subject
	{
	private:
		std::vector<IObserver*> _observers;

	public:
		// Register an observer with this subject.
		//	IObserver& observer: Observer to add
		void AddObserver(IObserver& observer);

		// Unregister an observer from this subject.
		//	IObserver& observer: Observer to remove
		void RemoveObserver(IObserver& observer);

		// Notify all current Observers that an event has happened.
		//	const Event& event: Event to pass to observers.
		void Notify(const Event& event);
	};
}

