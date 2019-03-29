#version 330 core

in vec3 vFragPositionVS;
in vec3 vFragNormalVS;
in vec2 vFragTex;

//out vec3 fFragColor;
layout(location = 0) out vec3 fPosition;
layout(location = 1) out vec3 fNormal;
layout(location = 2) out vec3 fAmbient;
layout(location = 3) out vec3 fDiffuse;
layout(location = 4) out vec4 fGlossyShininess;

uniform vec3 uDirectionalLightDir;
uniform vec3 uDirectionalLightIntensity;
uniform vec3 uPointLightPosition;
uniform vec3 uPointLightIntensity;

uniform sampler2D uKaSampler;
uniform sampler2D uKdSampler;
uniform sampler2D uKsSampler;
uniform sampler2D uShininessSampler;

uniform vec3 uKa;
uniform vec3 uKd;
uniform vec3 uKs;
uniform float uShininess;

/*vec3 blinnPhong(vec3 normal, vec3 intensity, vec3 outDir, vec3 incDir){
    vec3 Kd = texture(uKdSampler, vFragTex).rgb * uKd;
    vec3 Ks = texture(uKsSampler, vFragTex).rgb * uKs;
    vec3 halfVector = (outDir + incDir)/2;
    vec3 vec = intensity*((Kd*dot(incDir, normal)) + (Ks * pow(dot(halfVector, normal), uShininess)));
    return vec;
}*/

void main(){
    
    /*float distToPointLight = length(uPointLightPosition -vFragPositionVS);
    vec3 dirToPointLight = (uPointLightPosition - vFragPositionVS) / distToPointLight;
    vec3 Ka = texture(uKaSampler, vFragTex).rgb * uKa;
    fFragColor = blinnPhong(normalize(vFragNormalVS), uDirectionalLightIntensity, normalize(-vFragPositionVS), uDirectionalLightDir) +
                blinnPhong(normalize(vFragNormalVS), uPointLightIntensity / (distToPointLight * distToPointLight), normalize(-vFragPositionVS), dirToPointLight)+Ka;*/

    fPosition = vFragPositionVS;
    fNormal = vFragNormalVS;
    fAmbient = texture(uKaSampler, vFragTex).rgb * uKa;
    fDiffuse = texture(uKdSampler, vFragTex).rgb * uKd;
    fGlossyShininess = vec4(texture(uKsSampler, vFragTex).rgb * uKs,uShininess);
}