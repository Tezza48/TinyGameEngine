#include "Input.h"

void Input::OnUpdate()
{
	lastKeyboard = thisKeyboard;
	thisKeyboard = liveKeyboard;

	lastMouse = thisMouse;
	thisMouse = liveMouse;
}

bool Input::GetKey(TinyEngine::Key key)
{
	return thisKeyboard[key];
}

bool Input::GetKeyDown(TinyEngine::Key key)
{
	return !lastKeyboard[key] && thisKeyboard[key];
}

bool Input::GetKeyUp(TinyEngine::Key key)
{
	return lastKeyboard[key] && !thisKeyboard[key];
}

DirectX::XMFLOAT2 Input::GetMouseDelta()
{
	return { thisMouse.x - lastMouse.x, thisMouse.y - lastMouse.y };
}

// Inherited via BaseInput

void Input::OnKeyDown(TinyEngine::Key key)
{
	liveKeyboard[key] = true;
}

void Input::OnKeyUp(TinyEngine::Key key)
{
	liveKeyboard[key] = false;
}

void Input::OnMouseMove(float x, float y)
{
	liveMouse.x += x;
	liveMouse.y += y;
}
