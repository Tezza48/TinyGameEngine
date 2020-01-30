#define TINY_ENGINE_IMPLEMENTATION
#define TINY_ENGINE_LAZY_LIBS

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