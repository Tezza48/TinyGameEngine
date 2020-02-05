#define TINY_ENGINE_IMPLEMENTATION
#define TINY_ENGINE_LAZY_LIBS

#include "TinyEngine.h"

using namespace TinyEngine;

class Game
	: public TinyEngineGame
{
private:
	Mesh* _testMesh;
public:
	Game() : TinyEngineGame(1280, 720, "Game"), _testMesh(nullptr)
	{

	}

	~Game()
	{
		if (_testMesh)
		{
			delete _testMesh;
			_testMesh = nullptr;
		}
	}

private:
	void OnInit() override
	{
		_testMesh = new Mesh("./assets/mesh/icosphere.obj");

		//VertexStandard vertices[3];
		//vertices[0].position = XMFLOAT3(-0.5f, -0.5f, 0.0f);
		//vertices[1].position = XMFLOAT3(0.0f, 0.5f, 0.0f);
		//vertices[2].position = XMFLOAT3(0.5f, -0.5f, 0.0f);

		//_testMesh->SetVertices(vertices, 3);
	}

	bool OnUpdate() override
	{
		DrawMesh(_testMesh);

		return true;
	}
};

int main(int argc, char** argv)
{
	Game game;
	game.Run();
}