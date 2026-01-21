#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aUV;    // optional

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

out vec3 vWorldPos;
out vec3 vNormal;
out vec3 vNormalLocal;
out vec2 vUV;

void main()
{
    vec4 world = model * vec4(aPos, 1.0);
    vWorldPos = world.xyz;

    // transformed normal in world space
    vNormal = normalize(mat3(transpose(inverse(model))) * aNormal);

    // local (object) normal if you need spherical mapping in object space
    vNormalLocal = aNormal;

    vUV = aUV;

    gl_Position = projection * view * world;
}
