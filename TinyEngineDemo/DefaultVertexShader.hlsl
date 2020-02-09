#include "DefaultShader.hlsli"

VS_OUT main(VS_IN i)
{
	VS_OUT o;
	o.normalW = mul(float4(i.normalL, 1.0), WorldInverseTranspose).xyz;

	float4x4 wvp = Projection * View * World;

	float4 positionL = float4(i.positionL, 1.0);

	o.positionH = mul(positionL, World);
	o.positionH = mul(o.positionH, View);
	o.positionW = o.positionH.xyz;
	o.positionH = mul(o.positionH, Projection);
	o.texcoord = i.texcoord;

	return o;
}