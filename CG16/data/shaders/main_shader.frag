#version 330

smooth in vec2 vTexCoord;
smooth in vec3 vNormal;
smooth in vec4 vEyeSpacePos;
smooth in vec3 vWorldPos;
out vec4 outputColor;

uniform sampler2D gSampler;
uniform vec4 vColor;

#include "dirLight.frag"

uniform DirectionalLight sunLight;
uniform vec3 vEyePosition;

#include "pointLight.frag"
uniform PointLight pointLight1;

uniform Material matActive;

uniform struct FogParameters
{
	vec4 vFogColor; // Fog color
	float fStart; // This is only for linear fog
	float fEnd; // This is only for linear fog
	float fDensity; // For exp and exp2 equation
	bool showFog;
	
	int iEquation; // 0 = linear, 1 = exp, 2 = exp2
} fogParams;

float getFogFactor(FogParameters params, float fFogCoord)
{
	float fResult = 0.0;
	if(params.iEquation == 0)
		fResult = (params.fEnd-fFogCoord)/(params.fEnd-params.fStart);
	else if(params.iEquation == 1)
		fResult = exp(-params.fDensity*fFogCoord);
	else if(params.iEquation == 2)
		fResult = exp(-pow(params.fDensity*fFogCoord, 2.0));
		
	fResult = 1.0-clamp(fResult, 0.0, 1.0);
	
	return fResult;
}

void main()
{
   vec4 vTexColor = texture2D(gSampler, vTexCoord);
   vec4 vMixedColor = vTexColor*vColor;
   
   vec3 vNormalized = normalize(vNormal);

   vec4 vDiffuseColor = GetDirectionalLightColor(sunLight, vNormalized);
   vec4 vSpecularColor = GetSpecularColor(vWorldPos, vEyePosition, matActive, sunLight, vNormalized);
   vec4 vPointlightColor1 = getPointLightColor(pointLight1, vWorldPos, vNormalized);
   
   vec4 vMixedColorResult = vMixedColor*(vDiffuseColor+vSpecularColor+vPointlightColor1);

   if (fogParams.showFog) {
		float fFogCoord = abs(vEyeSpacePos.z/vEyeSpacePos.w);
		outputColor = mix(vMixedColorResult, fogParams.vFogColor, getFogFactor(fogParams, fFogCoord));
	} else {
		outputColor = vMixedColorResult;
	}
}