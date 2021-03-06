#include "Event.h"
#include "Key.h"

TinyEngine::Event::Event(int type) : _type(type)
{

}


int TinyEngine::Event::GetType() const
{
	return _type;
}

TinyEngine::KeyboardEvent::KeyboardEvent(int type, Key key) : Event(type), key(key) { }

TinyEngine::Vec2Event::Vec2Event(int type, float x, float y): Event(type), x(x), y(y)
{
}
