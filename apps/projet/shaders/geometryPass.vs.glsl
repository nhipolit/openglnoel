#version 330 core

layout(location = 0) in vec3 aVertexPosition;
layout(location = 1) in vec3 aVertexNormal;
layout(location = 2) in vec2 aVertexTex;

uniform mat4 uMVPMatrix;
uniform mat4 uMVMatrix;
uniform mat4 uNormalMatrix;

out vec3 vFragPositionVS;
out vec3 vFragNormalVS;
out vec2 vFragTex;


void main(){
    vec4 vertexPosition = vec4(aVertexPosition,1);
    vec4 vertexNormal = vec4(aVertexNormal,0);
    
    vFragTex = aVertexTex;
    vFragPositionVS = vec3(uMVMatrix * vertexPosition);
    vFragNormalVS = vec3(uNormalMatrix * vertexNormal);
    
    gl_Position = uMVPMatrix * vertexPosition;
}