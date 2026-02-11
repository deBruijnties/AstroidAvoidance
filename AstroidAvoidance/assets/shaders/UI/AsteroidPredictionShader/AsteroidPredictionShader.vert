#version 430 core
layout(location=0) in vec3 aPos;     // quad vertex offsets in NDC space
layout(location=1) in vec3 aNormal;
layout(location=3) in vec2 aUV;

out vec2 vUV;
out float value;

layout(std430, binding=2) buffer InstancePosition {
    vec4 InstancePositions[];
};

uniform mat4 view;
uniform mat4 projection;

void main()
{
    vec3 worldPos = InstancePositions[gl_InstanceID].xyz;
    vUV = aUV;

    // ---- A. Compute asteroid clip-space position ----
    vec4 clip = projection * view * vec4(worldPos, 1.0);

    // Perspective divide for NDC
    vec2 ndc = clip.xy / clip.w;

    // ---- B. Add quad vertex offset in NDC ----
    vec2 finalNdc = ndc + aPos.xy;

    // ---- C. Convert back to clip-space ----
    // x and y get multiplied by w again
    float depthZ = clip.z;   // keep actual depth
    float depthW = clip.w;

    gl_Position = vec4(finalNdc * depthW, depthZ, depthW);

    value = gl_InstanceID / 10.0;
}
