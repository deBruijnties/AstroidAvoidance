#version 430 core

// G-buffer outputs
layout (location = 0) out vec4 gAlbedoMetal; // RGB = albedo, A = metallic
layout (location = 1) out vec4 gNormalRough; // RGB = world normal, A = roughness
layout (location = 2) out vec4 gEmissionAO;  // RGB = emission, A = AO

// Material textures
uniform sampler2D uAlbedoMap;
uniform sampler2D uNormalMap;
uniform sampler2D uMetallicMap;
uniform sampler2D uRoughnessMap;
uniform sampler2D uAOMap;
uniform sampler2D uEmissionMap;

// Material defaults
uniform vec3  uAlbedoColor    = vec3(1.0);
uniform float uMetallicValue  = 0.0;
uniform float uRoughnessValue = 1.0;
uniform float uAOValue        = 1.0;
uniform vec3  uEmissionColor  = vec3(0.0);
uniform float uEmissionIntensity = 1.0f;

uniform float uNormalIntensity = 1.0;

// Feature toggles
uniform bool uUseAlbedoMap;
uniform bool uUseNormalMap;
uniform bool uUseMetallicMap;
uniform bool uUseRoughnessMap;
uniform bool uUseAOMap;
uniform bool uUseEmissionMap;

// Inputs
in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec2 vUV;
in mat3 vTBN;

void main()
{
    vec3 albedo = uUseAlbedoMap ? texture(uAlbedoMap, vUV).rgb : uAlbedoColor;
    float metallic = uUseMetallicMap ? texture(uMetallicMap, vUV).r : uMetallicValue;
    float roughness = uUseRoughnessMap ? texture(uRoughnessMap, vUV).r : uRoughnessValue;
    float ao = uUseAOMap ? texture(uAOMap, vUV).r : uAOValue;

    vec3 emission = uUseEmissionMap
        ? texture(uEmissionMap, vUV).rgb
        : uEmissionColor;

    vec3 normal;

    if (uUseNormalMap)
    {
        vec3 n = texture(uNormalMap, vUV).rgb;
        n = n * 2.0 - 1.0;

        // Scale only X/Y, keep Z reconstructed
        n.xy *= uNormalIntensity;
        n.z = sqrt(clamp(1.0 - dot(n.xy, n.xy), 0.0, 1.0));

        normal = normalize(vTBN * n);

    }
    else
    {
        normal = normalize(vTBN[2]);
    }

    gAlbedoMetal = vec4(albedo, metallic);
    gNormalRough = vec4(normal * 0.5 + 0.5, roughness);
    gEmissionAO  = vec4(emission * uEmissionIntensity, ao);
}