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

struct VS_IN
{ 
	float3 position: POSITION; 
	float2 texcoord: TEXCOORD; 
	float3 normal: NORMAL; 
};

struct VS_OUT 
{ 
	float4 position: SV_POSITION; 
	float3 normal: NORMAL; 
};

VS_OUT main(VS_IN i)
{
	VS_OUT o;
	o.normal = i.normal;

	float4x4 wvp = Projection * View * World;

	float4 positionL = float4(i.position, 1.0);

	o.position = mul(positionL, World);
	o.position = mul(o.position, View);
	o.position = mul(o.position, Projection);

	return o;
}