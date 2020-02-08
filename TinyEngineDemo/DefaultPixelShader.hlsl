cbuffer CBMaterial : register(b1)
{
	float3 Mat_Ambient;
	float _pad0;
	float3 Mat_Diffuse;
	float _pad1;
	float3 Mat_Specular;
	float _pad2;
	float Mat_SpecularExponent;
	float Mat_Transparency;
	float Mat__pad[2];
};

cbuffer CbPerObject : register(b0)
{
	float4x4 World;
	float4x4 View;
	float4x4 Projection;
};

struct VS_OUT
{ 
	float4 position: SV_POSITION; 
	float3 normal: NORMAL;
};

float4 main(VS_OUT i) : SV_TARGET
{
	float nDotL = dot(i.normal, normalize(float3(1.0f, 0.1f, 0.0f)));
	return float4(Mat_Diffuse.rgb * nDotL, 1.0);
}