#version 330 core

in vec3 vViewSpacePosition;
in vec3 vViewSpaceNormal;
in vec2 vTexCoords;

uniform vec3 uDirectionLightDir;
uniform vec3 uDirectionalLightIntensity;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;
uniform vec3 uKd;

uniform sampler2D uKdSampler;
uniform float uShininess;

uniform sampler2D uGPosition;
uniform sampler2D uGNormal;
uniform sampler2D uGAmbient;
uniform sampler2D uGDiffuse;
uniform sampler2D uGlossyShininess;

out vec3 fFragColor;

vec3 blinnPhong(vec3 normal, vec3 intensity, vec3 outDir, vec3 incDir){
	vec3 Kd = vec3(texelFetch(uGDiffuse, ivec2(gl_FragCoord.xy), 0)); 
	vec3 Ks = vec3(texelFetch(uGlossyShininess, ivec2(gl_FragCoord.xy), 0)); 

    vec3 halfVector = (outDir + incDir)/2;
    vec3 vec = intensity*((Kd*dot(incDir, normal)) + (Ks * pow(dot(halfVector, normal), uShininess)));
    return vec;
}

void main(){
	
	vec3 position = vec3(texelFetch(uGPosition, ivec2(gl_FragCoord.xy), 0));
	vec3 normal = vec3(texelFetch(uGNormal, ivec2(gl_FragCoord.xy), 0)); 

	float distToPointLight = length(uPointLightPosition -position);
	vec3 dirToPointLight = (uPointLightPosition - position) / distToPointLight;

	vec3 Ka = vec3(texelFetch(uGAmbient, ivec2(gl_FragCoord.xy), 0)); 

	fFragColor = blinnPhong(normalize(normal), uDirectionalLightIntensity, normalize(-position), uDirectionalLightDir) +
				 blinnPhong(normalize(normal), uPointLightIntensity / (distToPointLight * distToPointLight), normalize(-position), dirToPointLight)+Ka;
}