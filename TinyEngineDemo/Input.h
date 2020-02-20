#pragma once
#include "BaseInput.h"
#include <unordered_map>
#include <DirectXMath.h>

class Input :
	public TinyEngine::BaseInput
{
private:
	std::unordered_map<TinyEngine::Key, bool> lastKeyboard;
	std::unordered_map<TinyEngine::Key, bool> thisKeyboard;
	std::unordered_map<TinyEngine::Key, bool> liveKeyboard;

	DirectX::XMFLOAT2 lastMouse;
	DirectX::XMFLOAT2 thisMouse;
	DirectX::XMFLOAT2 liveMouse;

public:
	void OnUpdate();

	bool GetKey(TinyEngine::Key key);

	bool GetKeyDown(TinyEngine::Key key);

	bool GetKeyUp(TinyEngine::Key key);

	DirectX::XMFLOAT2 GetMouseDelta();

private:
	// Inherited via BaseInput
	virtual void OnKeyDown(TinyEngine::Key key);
	virtual void OnKeyUp(TinyEngine::Key key);
	virtual void OnMouseMove(float x, float y);
};