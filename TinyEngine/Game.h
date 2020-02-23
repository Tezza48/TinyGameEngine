#pragma once

#include "Renderer.h"
#include "Window.h"
#include "BaseInput.h"
#include <chrono>

namespace TinyEngine
{
	class Game :
		public IObserver, public Subject
	{
	public:
	protected:
		bool _isRunning;

	private:
		int _width;
		int _height;

		Window* _window;
		BaseInput* _input;
		Renderer* _renderer;

		std::chrono::high_resolution_clock::time_point _startTime;

	public:
		Game(int width, int height, const char* title);
		~Game();

		Game(const Game& game) = delete;

		int GetWidth() const;
		int GetHeight() const;

		void Run();

	protected:
		void SetInputHandler(BaseInput* input);

		Renderer* GetRenderer() const;
		Window* GetWindow() const;

		virtual BaseInput* GetInput() const;

		// Inherited via Observer
		virtual void OnNotify(const Event& event) override;

		virtual void OnInit() = 0;
		virtual void OnUpdate(float elapsed, float delta) = 0;

	private:
		void OnResize(int width, int height);
	};
}
