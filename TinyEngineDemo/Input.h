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
	virtual void OnUpdate();

	virtual bool GetKey(TinyEngine::Key key);
	virtual bool GetKeyDown(TinyEngine::Key key);
	virtual bool GetKeyUp(TinyEngine::Key key);

	DirectX::XMFLOAT2 GetMouseDelta();

private:
	// Inherited via BaseInput
	virtual void OnKeyDown(TinyEngine::Key key);
	virtual void OnKeyUp(TinyEngine::Key key);
	virtual void OnMouseMove(float x, float y);
};

// You could disable input by implementing a null override of Input like this.
// Pass an instance of this into Game::SetInputHandler and input is effectively disabled.
//
//			class NullInput :
//				public Input
//			{
//			public:
//				virtual void OnUpdate() override {}
//			
//				virtual bool GetKey(TinyEngine::Key key) override { return false; }
//				virtual bool GetKeyDown(TinyEngine::Key key) override { return false; }
//				virtual bool GetKeyUp(TinyEngine::Key key) override { return false; }
//			
//			private:
//				virtual void OnKeyDown(TinyEngine::Key key) override {}
//				virtual void OnKeyUp(TinyEngine::Key key) override {}
//				virtual void OnMouseMove(float x, float y) override {}
//			};