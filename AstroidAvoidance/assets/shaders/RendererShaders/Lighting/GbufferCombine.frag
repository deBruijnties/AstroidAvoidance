#version 450 core
out vec4 FragColor;

in vec2 vUV;

uniform sampler2D gAlbedoMetal;
uniform sampler2D gNormalRough;
uniform sampler2D gEmissionAO;
uniform sampler2D gDepth;


#include <Camera.glsl>
#include <Lighting.glsl>

vec3 ReconstructWorldPosition(vec2 uv, float depth)
{
    vec4 ndc = vec4(
        uv * 2.0 - 1.0,
        depth * 2.0 - 1.0,
        1.0
    );

    vec4 world = u_InverseViewProjection * ndc;
    return world.xyz / world.w;
}



void main()
{

    float depth = texture(gDepth, vUV).r;
    
    // Skip skybox pixels
    if (depth >= 1.0)
        discard;
    
    vec3 worldPos = ReconstructWorldPosition(vUV, depth);
    
    vec4 albedoMetal = texture(gAlbedoMetal, vUV);
    vec4 normalRough = texture(gNormalRough, vUV);
    vec4 emissionAO  = texture(gEmissionAO, vUV);
    
    vec3 albedo    = albedoMetal.rgb;
    float metallic = albedoMetal.a;
    
    vec3 normal = normalize(normalRough.rgb * 2.0 - 1.0);
    float roughness = normalRough.a;
    
    vec3 emission = emissionAO.rgb;
    float ao      = emissionAO.a;
    
    vec3 viewDir = normalize(u_CameraPos - worldPos);

    vec3 lighting = ComputePointLights(
        worldPos,
        normal,
        viewDir,
        albedo,
        roughness,
        metallic
    );

    // Ambient + emission
    lighting += albedo * 0.03 * ao;
    lighting += emission;

    FragColor = vec4(lighting, 1.0);
}

