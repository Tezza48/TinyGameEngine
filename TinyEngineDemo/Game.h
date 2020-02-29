#pragma once

#include "TinyEngineGame.h"
#include <vector>
#include <unordered_map>
#include "Input.h"
#include <string>
#include "Mesh.h"
#include "Texture.h"
#include "Material.h"

// OBJL is a bit rubbish, since i cant indlude the header i've prototyped what i need here.
namespace objl
{
	struct Material;
}

struct Rect
{
	float left = 0, top = 0, right = 0, bottom = 0;

	static inline bool Intersect(const Rect& a, const Rect& b);
};

struct Object
{
	DirectX::XMFLOAT2 position = {};
	float width = 1.0f;
	float height = 1.0f;
	TinyEngine::Mesh* mesh = nullptr;
	std::vector<TinyEngine::Material*> materials;

	Rect GetBounds();

	Object() {}
	Object(TinyEngine::Mesh* mesh, std::vector<TinyEngine::Material*> materials);
};

struct Camera : public TinyEngine::ICamera, public TinyEngine::IObserver
{
public:
	DirectX::XMFLOAT3 position = {};

private:
	float _aspectRatio = 1.0f;

public:
	void SetAspectRatio(float aspectRatio);

	// Inherited via ICamera
	virtual DirectX::XMFLOAT3 GetEyePosition() override;
	virtual DirectX::XMMATRIX GetView() override;
	virtual DirectX::XMMATRIX GetProjection() override;

	// Inherited via IObserver
	virtual void OnNotify(const TinyEngine::Event& event) override;
};

class Game :
	public TinyEngine::TinyEngineGame
{
public:
	struct MeshAsset
	{
		TinyEngine::Mesh* mesh;
		std::vector<TinyEngine::Material*> materials;
	};

private:
	// Game
	Input _inputHandler;

	Camera _camera;

	DirectX::XMFLOAT3 _ballDirection;

	Object _leftPaddle;
	Object _rightPaddle;
	Object _ball;

public:
	// Asset manager?
	// TODO WT: should all be maps so assets can be requested by name, far easier to work with.
	std::vector<MeshAsset> _meshes;
	std::unordered_map<std::string, TinyEngine::Texture*> _textures;
	TinyEngine::Texture* _nullTexture;

public:
	Game(int width, int height, const char* title);
	~Game();

	TinyEngine::Texture* LoadTexture(const char* path);

	MeshAsset LoadMesh(const char* path);

	TinyEngine::Material* ConvertMaterial(const char* path, const objl::Material& objlMat);

	// Inherited via Game
	virtual void OnInit() override;

	virtual void OnUpdate(float elapsed, float delta) override;

	virtual Input* GetInput() const override;

private:
	void ShootBall();
};