#include "DefaultShader.hlsli"

Texture2D DiffuseTexture;

SamplerState DefaultSampler : register(s0);

float3 BlinnPhong(DirectionLight light, float3 normal, float3 toEye, Material mat)
{
	light.Direction = normalize(light.Direction);
	float nDotL = dot(normal, light.Direction);
	float intensity = saturate(nDotL);

	float lightColor = light.Color.xyz * light.Color.w;

	float3 diffuse = intensity * mat.Diffuse;// *lightColor;

	//float3 h = normalize(-light.Direction + toEye);

	//float nDotH = dot(normal, h);
	//intensity = pow(saturate(nDotH), mat.SpecularExponent);

	//float3 specular = intensity * mat.Specular * light.Color.xyz * light.Color.w;

	return diffuse;// +specular;
}

float4 main(PS_IN i) : SV_TARGET
{
	float3 color;
	float3 toEye = normalize(EyePositionW - i.positionW);

	Material mat = Mat;
	mat.Diffuse += DiffuseTexture.Sample(DefaultSampler, i.texcoord).rgb;

	for (int j = 0; j < 3; j++)
	{
		color += BlinnPhong(DirectionLights[j], normalize(i.normalW), toEye, mat);
	}

	color += mat.Diffuse * AmbientLight.rgb * AmbientLight.a;

	return float4(color, 1.0); // Gamma correction
}