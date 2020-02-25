#pragma once
#include "Key.h"

namespace TinyEngine
{
	// A base Event class.
	class Event
	{
	private:
		int _type;

	public:
		// Creates an Event instance.
		// int type: This event's type. Typically an enum cast to int
		Event(int type);

		// Get this event's type.
		//	returns: This event's type
		int GetType() const;
	};

	// An event representing a Keyboard input.
	class KeyboardEvent :
		public Event
	{
	public:
		// Which key was pressed/released
		Key key;

		// Construct a KeyboardEvent instance.
		//	int type: This event's type
		//	Key key: Which key has been affected
		KeyboardEvent(int type, Key key);
	};

	// An event representing a 2 component vector.
	class Vec2Event :
		public Event
	{
	public:
		// X component.
		float x;

		// Y component.
		float y;

		// Construct a Vec2Event instance.
		//	int type: This event's type
		//	float x: X component
		//	float y: Y component
		Vec2Event(int type, float x, float y);
	};
}

