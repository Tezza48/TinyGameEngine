#include "Subject.h"
#include <iostream>

using std::cout;
using std::endl;

void TinyEngine::Subject::AddObserver(IObserver& observer)
{
	for (auto it = _observers.begin(); it != _observers.end(); it++)
	{
		if (*it == &observer) {
			cout << "You have already added this observer." << endl;
			return;
		}
	}
	_observers.push_back(&observer);
}

void TinyEngine::Subject::RemoveObserver(IObserver& observer)
{
	for (auto it = _observers.begin(); it != _observers.end(); it++)
	{
		if (*it == &observer)
		{
			_observers.erase(it);
			break;
		}
	}
}

void TinyEngine::Subject::Notify(const Event& event)
{
	for (const auto& observer : _observers)
	{
		observer->OnNotify(event);
	}
}