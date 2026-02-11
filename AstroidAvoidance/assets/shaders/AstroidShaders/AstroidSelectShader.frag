#version 430 core

// G-buffer outputs
layout (location = 0) out vec4 gAlbedoMetal; // RGB = albedo, A = metallic
layout (location = 1) out vec4 gNormalRough; // RGB = world normal, A = roughness
layout (location = 2) out vec4 gEmissionAO;  // RGB = emission, A = AO

// Material textures
uniform sampler2D uAlbedoMap;
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

// Feature toggles
uniform bool uUseAlbedoMap;
uniform bool uUseMetallicMap;
uniform bool uUseRoughnessMap;
uniform bool uUseAOMap;
uniform bool uUseEmissionMap;

// Inputs
in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec2 vUV;

void main()
{
    gAlbedoMetal = vec4(1,1,1,1);

   
}
