#version 430 core

out vec4 FragColor;

uniform vec3 color;

in vec2 vUV;


void main()
{
	FragColor = vec4(color,1);
	FragColor = vec4(vUV,0,1);
}