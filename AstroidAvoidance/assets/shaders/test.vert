#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 2) in vec4 aColor;
layout(location = 3) in vec2 aUV;
layout(location = 4) in vec3 aTangent;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 vUV;

void main()
{
	vUV = aUV;
	vec4 world = u_model * vec4(aPos, 1.0);
	gl_Position = u_projection * u_view * world;
	gl_Position = vec4(aPos,1);
}