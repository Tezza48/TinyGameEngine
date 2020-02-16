#include "Game.h"
#include "EngineEventType.h"
#include <iostream>

using std::cout;
using std::endl;

using namespace std::chrono;

TinyEngine::Game::Game(int width, int height, const char* title) :
	_width(width), _height(height), _input(nullptr), _isRunning(false)
{
	_window = new Window(width, height, title);
	_renderer = new Renderer(width, height, *_window);

	_window->AddObserver(*this);
	_window->AddObserver(*_renderer);
}

TinyEngine::Game::~Game()
{
	delete _window;
	_window = nullptr;

	delete _renderer;
	_renderer = nullptr;

	if (_input)
	{
		delete _input;
		_input = nullptr;
	}
}

void TinyEngine::Game::Run()
{
	_isRunning = true;

	_startTime = high_resolution_clock::now();
	auto lastTime = high_resolution_clock::now();

	OnInit();

	while (_isRunning)
	{
		_window->PeekMessages();

		_renderer->Clear();

		auto thisTime = high_resolution_clock::now();

		auto elapsed = static_cast<float>(duration_cast<milliseconds>(thisTime - _startTime).count()) / 1000.0f;
		auto delta = static_cast<float>(duration_cast<milliseconds>(thisTime - lastTime).count()) / 1000.0f;

		// TODO: Delta Time.
		OnUpdate(elapsed, delta);

		lastTime = thisTime;

		_renderer->SwapBuffers();
	}
}

void TinyEngine::Game::SetInputHandler(BaseInput* input)
{
	_input = input;

	_window->AddObserver(*_input);
}

int TinyEngine::Game::GetWidth() const
{
	return _width;
}

int TinyEngine::Game::GetHeight() const
{
	return _height;
}

void TinyEngine::Game::OnNotify(const Event& event)
{
	const auto type = static_cast<EngineEventType>(event.GetType());

	switch (type)
	{
	case EngineEventType::WINDOW_CREATED:
		break;

	case EngineEventType::WINDOW_CLOSE:
		_isRunning = false;

		break;

	case EngineEventType::WINDOW_RESIZE:
		const auto& resizeEvent = static_cast<const ResizeEvent&>(event);
		OnResize(resizeEvent.x, resizeEvent.y);

		Notify(ResizeEvent(resizeEvent));

		break;
	}
}

TinyEngine::Renderer* TinyEngine::Game::GetRenderer() const
{
	return _renderer;
}

void TinyEngine::Game::OnResize(int width, int height)
{
	cout << "width " << width << ", height " << height << endl;
}
