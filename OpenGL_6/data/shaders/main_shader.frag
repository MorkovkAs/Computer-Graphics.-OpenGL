#version 330

smooth in vec2 vTexCoord;
smooth in vec3 vNormal;
smooth in vec4 vEyeSpacePos;
smooth in vec3 vWorldPos;
out vec4 outputColor;

uniform sampler2D gSampler;
uniform vec4 vColor;

#include "dirLight.frag"
#include "pointLight.frag"
uniform DirectionalLight sunLight;
uniform PointLight pointLight1;
uniform PointLight pointLight2;
uniform PointLight pointLight3;

#include "shadows.frag"

smooth in vec4 ShadowCoord;
uniform sampler2DShadow  shadowMap;
//uniform sampler2D  shadowMap;

void main()
{
	vec3 vNormalized = normalize(vNormal);
	
	vec4 vTexColor = texture2D(gSampler, vTexCoord);
	vec4 vMixedColor = vTexColor*vColor;
	
	float visibility = GetVisibility(shadowMap, ShadowCoord);
	vec4 vDiffuseColor = getDirectionalLightColor(sunLight, vNormalized, visibility);
	vec4 vPointlightColor1 = getPointLightColor(pointLight1, vWorldPos, vNormalized);
	vec4 vPointlightColor2 = getPointLightColor(pointLight2, vWorldPos, vNormalized);
	vec4 vPointlightColor3 = getPointLightColor(pointLight3, vWorldPos, vNormalized);
	
	outputColor = vMixedColor*(vDiffuseColor + vPointlightColor1 + vPointlightColor2 + vPointlightColor3);
}