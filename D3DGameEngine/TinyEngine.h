#pragma once

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

class TinyEngine
{
public:
protected:
	//Assets _assets;

private:
	HWND _window = 0;
	bool isRunning;

public:
	TinyEngine(int width, int height, const char* Title);

	void Run();

protected:
	virtual void OnInit() = 0;
	virtual bool OnUpdate() = 0;

private:
	static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);
};

class Renderer {
	// DrawSprite
};

class Mesh {

};

class Shader {

};

class Input {

};

class Scene {

};

class Assets {
public:
	//Shader* LoadShader(const char* path);
	//Mesh* LoadMesh(const char* path);
};

#ifdef TINY_ENGINE_IMPLEMENTATION
#include <iostream>

using std::cerr;
using std::endl;

LRESULT CALLBACK TinyEngine::WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	static TinyEngine* engine;

	switch (uMsg)
	{
	case WM_CREATE:
		engine = reinterpret_cast<TinyEngine*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
		engine->isRunning = true;
		return 0;

	case WM_CLOSE:
		DestroyWindow(hwnd);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		engine->isRunning = false;
		return 0;

	}

	return DefWindowProc(hwnd, uMsg, wparam, lparam);;
}

TinyEngine::TinyEngine(int width, int height, const char* title)
{
	WNDCLASS wc = {};
	wc.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
	wc.lpfnWndProc = WndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = 0;
	wc.hInstance = GetModuleHandle(nullptr);
	wc.hIcon = LoadIcon(NULL, IDI_APPLICATION);
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = nullptr;
	wc.lpszMenuName = nullptr;
	wc.lpszClassName = "TinyEngine";

	if (!RegisterClass(&wc))
	{
		cerr << __FILE__ << ":" << __LINE__ << " Failed to register class." << endl;
	}

	DWORD dwStyleEX = NULL;
	DWORD dwStyle = WS_OVERLAPPEDWINDOW | WS_VISIBLE;
	RECT windowRect = { 0, 0, width, height };
	AdjustWindowRectEx(&windowRect, dwStyle, false, dwStyleEX);

	_window = CreateWindowEx(
		dwStyleEX,
		wc.lpszClassName,
		title,
		dwStyle,
		CW_USEDEFAULT, CW_USEDEFAULT,
		windowRect.right - windowRect.left, windowRect.bottom - windowRect.top,
		NULL,
		NULL,
		wc.hInstance,
		this);

	if (!_window)
	{
		cerr << __FILE__ << ":" << __LINE__ << " Failed to Create Window." << endl;
	}

	// Create D3D device/context.
	// create sound stuff.
}

void TinyEngine::Run()
{
	OnInit();

	MSG message;

	while (OnUpdate())
	{
		if (PeekMessage(&message, _window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&message);
			DispatchMessage(&message);
		}

		if (!isRunning) {
			break;
		}
	}
}

#endif