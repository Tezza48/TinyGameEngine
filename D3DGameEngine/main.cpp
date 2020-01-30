#define TINY_ENGINE_IMPLEMENTATION
#include "TinyEngine.h"

class Game
	: public TinyEngine
{
public:
	Game() : TinyEngine(1280, 720, "Game") {

	}

	void OnInit() override
	{

	}

	bool OnUpdate() override
	{
		return true;
	}
};

int main(int argc, char** argv)
{
	Game game;
	game.Run();
}