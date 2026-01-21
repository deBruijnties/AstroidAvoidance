#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aUV;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec3 vLocalNormal;
out vec2 vUV;

layout(std430, binding = 12) readonly buffer InstanceMatrices
{
    mat4 models[];
};

void main()
{
    mat4 u_Model = models[gl_InstanceID];
    vec4 world = u_Model * vec4(aPos, 1.0);
    vWorldPos = world.xyz;

    // Correct world-space normal
    vWorldNormal = normalize(mat3(transpose(inverse(u_Model))) * aNormal);
    vLocalNormal = aNormal;
    vUV = aUV;

    gl_Position = u_Projection * u_View * world;
}
