#include "MeshActor.h"
#include "SpaceGame.h"

void MeshActor::SetMesh(TinyEngine::Mesh* mesh)
{
	_mesh = mesh;
}

void MeshActor::SetMaterials(std::vector<TinyEngine::Material*> materials)
{
	_materials = materials;
}

void MeshActor::OnDraw(TinyEngine::Renderer* renderer)
{
	if (_mesh)
	{
		renderer->DrawMesh(_mesh, _materials, _game->_activeCamera, GetWorld());
	}

	Actor::OnDraw(renderer);
}
