#version 330 core

layout(location = 0) in vec3 aPosition;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec2 aTexCoords;

uniform mat4 uModelViewProjMatrix;
uniform mat4 uModelViewMatrix;
uniform mat4 uNormalMatrix;

out vec3 vViewSpacePosition;
out vec3 vViewSpaceNormal;
out vec3 vTexCoords;

void main(){
	vViewSpacePosition = uModelViewMatrix*aPosition;
	vViewSpaceNormal = uNormalMatrix*aNormal;
	vTexCoords = vec3(aTexCoords, 1);
	gl_Position = uModelViewProjMatrix*aPosition;
};