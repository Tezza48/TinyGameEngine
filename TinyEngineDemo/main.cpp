#include <DirectXMath.h>
#include <filesystem>
#include <unordered_map>
#include <iostream>
#include <string>
#include "Game.h"

#ifdef TINY_ENGINE_EXPOSE_NATIVE
#error "TINY_ENGINE_EXPOSE_NATIVE has leaked to main."
#endif

using std::string;
using std::cout;
using std::endl;

using namespace DirectX;

int main(int argc, char** argv)
{
	for (auto i = 0, l = argc - 1; i < l; i++)
	{
		if (std::string(argv[i]) == "/r") {
			std::cout << "Setting Working Directory to: " << argv[i + 1] << std::endl;
			SetCurrentDirectory(argv[i + 1]);
			break;
		}
	}

	auto game = Game(1600, 900, "Game");
	game.Run();

	return 0;
}
	