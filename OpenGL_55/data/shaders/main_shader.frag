#version 330

smooth in vec2 vTexCoord;
smooth in vec3 vNormal;
smooth in vec4 vEyeSpacePos;
smooth in vec3 vWorldPos;
out vec4 outputColor;


#include "dirLight.frag"
#include "spotLight.frag"
#include "pointLight.frag"


uniform sampler2D gSamplers[2];
uniform float fTexture[2];
uniform vec4 vColor;
uniform int nT;
uniform SpotLight sLight;
uniform DirectionalLight dLight;
uniform PointLight pLight;
uniform vec3 vEyePosition;

uniform DirectionalLight sunLight;

uniform int Refl;

void main()
{
	vec3 vNormalized = normalize(vNormal);
	
	vec4 vTexColor1 = texture2D(gSamplers[0], vTexCoord);
	vec4 vTexColor2 = texture2D(gSamplers[1], vTexCoord);
	vec4 vMixedTexColor = vTexColor1*fTexture[0];
	if (nT > 1)
		 vMixedTexColor += vTexColor2*fTexture[1];
	vec4 vDirLightColor = getDirectionalLightColor(sunLight, vNormal);
	vec4 vDirSideColor = getDirectionalLightColor(dLight, vNormal);
	vec4 vStreetlightColor = GetSpotLightColor(sLight, vWorldPos);
	vec4 vMyPointlightColor = getPointLightColor(pLight, vWorldPos, vNormalized);
	
	// considering reflection
	vec4 vSpecularColor;
	if (Refl == 1) {
		vSpecularColor = GetSpecularColor(vWorldPos, vEyePosition, vNormalized, sunLight);
	} else {
		vSpecularColor = vec4(0.0, 0.0, 0.0, 0.0);
	}

	vec4 vMixedColor = vColor*vMixedTexColor*(vDirLightColor+vDirSideColor+vStreetlightColor+vMyPointlightColor+vSpecularColor);
 	outputColor = vMixedColor;
}