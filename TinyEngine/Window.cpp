#include "Window.h"
#include <iostream>
#include "Game.h"
#include "Key.h"

using std::cout;
using std::endl;

TinyEngine::Window::Window(int width, int height, const char* title)
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
		cout << "Failed to register class." << endl;
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

	if (!_window) {
		cout << "Failed to create Window" << endl;
	}
}

TinyEngine::Window::~Window()
{
	DestroyWindow(_window);
}

void TinyEngine::Window::PeekMessages()
{
	MSG message = {};

	while (PeekMessage(&message, _window, NULL, NULL, PM_REMOVE))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}
}

void TinyEngine::Window::SetCaptureMouse(bool shouldCapture)
{
	_captureMouse = shouldCapture;
}

bool TinyEngine::Window::GetCaptureMouse() const
{
	return _captureMouse;
}

void TinyEngine::Window::SetMouseVisible(bool isVisible)
{
	ShowCursor(isVisible);
	_mouseVisible = isVisible;
}

bool TinyEngine::Window::GetMouseVisible() const
{
	return _mouseVisible;
}

LRESULT TinyEngine::Window::WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam)
{
	static Window* window;

	switch (uMsg)
	{
	case WM_CREATE:
	{
		window = reinterpret_cast<Window*>(reinterpret_cast<LPCREATESTRUCT>(lparam)->lpCreateParams);
		
		return 0;
	}		
	case WM_SIZE:
	{
		const auto width = LOWORD(lparam);
		const auto height = HIWORD(lparam);

		window->_restingMouseX = width / 2;
		window->_restingMouseY = height / 2;

		cout << width << " " << window->_restingMouseX << endl;

		window->Notify(ResizeEvent(width, height));

		return 0;
	}
	case WM_KEYDOWN:
	{
		if (~(lparam >> 30) & 1)
		{
			window->Notify(KeyboardEvent(static_cast<int>(EngineEventType::WINDOW_KEY_DOWN), static_cast<Key>(wparam)));

		}

		return 0;
	}
	case WM_KEYUP:
	{
		window->Notify(KeyboardEvent(static_cast<int>(EngineEventType::WINDOW_KEY_UP), static_cast<Key>(wparam)));

		return 0;
	}
	case WM_MOUSEMOVE:
	{
		if (window->_captureMouse)
		{
			float x = static_cast<float>(LOWORD(lparam) - window->_restingMouseX);
			float y = static_cast<float>(HIWORD(lparam) - window->_restingMouseY);
			window->Notify(Vec2Event(static_cast<int>(EngineEventType::WINDOW_MOUSE_MOVE), x, y));

			POINT originScreenSpace = { };
			ClientToScreen(window->_window, &originScreenSpace);
			SetCursorPos(originScreenSpace.x + window->_restingMouseX, originScreenSpace.y + window->_restingMouseY);
		}

		return 0;
	}
	case WM_CLOSE:
	{
		window->Notify(Event(static_cast<int>(EngineEventType::WINDOW_CLOSE)));
		DestroyWindow(window->_window);

		return 0;
	}
	case WM_DESTROY:
	{

		return 0;
	}
	}

	return DefWindowProc(hwnd, uMsg, wparam, lparam);
}