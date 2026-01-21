#version 430 core

layout(location = 0) in vec3 aPos;
layout(location = 1) in vec3 aNormal;
layout(location = 3) in vec2 aUV;
layout(location = 4) in vec3 aTangent;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec3 vWorldPos;
out vec3 vWorldNormal;
out vec3 vLocalNormal;
out vec2 vUV;
out mat3 vTBN; 

layout(std430, binding = 12) readonly buffer InstanceMatrices
{
    mat4 models[];
};

void main()
{
    mat4 u_Model = models[gl_InstanceID];
    vec4 world = u_Model * vec4(aPos, 1.0);
    vWorldPos = world.xyz;
    vUV = aUV;

    // Normal matrix
    mat3 normalMat = mat3(transpose(inverse(u_Model)));

    vLocalNormal = aNormal;   // <<< THIS WAS MISSING

    // World-space normal
    vec3 N = normalize(normalMat * aNormal);

    vec3 T = normalMat * aTangent;    // do NOT normalize yet

    if (length(T) < 0.0001)
    {
        // Build a fallback tangent from the normal
        vec3 up = abs(N.y) < 0.999 ? vec3(0,1,0) : vec3(1,0,0);
        T = normalize(cross(up, N));
    }
    else
    {
        T = normalize(T);
    }

    // Orthonormalize tangent against normal
    T = normalize(T - dot(T, N) * N);

    vec3 B = cross(N, T);

    // --- FIX 3: WRITE WORLD NORMAL (you already fixed this) ---
    vWorldNormal = N;

    // TBN matrix for normal mapping (even if not used yet)
    vTBN = mat3(T, B, N);

    gl_Position = u_Projection * u_View * world;
}