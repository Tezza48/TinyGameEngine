#pragma once
#include <vector>
#include "Subject.h"
#include "EngineEventType.h"

struct HWND__;
typedef HWND__* HWND;

#if _WIN64
typedef long long LRESULT;
typedef unsigned int UINT;
typedef unsigned long long WPARAM;
typedef long long LPARAM;
#else
typedef long LRESULT;
typedef unsigned int UINT;
typedef unsigned int WPARAM;
typedef long LPARAM;
#endif

namespace TinyEngine
{
	class Window :
		public Subject
	{
	private:
		HWND _window;
		int _restingMouseX;
		int _restingMouseY;
		bool _captureMouse = false;
		bool _mouseVisible = true;

	public:
		Window(int width, int height, const char* title);
		~Window();

		void PeekMessages();

		// Set whether the mouse is being captured by the window.
		// Locks the cursor to the centre of the window.
		void SetCaptureMouse(bool shouldCapture);

		// Get whether the mouse is being captured by the window.
		bool GetCaptureMouse() const;

		// Set whether the mouse cursor is visible or not.
		void SetMouseVisible(bool isVisible);
		// Get whether the mouse cursor is visible or not.
		bool GetMouseVisible() const;

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		inline HWND GetWindow() const
		{
			return _window;
		}
#endif

	private:
		static LRESULT __stdcall WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);
	};

	class ResizeEvent :
		public Event
	{
	public:
		int x;
		int y;

		ResizeEvent(int x, int y) : Event(static_cast<int>(EngineEventType::WINDOW_RESIZE)), x(x), y(y) {}
	};
}

