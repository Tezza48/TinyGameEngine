#include "MeshActor.h"
#include "SpaceGame.h"

void MeshActor::SetMesh(TinyEngine::Mesh* mesh)
{
	_mesh = mesh;
}

void MeshActor::OnDraw(TinyEngine::Renderer* renderer)
{
	if (_mesh)
	{
		renderer->DrawMesh(_mesh, _game->_activeCamera, GetWorld());
	}

	Actor::OnDraw(renderer);
}
