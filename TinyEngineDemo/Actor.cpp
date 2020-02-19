#include "Actor.h"

using namespace DirectX;

Actor::Actor(SpaceGame* game) : _game(game)
{

}

Actor::~Actor()
{
	for (auto& child : _children)
	{
		delete child;
	}
}

DirectX::XMFLOAT3 Actor::GetPosition() const
{
	return _position;
}

void Actor::SetPosition(DirectX::XMFLOAT3 position)
{
	_position = position;
}

XMMATRIX Actor::GetWorld() const
{
	XMMATRIX thisWorld = GetLocalTransform();

	if (!_parent)
	{
		return thisWorld;
	}
	else
	{
		return _parent->GetWorld() * thisWorld;
	}
}

DirectX::XMMATRIX Actor::GetLocalTransform() const
{
	return XMMatrixTransformation({}, {}, XMLoadFloat3(&_scale), {}, XMLoadFloat4(&_orientation), XMLoadFloat3(&_position));
}

void Actor::AddChild(Actor* child)
{
	_children.push_back(child);
}

void Actor::RemoveChild(Actor* child)
{
	_children.remove(child);
}

void Actor::SetParent(Actor* parent)
{
	this->_parent = parent;
	parent->AddChild(this);
}

void Actor::OnUpdate(float elapsed, float delta)
{
	for (auto& child : _children)
	{
		child->OnUpdate(elapsed, delta);
	}
}

void Actor::OnDraw(TinyEngine::Renderer* renderer)
{
	for (auto& child : _children)
	{
		child->OnDraw(renderer);
	}
}
