#include "DefaultShader.hlsli"

float3 BlinnPhong(DirectionLight light, float3 normal, float3 toEye, Material mat)
{
	float3 lightVec = -normalize(light.Direction);

	float diffuseFactor = saturate(dot(normal, lightVec));

	float lightColor = light.Color.xyz * light.Color.w;

	float3 diffuse = diffuseFactor * mat.Diffuse;// *lightColor;

	float3 h = normalize(lightVec + toEye);

	float nDotH = dot(normal, h);
	float3 specFactor = pow(saturate(nDotH), mat.SpecularExponent);

	float3 specular = specFactor * mat.Specular * lightColor;

	return diffuse + specular;
}

float4 main(PS_IN i) : SV_TARGET
{
	float3 color = float3(0.0, 0.0, 0.0);
	float3 toEye = normalize(EyePositionW - i.positionW);

	float4 specTex = SpecularTexture.Sample(DefaultSampler, i.texcoord);
	float3 diffuseTex = DiffuseTexture.Sample(DefaultSampler, i.texcoord).rgb;

	Material mat = Mat;
	mat.Diffuse += diffuseTex;
	mat.Specular += specTex.rgb;
	mat.SpecularExponent += (specTex.a * 500.0);

	for (int j = 0; j < 3; j++)
	{
		 color += BlinnPhong(DirectionLights[j], normalize(i.normalW), toEye, mat);
	}

	color += mat.Ambient * AmbientLight.rgb * AmbientLight.a;

	return float4(color, 1.0); // Gamma correction
}