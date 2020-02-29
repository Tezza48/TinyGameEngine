#include "Game.h"

#pragma warning(disable: 4067 4244 4018)
#include "vendor/OBJ_Loader.h";
#pragma warning(default: 4067 4244 4018)

#define STB_IMAGE_IMPLEMENTATION
#define STBI_FAILURE_USERMSG
#include "vendor\stb_image.h"

#include <iostream>
#include <DirectXMath.h>
#include <filesystem>
#include <algorithm>

using namespace DirectX;
using namespace TinyEngine;

using std::cout;
using std::endl;
using std::vector;
using std::clamp;

Game::Game(int width, int height, const char* title) : TinyEngine::TinyEngineGame(width, height, title)
{
	SetInputHandler(&_inputHandler);

	_nullTexture = new Texture(GetRenderer());
}

Game::~Game()
{
	delete _nullTexture;
	_nullTexture = nullptr;

	for (auto mesh : _meshes)
	{
		delete mesh.mesh;
		
		for (auto mat : mesh.materials)
		{
			delete mat;
		}
	}

	for (auto textureKeyVal : _textures)
	{
		delete textureKeyVal.second;
		textureKeyVal.second = nullptr;
	}
}

Texture* Game::LoadTexture(const char* path)
{
	if (_textures[path])
	{
		return _textures[path];
	}

	int w, h, bpp;
	unsigned char* texData = stbi_load(path, &w, &h, &bpp, STBI_rgb_alpha);

	auto failure = stbi_failure_reason();
	if (failure)
	{
		cout << "STB Failed to load Image: " << failure << endl;
	}

	Texture* tex = new Texture(GetRenderer(), texData, w, h);
	_textures[path] = tex;

	stbi_image_free(texData);

	return tex;
}

Game::MeshAsset Game::LoadMesh(const char* path)
{
	objl::Loader loader;

	auto* renderer = GetRenderer();

	if (loader.LoadFile(path))
	{
		vector<VertexStandard> vertices;

		MeshAsset asset;
		asset.mesh = new Mesh(renderer);

		for (objl::Mesh& meshData : loader.LoadedMeshes)
		{
			auto base = vertices.size();
			for (const auto& objlVertex : meshData.Vertices)
			{
				XMFLOAT3 pos(&objlVertex.Position.X);
				XMFLOAT2 tex(&objlVertex.TextureCoordinate.X);
				XMFLOAT3 norm(&objlVertex.Normal.X);

				vertices.emplace_back(pos, tex, norm);
			}

			const auto& objlMat = meshData.MeshMaterial;

			auto* mat = ConvertMaterial(path, objlMat);

			asset.materials.push_back(mat);
			asset.mesh->AddIndexBuffer(meshData.Indices.data(), static_cast<unsigned int>(meshData.Indices.size()), static_cast<unsigned int>(base));
		}

		asset.mesh->SetVertices(vertices.data(), static_cast<unsigned int>(vertices.size()));
		_meshes.push_back(asset);

		return asset;
	}
	else
	{
		cout << "Could not load mesh: " << path << endl;

		return MeshAsset();
	}
}

Material* Game::ConvertMaterial(const char* path, const objl::Material& objlMat)
{
	auto* mat = new Material{};
	mat->transparency = objlMat.d;

	if (objlMat.map_Ka != "")
	{
		std::filesystem::path texPath(path);
		texPath.remove_filename().append(objlMat.map_Ka);
		mat->ambientTexture = LoadTexture(texPath.string().c_str());
	}
	else
	{
		mat->ambientTexture = _nullTexture;
		mat->ambient = XMFLOAT3(&objlMat.Ka.X);
	}

	if (objlMat.map_Kd != "")
	{
		std::filesystem::path texPath(path);
		texPath.remove_filename().append(objlMat.map_Kd);
		mat->diffuseTexture = LoadTexture(texPath.string().c_str());
	}
	else
	{
		mat->diffuseTexture = _nullTexture;
		mat->diffuse = XMFLOAT3(&objlMat.Kd.X);
	}

	if (objlMat.map_Ks != "")
	{
		std::filesystem::path texPath(path);
		texPath.remove_filename().append(objlMat.map_Ks);
		mat->specularTexture = LoadTexture(texPath.string().c_str());
	}
	else
	{
		mat->specularTexture = _nullTexture;
		mat->specular = XMFLOAT3(&objlMat.Ks.X);
		mat->specularExponent = objlMat.Ns;
	}

	return mat;
}

