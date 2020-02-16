#pragma once

#include <vector>
#include "Event.h"

namespace TinyEngine
{

	class IObserver
	{
	public:
		virtual void OnNotify(const Event& event) = 0;
	};

	class Subject
	{
	private:
		std::vector<IObserver*> _observers;

	public:
		void AddObserver(IObserver& observer);
		void RemoveObserver(IObserver& observer);

		void Notify(const Event& event);
	};
}

