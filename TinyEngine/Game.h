#pragma once

#include "Renderer.h"
#include "Window.h"
#include "BaseInput.h"
#include <chrono>

namespace TinyEngine
{
	// The base class for a TinyEngine Game.
	class Game :
		public IObserver, public Subject
	{
	public:
	protected:
		// Is the game running? The game will exit if
		// this is false at the start of the game loop.
		bool _isRunning;

	private:
		int _width;
		int _height;

		Window* _window;
		BaseInput* _input;
		Renderer* _renderer;

		BaseInput _nullInput;

		std::chrono::high_resolution_clock::time_point _startTime;

	public:
		// Construct a new Game.
		//	int width: Game window's initial width
		//	int height: Game window's initial height
		//	const char* title: Game window's title text
		Game(int width, int height, const char* title);
		~Game();

		Game(const Game& game) = delete;

		// Get the game window's width.
		int GetWidth() const;

		// Get the game window's height.
		int GetHeight() const;

		// Run the game. Starts the game loop.
		void Run();

	protected:
		// Set the input handler and configure internal components.
		void SetInputHandler(BaseInput* input);

		// Get the game's Renderer.
		Renderer* GetRenderer() const;
		// Get the game's Window.
		Window* GetWindow() const;

		// Get the current input handler.
		virtual BaseInput* GetInput() const;

		// Called when the game has finished setup and before the game loop has started.
		virtual void OnInit() = 0;

		// Called every frame.
		//	float elapsed: Time elapsed since the game started
		//	float delta: Time since the last frame
		virtual void OnUpdate(float elapsed, float delta) = 0;

		// Inherited via Observer
		virtual void OnNotify(const Event& event) override;

	private:
		void OnResize(int width, int height);
	};
}
