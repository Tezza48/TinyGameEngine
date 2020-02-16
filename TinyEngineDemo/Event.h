#pragma once
#include "Key.h"

namespace TinyEngine
{
	class Event
	{
	private:
		int _type;

	public:
		Event(int type);

		int GetType() const;
	};

	class KeyboardEvent :
		public Event
	{
	public:
		Key key;

		KeyboardEvent(int type, Key key);
	};
}