// Inherited via Game

void Game::OnInit()
{
	auto renderer = GetRenderer();

	_camera.position = { 0.0f, 0.0f, -50.0f };
	_camera.SetAspectRatio(static_cast<float>(this->GetWidth()) / static_cast<float>(this->GetHeight()));
	AddObserver(_camera);

	auto sphereMesh = LoadMesh("./assets/mesh/sphere_1u.obj");
	auto cubeMesh = LoadMesh("./assets/mesh/cube_1u.obj");

	_ball.mesh = sphereMesh.mesh;
	_ball.materials = sphereMesh.materials;
	_ball.position = { 0.0f, 0.0f };
	_ball.width = 1.0f;
	_ball.height = 1.0f;

	float paddleDistance = 30.0f;

	_leftPaddle.mesh = cubeMesh.mesh;
	_leftPaddle.materials = cubeMesh.materials;
	_leftPaddle.position = { -paddleDistance, 0.0f };
	_leftPaddle.width = 1.0f;
	_leftPaddle.height = 4.0f;

	_rightPaddle.mesh = cubeMesh.mesh;
	_rightPaddle.materials = cubeMesh.materials;
	_rightPaddle.position = { paddleDistance, 0.0f };
	_rightPaddle.width = 1.0f;
	_rightPaddle.height = 4.0f;

	ShootBall();

	XMStoreFloat3(&renderer->lights[0].direction, XMVector3Normalize(XMVectorSet(-1.0f, 0.0f, 1.0f, 0.0f)));
	renderer->lights[0].color = { 1.0, 1.0, 1.0, 1.0f };

	XMStoreFloat3(&renderer->lights[1].direction, XMVector3Normalize(XMVectorSet(1.0f, 0.0f, 1.0f, 0.0f)));
	renderer->lights[1].color = { 0.0, 1.0, 1.0, 1.0f };

	XMStoreFloat3(&renderer->lights[2].direction, XMVector3Normalize(XMVectorSet(-1.0f, -2.0f, -1.0f, 0.0f)));
	renderer->lights[2].color = { 1.0, 0.0, 1.0, 1.0f };

	renderer->ambientLight = { 0.1f, 0.1f, 0.2f, 0.5f };
	renderer->SetClearColor({ 0.1f, 0.1f, 0.2f, 1.0f });
}

void UpdatePaddle(Input& input, Object& paddle, Key upKey, Key downKey, float delta)
{
	const float moveSpeed = 25.0f;
	const float maxVertical = 15.0f;
	float move = 0.0f;

	if (input.GetKey(upKey))
	{
		move++;
	}

	if (input.GetKey(downKey))
	{
		move--;
	}

	paddle.position.y += move * moveSpeed * delta;

	paddle.position.y = clamp(paddle.position.y, -maxVertical, maxVertical);
}

