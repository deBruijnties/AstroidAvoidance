#version 430 core

layout (location = 0) out vec4 outColor;

uniform sampler2D uAlbedoMap;
uniform sampler2D uEmissionMap;

uniform vec3  uAlbedoColor   = vec3(1.0);
uniform vec3  uEmissionColor = vec3(0.0);

uniform bool uUseAlbedoMap;
uniform bool uUseEmissionMap;

in vec2 vUV;

void main()
{
    vec3 albedo = uUseAlbedoMap
        ? texture(uAlbedoMap, vUV).rgb
        : uAlbedoColor;

    vec3 emission = uUseEmissionMap
        ? texture(uEmissionMap, vUV).rgb
        : uEmissionColor;

    // Forward unlit = base color + self emission
    vec3 color = albedo + emission;

    outColor = vec4(color, 1.0);
}
