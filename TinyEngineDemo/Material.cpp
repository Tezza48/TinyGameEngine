#include "Material.h"

TinyEngine::Material::Material()
{
	ambient = {};
	diffuse = {};
	specular = {};
	specularExponent = {};
	transparency = {};
	//diffuseTexture = nullptr;
}

TinyEngine::Material::Material(DirectX::XMFLOAT3 ambient, DirectX::XMFLOAT3 diffuse, DirectX::XMFLOAT3 specular, float specularExponent, float transparency)
{
	this->ambient = ambient;
	this->diffuse = diffuse;
	this->specular = specular;
	this->specularExponent = specularExponent;
	this->transparency = transparency;
	//this->diffuseTexture = diffuseTexture;
}
