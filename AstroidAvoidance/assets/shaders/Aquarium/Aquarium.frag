#version 430 core

in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec3 vLocalNormal;
in vec2 vUV;

#include <Camera.glsl>
#include <Lighting.glsl>

// G-buffer outputs
out vec4 FragColor;

void main()
{

    vec3 albedo = vec3(0.5333333333333333f, 0.7725490196078432f, 0.9098039215686274f);

    vec4 emissionAO  = vec4(0.0, 0.0, 0.0, 1.0);
    
    float metallic = 0.0f;
    
    vec3 normal = normalize(vWorldNormal);

    float roughness = 0.0;
    
    vec3 emission = emissionAO.rgb;
    float ao      = emissionAO.a;
    
    vec3 viewDir = normalize(u_CameraPos - vWorldPos);
    
    vec3 lighting = ComputePointLights(
        vWorldPos,
        normal,
        viewDir,
        albedo,
        roughness,
        metallic
    );

    // Ambient + emission
    lighting += albedo * 0.03 * ao;
    lighting += emission;
    
    FragColor = vec4(lighting, 0.3f);
}
