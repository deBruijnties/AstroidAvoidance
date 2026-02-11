#version 430 core

// Fragment-stage inputs (coming from vertex shader)
in vec3 vWorldPos;
in vec3 vNormal;
in vec3 vNormalLocal;
in vec2 vUV; // if you want to use UVs later

// Texture samplers
uniform sampler2D sunTexture;

// G-buffer outputs
layout (location = 0) out vec4 gAlbedoMetal; // RGBA8
layout (location = 1) out vec4 gNormalRough; // RGBA16F
layout (location = 2) out vec4 gEmissionAO;  // RGBA16F

void main()
{
    vec3 sunColor = texture(sunTexture, vUV).xyz;
    gAlbedoMetal = vec4(sunColor, 0.0f); // include alpha
    gNormalRough = vec4(normalize(vNormal),0.5f);
    gEmissionAO = vec4(texture(sunTexture, vUV).xyz * 20,0);
    //gAlbedoMetal = vec4(1,1,0, 1.0f);

}
