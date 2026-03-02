#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 3) in vec2 aUV;

out vec2 vUV;

uniform mat4 u_model;      // element transform (position, scale, rotation)
uniform mat4 u_Projection; // orthographic projection

void main() {
    vUV = aUV;
    gl_Position = u_Projection * u_model * vec4(aPos, 1.0);
}