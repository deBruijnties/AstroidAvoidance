#version 430 core

layout (location = 0) in vec3 aPos; // quad vertex (0..1)
layout (location = 3) in vec2 aUV;

uniform mat4 model;
uniform mat4 projection;

out vec2 vUV;

void main()
{
    vUV = aUV;

    // model already contains rect size & position
    gl_Position = projection * model * vec4(aPos, 1.0);
}
