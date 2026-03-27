#version 430 core

in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vNormalLocal;
in vec2 vUV;

// Texture samplers
uniform sampler2D sunTexture;

// G-buffer outputs
layout (location = 0) out vec4 gAlbedoMetal;
layout (location = 1) out vec4 gNormalRough;
layout (location = 2) out vec4 gEmissionAO; 

void main()
{
    vec3 sunColor = texture(sunTexture, vUV).xyz;
    gAlbedoMetal = vec4(sunColor, 0.0f);
    gNormalRough = vec4(normalize(vNormal),0.5f);
    gEmissionAO = vec4(texture(sunTexture, vUV).xyz * 20,0);

}
