#pragma once
#include "BaseInput.h"
#include <unordered_map>

class Input :
	public TinyEngine::BaseInput
{
private:
	std::unordered_map<TinyEngine::Key, bool> lastKeyboard;
	std::unordered_map<TinyEngine::Key, bool> thisKeyboard;
	std::unordered_map<TinyEngine::Key, bool> liveKeyboard;

public:
	void OnUpdate()
	{
		lastKeyboard = thisKeyboard;
		thisKeyboard = liveKeyboard;
	}

	bool GetKey(TinyEngine::Key key)
	{
		return thisKeyboard[key];
	}

	bool GetKeyDown(TinyEngine::Key key)
	{
		return !lastKeyboard[key] && thisKeyboard[key];
	}

	bool GetKeyUp(TinyEngine::Key key)
	{
		return lastKeyboard[key] && !thisKeyboard[key];
	}

private:
	// Inherited via BaseInput
	virtual void OnKeyDown(TinyEngine::Key key)
	{
		liveKeyboard[key] = true;
	}

	virtual void OnKeyUp(TinyEngine::Key key)
	{
		liveKeyboard[key] = false;
	}
};