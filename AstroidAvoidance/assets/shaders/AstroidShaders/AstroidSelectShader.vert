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
    vec3 worldPos = u_Model[3].xyz;
    vUV = aUV;

    // ---- A. Compute asteroid clip-space position ----
    vec4 clip = u_Projection * u_View * vec4(worldPos, 1.0);

    // Perspective divide for NDC
    vec2 ndc = clip.xy / clip.w;

    // ---- B. Add quad vertex offset in NDC ----
    vec2 finalNdc = ndc + aPos.xy;

    // ---- C. Convert back to clip-space ----
    // x and y get multiplied by w again
    float depthZ = clip.z;   // keep actual depth
    float depthW = clip.w;

    gl_Position = vec4(finalNdc * depthW, depthZ, depthW);
}
