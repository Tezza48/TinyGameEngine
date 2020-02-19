#include "DefaultShader.hlsli"

float4 main(PS_IN i) : SV_TARGET
{
	return AmbientTexture.Sample(DefaultSampler, i.texcoord);
}