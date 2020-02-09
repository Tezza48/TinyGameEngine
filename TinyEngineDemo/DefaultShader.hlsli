struct Material
{
	float3 Specular;
	float SpecularExponent;
	float3 Ambient;
	float Transparency;
	float3 Diffuse;
};

struct DirectionLight
{
	float4 Color;
	float3 Direction;
	float _pad;
};

cbuffer CBMaterial : register(b1)
{
	Material Mat;
	float _pad0;
};

cbuffer CbPerObject : register(b0)
{
	DirectionLight DirectionLights[3];
	float4 AmbientLight;
	float4x4 World;
	float4x4 WorldInverseTranspose;
	float4x4 View;
	float4x4 Projection;
	float3 EyePositionW;
	float _pad;
};

struct VS_IN
{
	float3 positionL: POSITION;
	float2 texcoord: TEXCOORD;
	float3 normalL: NORMAL;
};

typedef struct VS_OUT
{
	float4 positionH: SV_POSITION;
	float3 positionW: POSITION;
	float3 normalW: NORMAL;
	float2 texcoord: TEXCOORD;
} PS_IN;