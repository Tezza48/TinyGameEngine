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

	class Vec2Event :
		public Event
	{
	public:
		float x;
		float y;

		Vec2Event(int type, float x, float y);
	};
}

