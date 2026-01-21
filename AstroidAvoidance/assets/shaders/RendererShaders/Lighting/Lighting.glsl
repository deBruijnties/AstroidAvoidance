#ifndef LIGHTING_GLSL
#define LIGHTING_GLSL

#include <Camera.glsl>

uniform int u_PointLightCount;

struct PointLight
{
    vec4 position; // xyz + radius
    vec4 color;    // rgb + intensity
};

layout(std430, binding = 1) buffer PointLights
{
    PointLight lights[];
};

const float PI = 3.14159265359;

vec3 ComputePointLights(
    vec3 worldPos,
    vec3 normal,
    vec3 viewDir,
    vec3 albedo,
    float roughness,
    float metallic)
{
    vec3 result = vec3(0.0);

    vec3 N = normalize(normal);
    vec3 V = normalize(viewDir);

    // Clamp roughness to avoid NaNs / fireflies
    roughness = clamp(roughness, 0.04, 1.0);

    vec3 F0 = mix(vec3(0.04), albedo, metallic);

    for (int i = 0; i < u_PointLightCount; i++)
    {
        vec3 Lvec = lights[i].position.xyz - worldPos;
        float dist = length(Lvec);
        float radius = lights[i].position.w;

        if (dist > radius)
            continue;

        vec3 L = Lvec / dist;
        vec3 H = normalize(V + L);

        float NdotL = max(dot(N, L), 0.0);
        float NdotV = max(dot(N, V), 0.0);
        float NdotH = max(dot(N, H), 0.0);
        float VdotH = max(dot(V, H), 0.0);

        if (NdotL <= 0.0 || NdotV <= 0.0)
            continue;

        // Attenuation (your original model)
        float attenuation = 1.0 - (dist / radius);
        vec3 lightColor = lights[i].color.rgb * lights[i].color.a;
        vec3 radiance = lightColor * attenuation;

        // ----- GGX Specular -----
        float a  = roughness * roughness;
        float a2 = a * a;

        float denom = (NdotH * NdotH) * (a2 - 1.0) + 1.0;
        float D = a2 / (PI * denom * denom);

        float k = (a + 1.0) * (a + 1.0) / 8.0;
        float Gv = NdotV / (NdotV * (1.0 - k) + k);
        float Gl = NdotL / (NdotL * (1.0 - k) + k);
        float G = Gv * Gl;

        vec3 F = F0 + (1.0 - F0) * pow(1.0 - VdotH, 5.0);

        vec3 specular = (D * G * F) / max(4.0 * NdotV * NdotL, 0.001);

        // ----- Diffuse (energy conserving) -----
        vec3 kd = (1.0 - F) * (1.0 - metallic);
        vec3 diffuse = kd * albedo / PI;

        result += (diffuse + specular) * radiance * NdotL;

    }

    return result;
}

#endif
