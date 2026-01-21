#version 330 core

// G-buffer outputs
layout (location = 0) out vec4 gAlbedoMetal; // RGBA8
layout (location = 1) out vec4 gNormalRough; // RGBA16F
layout (location = 2) out vec4 gEmissionAO;  // RGBA16F

uniform sampler2D skybox;
uniform sampler2D skyboxEmission;

in vec3 vWorldPos;
in vec3 vNormal;
in vec2 vUV;

void main()
{
    vec4 texColor = texture(skybox, vUV);
    vec4 texEmission = texture(skyboxEmission, vUV);

    
    gNormalRough.xyz =  vNormal;
    gAlbedoMetal.xyz = (texColor + texEmission).xyz;
    gEmissionAO.xyz = texEmission.xyz;
}