#include "DefaultShader.hlsli"

float3 BlinnPhong(Light light, float3 normal, float3 toEye, Material mat)
{
	light.Position = normalize(light.Position);
	float nDotL = dot(normal, light.Position);
	float intensity = saturate(nDotL);

	float3 diffuse = intensity * mat.Diffuse * light.Color.xyz * light.Color.w;

	float3 h = normalize(light.Position + toEye);

	float nDotH = dot(normal, h);
	intensity = pow(saturate(nDotH), mat.SpecularExponent);

	float3 specular = intensity * mat.Specular * light.Color.xyz * light.Color.w;

	return diffuse + specular;
}

float4 main(PS_IN i) : SV_TARGET
{
	float3 color;
	float3 lightDir = -normalize(float3(0.0f, -1.0f, 1.0f));
	float3 toEye = normalize(EyePositionW - i.positionW);

	for (int j = 0; j < 3; j++)
	{
		color += BlinnPhong(SceneLights[j], normalize(i.normalW), toEye, Mat);
	}


	return float4(pow(color, 1/2.2), 1.0); // Gamma correction
}