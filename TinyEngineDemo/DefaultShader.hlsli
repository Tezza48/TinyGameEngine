struct Material
{
	float3 Specular;
	float SpecularExponent;
	float3 Ambient;
	float Transparency;
	float3 Diffuse;
};

struct Light
{
	float4 Color;
	float3 Position;
	float _pad;
};

cbuffer CBMaterial : register(b1)
{
	Material Mat;
	float _pad0;
};

cbuffer CbPerObject : register(b0)
{
	Light SceneLights[3];
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
} PS_IN;