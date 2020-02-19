#pragma once
#include <Windows.h>
#include <vector>
#include "Subject.h"
#include "EngineEventType.h"

namespace TinyEngine
{
	class Window :
		public Subject
	{
	private:
		HWND _window;

	public:
		Window(int width, int height, const char* title);
		~Window();

		void PeekMessages();

#ifdef TINY_ENGINE_EXPOSE_NATIVE
		inline HWND GetWindow() const
		{
			return _window;
		}
#endif

	private:
		static LRESULT CALLBACK WndProc(HWND hwnd, UINT uMsg, WPARAM wparam, LPARAM lparam);
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