void Game::OnUpdate(float elapsed, float delta)
{
	auto input = GetInput();

	UpdatePaddle(*input, _leftPaddle, Key::W, Key::S, delta);
	UpdatePaddle(*input, _rightPaddle, Key::I, Key::K, delta);

	const float speed = 25.0f;
	XMVECTOR vecDirection = XMVector3Normalize(XMLoadFloat3(&_ballDirection));

	XMFLOAT2 ballPos = _ball.position;

	float verticalBound = 17.0f;

	if (verticalBound - ballPos.y < 0) {
		vecDirection = XMVector3Normalize(XMVector3Reflect(vecDirection, XMVectorSet(0.0f, -1.0f, 0.0f, 0.0f)));
	}

	if (-verticalBound - ballPos.y > 0) {
		vecDirection = XMVector3Normalize(XMVector3Reflect(vecDirection, XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f)));
	}

	if (input->GetKey(Key::SPACE)) {
		__debugbreak();
	}

	auto vecBallPos = XMLoadFloat2(&_ball.position);

	if (Rect::Intersect(_ball.GetBounds(), _leftPaddle.GetBounds()))
	{
		auto vecPaddlePos = XMLoadFloat2(&_leftPaddle.position);
		vecDirection = XMVector2Normalize(vecBallPos - vecPaddlePos);
	}

	if (Rect::Intersect(_ball.GetBounds(), _rightPaddle.GetBounds()))
	{
		auto vecPaddlePos = XMLoadFloat2(&_rightPaddle.position);
		vecDirection = XMVector2Normalize(vecBallPos - vecPaddlePos);
	}

	XMStoreFloat3(&_ballDirection, vecDirection);

	auto vecPos = XMLoadFloat2(&_ball.position);

	vecPos += vecDirection * speed * delta;

	XMStoreFloat2(&_ball.position, vecPos);

	if (ballPos.x > 34.0f || ballPos.x < -34.0f)
	{
		ShootBall();
	}

	auto renderer = GetRenderer();

	auto world = XMMatrixScaling(_ball.width, _ball.height, 1.0f) * XMMatrixTranslation(_ball.position.x, _ball.position.y, 0.0f);
	renderer->DrawMesh(_ball.mesh, _ball.materials, &_camera, world);

	world = XMMatrixScaling(_leftPaddle.width, _leftPaddle.height, 1.0f) * XMMatrixTranslation(_leftPaddle.position.x, _leftPaddle.position.y, 0.0f);
	renderer->DrawMesh(_leftPaddle.mesh, _leftPaddle.materials, &_camera, world);

	world = XMMatrixScaling(_rightPaddle.width, _rightPaddle.height, 1.0f) * XMMatrixTranslation(_rightPaddle.position.x, _rightPaddle.position.y, 0.0f);
	renderer->DrawMesh(_rightPaddle.mesh, _rightPaddle.materials, &_camera, world);
}

Input* Game::GetInput() const
{
	return static_cast<Input*>(TinyEngineGame::GetInput());
}

void Game::ShootBall()
{
	_ball.position = {};
	_ballDirection =  { (rand() > (RAND_MAX / 2)) ? -1.0f : 1.0f, 0.0f, 0.0f};
}

inline bool Rect::Intersect(const Rect& a, const Rect& b)
{
	return (a.left < b.right) && (a.right > b.left) && (a.top > b.bottom) && (a.bottom < b.top);
}

Rect Object::GetBounds()
{
	auto w = width / 2;
	auto h = height / 2;

	return {
		position.x - w,
		position.y + h,
		position.x + w,
		position.y - h,
	};
}

Object::Object(Mesh* mesh, std::vector<Material*> materials): mesh(mesh), materials()
{

}

void Camera::SetAspectRatio(float aspectRatio)
{
	_aspectRatio = aspectRatio;
}

DirectX::XMFLOAT3 Camera::GetEyePosition()
{
	return position;
}

DirectX::XMMATRIX Camera::GetView()
{
	return XMMatrixLookAtLH(XMLoadFloat3(&position), XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f), XMVectorSet(0.0f, 1.0f, 0.0f, 0.0f));
}

DirectX::XMMATRIX Camera::GetProjection()
{
	return XMMatrixPerspectiveFovLH(XM_PIDIV4, _aspectRatio, 0.01f, 1000.0f);
}

void Camera::OnNotify(const TinyEngine::Event& event)
{
	switch (static_cast<EngineEventType>(event.GetType()))
	{
	case EngineEventType::WINDOW_RESIZE:
	{
		const auto& resizeEvent = static_cast<const ResizeEvent&>(event);
		SetAspectRatio(static_cast<float>(resizeEvent.x) / static_cast<float>(resizeEvent.y));

		break;
	}
	default:
		break;
	}
}
