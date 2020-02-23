#include "Universe.h"
#include <random>
#include "MeshActor.h"
#include <functional>
#include "SpaceGame.h"
#include "Renderer.h"

using namespace DirectX;
using namespace TinyEngine;

Universe::Universe(SpaceGame* game, Mesh* starMesh, Material* material) : Actor(game), _starMesh(starMesh), _baseMaterial(material)
{
	_materialInstances[0] = new Material(*material);
	_materialInstances[1] = new Material(*material);
	_materialInstances[2] = new Material(*material);
	_materialInstances[3] = new Material(*material);
	_materialInstances[4] = new Material(*material);
	_materialInstances[5] = new Material(*material);
	_materialInstances[6] = new Material(*material);

	_materialInstances[0]->ambient = { 1.0f, 0.0f, 0.0f };
	_materialInstances[1]->ambient = { 0.0f, 1.0f, 0.0f };
	_materialInstances[2]->ambient = { 0.0f, 0.0f, 1.0f };
	_materialInstances[3]->ambient = { 1.0f, 1.0f, 0.0f };
	_materialInstances[4]->ambient = { 0.0f, 1.0f, 1.0f };
	_materialInstances[5]->ambient = { 1.0f, 0.0f, 1.0f };
	_materialInstances[6]->ambient = { 1.0f, 1.0f, 1.0f };
}

Universe::~Universe()
{
	for (auto& material : _materialInstances)
	{
		delete material;
	}
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
	std::uniform_int_distribution<int> materialDist(0, NUM_MATERIAL_INSTANCES - 1);

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

				auto newPlanet = new MeshActor(_game);
				newPlanet->SetMesh(_starMesh);
				newPlanet->SetParent(this);
				auto materialId = materialDist(generator);
				newPlanet->SetMaterials({ _materialInstances[materialId] });

				XMFLOAT3 realPos = {
					x + position.x + random() - 0.5f,
					y + position.y + random() - 0.5f,
					z + position.z + random() - 0.5f
				};

				newPlanet->SetPosition(realPos);

				auto scale = 0.1f * random();

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
