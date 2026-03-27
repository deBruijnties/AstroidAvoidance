#version 430 core

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec2 vUV;
in mat3 vTBN;

uniform sampler2D Texture;
uniform sampler2D CrtScreenTexture;
uniform float intensity = 1;

// G-buffer outputs
layout (location = 0) out vec4 gAlbedoMetal;
layout (location = 1) out vec4 gNormalRough;
layout (location = 2) out vec4 gEmissionAO;

void main()
{
    vec3 albedo = texture(Texture, vUV).rgb;
    float metallic = 0.0;
    float roughness = 0.5;
    float ao = 1.0;
    vec3 emission = vec3(0.0);
    vec3 normal = normalize(vWorldNormal);

    // set color metall normal roughtness and emmission and ambient occlusion 
    gAlbedoMetal = vec4(albedo, metallic);
    gNormalRough = vec4(normal * 0.5 + 0.5, roughness);
    gEmissionAO  = vec4(emission, ao);

    // CRT screen region
    if (vUV.x > 0.5 && vUV.y < 0.5)
    {
        vec2 localUV = vUV;
        localUV.x -= 0.5;
        localUV *= 2.0;

        vec3 screenColor = texture(CrtScreenTexture, localUV).rgb;

        gAlbedoMetal.rgb = screenColor;
        gNormalRough.a = 1;

        // set crt screen emission rendertexture color
        gEmissionAO.rgb  = screenColor * intensity;
    }
}
