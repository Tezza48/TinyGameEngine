#include "Universe.h"
#include <random>
#include "MeshActor.h"
#include <functional>
#include "SpaceGame.h"

using namespace DirectX;
using namespace TinyEngine;

Universe::Universe(SpaceGame* game, TinyEngine::Mesh* starMesh) : Actor(game), _starMesh(starMesh)
{

}

void Universe::Clear()
{
	while(_children.size())
	{
		delete _children.back();
		RemoveChild(_children.back());
	}
}

void Universe::Generate(DirectX::XMFLOAT3 position)
{
	Clear();

	_lastPosition = position;

	std::uniform_real_distribution<float> normDist(0.0f, 1.0f);

	const int size = 16;
	const int halfSize = size / 2;

	for (int z = -halfSize; z < halfSize; z++)
	{
		for (int y = -halfSize; y < halfSize; y++)
		{
			for (int x = -halfSize; x < halfSize; x++)
			{
				XMFLOAT3 quadrantPos;
				quadrantPos.x = (x + position.x);
				quadrantPos.y = (y + position.y);
				quadrantPos.z = (z + position.z);

				std::mt19937 generator(hashVector3({ (int)quadrantPos.x, (int)quadrantPos.y, (int)quadrantPos.z }));
				auto random = std::bind(normDist, generator);

				if (0.1f < random())
				{
					continue;
				}

				// TODO WT: Meshes and materials should not exist together cos it makes this stuff confusing and inefficent.
				// Suggesting either a MeshRenderer component class or the MeshActor maintains a list of material and the mesh itself.

				// Grab a copy of the mesh, will have the same internal buffers
				//auto meshCpy = new Mesh(*_starMesh);
				//_meshInstances.push_back(meshCpy);
				//
				//// Copy the mesh's material
				//auto mat = new Material(*meshCpy->GetPartMaterial(0));
				//// Set the mat's ambient to a random color
				//mat->ambient = {
				//	random(),
				//	random(),
				//	random()
				//};
				//_materialInstances.push_back(mat);
				//
				//meshCpy->SetPartMaterial(0, mat);

				auto newPlanet = new MeshActor(_game);
				newPlanet->SetMesh(_starMesh);
				newPlanet->SetParent(this);

				XMFLOAT3 realPos = {
					x + position.x + random() - 0.5f,
					y + position.y + random() - 0.5f,
					z + position.z + random() - 0.5f
				};

				newPlanet->SetPosition(realPos);

				auto scale = 0.1f;//  *random();

				newPlanet->SetScale({ scale, scale, scale });
			}
		}
	}
}

void Universe::OnDraw(TinyEngine::Renderer* renderer)
{
	// Batch render the stars.

	Actor::OnDraw(renderer);
}

void Universe::OnUpdate(float elapsed, float delta)
{
	const auto& cam = _game->_activeCamera;
	const auto& pos = cam->GetEyePosition();

	XMFLOAT3 centre = { floorf(pos.x), floorf(pos.y), floorf(pos.z) };
	float length;
	XMStoreFloat(&length, XMVector3Length(XMLoadFloat3(&centre) - XMLoadFloat3(&_lastPosition)));

	if (length > 1.0f)
	{
		Generate(centre);
	}

	Actor::OnUpdate(elapsed, delta);
}

int Universe::hashVector3(DirectX::XMINT3 seed)
{
	return (seed.x << 16) + (seed.y << 8) + seed.z;
}
