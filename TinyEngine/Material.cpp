#include "Material.h"
#include <algorithm>

using namespace TinyEngine;

Material::Material()
{

}

Material::Material(const Material& material)
{
	shader = material.shader;

	ambient = material.ambient;
	ambientTexture = material.ambientTexture;

	diffuse = material.diffuse;
	diffuseTexture = material.diffuseTexture;

	specular = material.specular;
	specularExponent = material.specularExponent;
	specularTexture = material.specularTexture;

	transparency = material.transparency;
}

TinyEngine::Material::~Material()
{

}
