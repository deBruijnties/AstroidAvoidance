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
    // Base surface color (light blue)
    vec3 albedo = vec3(0.5333333333333333f, 0.7725490196078432f, 0.9098039215686274f);
    
    // RGB = emission, A = ambient occlusion packed together
    vec4 emissionAO  = vec4(0.0, 0.0, 0.0, 1.0);
    
    float metallic = 0.0f;
    
    vec3 normal = normalize(vWorldNormal);

    float roughness = 0.0;
    
    vec3 emission = emissionAO.rgb;
    float ao      = emissionAO.a;
    
    // Direction from fragment to camera (used for specular lighting)
    vec3 viewDir = normalize(u_CameraPos - vWorldPos);
    
    // Compute lighting contribution from all point lights
    vec3 lighting = ComputePointLights(
        vWorldPos,
        normal,
        viewDir,
        albedo,
        roughness,
        metallic
    );

    // Add simple ambient term scaled by AO
    lighting += albedo * 0.03 * ao;
    // Add emissive contribution (unlit)
    lighting += emission;
    
    // Output final color, alpha is fixed for blending
    FragColor = vec4(lighting, 0.3f);
}
