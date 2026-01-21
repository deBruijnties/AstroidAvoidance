#version 430 core

// G-buffer outputs
layout (location = 0) out vec4 gAlbedoMetal; // RGB = albedo, A = metallic
layout (location = 1) out vec4 gNormalRough; // RGB = world normal, A = roughness
layout (location = 2) out vec4 gEmissionAO;  // RGB = emission, A = AO

// Material textures
uniform sampler2D earthDay;
uniform sampler2D earthInside;
uniform sampler2D earthSpec;


// Inputs
in vec3 vWorldPos;
in vec3 vWorldNormal;
in vec3 vLocalNormal;
in vec2 vUV;

void main()
{
    vec3 nLocal = normalize(vLocalNormal);

    float pixelSteps = 75.0;

    // Quantize the local normal direction
    vec3 q = normalize(floor(nLocal * pixelSteps) / pixelSteps);

    float u = 0.5 + atan(q.z, q.x) / (2.0 * 3.1415926535);
    float v = 0.5 - asin(q.y) / 3.1415926535;
    vec2 TexCoord = vec2(u, v);



    vec4 dayTex = texture(earthDay, TexCoord);
    vec4 insideTex = texture(earthInside, TexCoord);
    vec4 specTex = texture(earthSpec, TexCoord);

    float roughness = mix(0.5f,0.1f,specTex.r);

    vec3 normal = normalize(vWorldNormal);

     // inside emission mask
    float d = length(vWorldPos);
    float insideBlend = 1 - clamp(smoothstep(71.5, 73.0, d),0.0,1.0);

    // blend inside texture into albedo
    vec3 albedo = mix(dayTex.rgb, insideTex.rgb, insideBlend);
    roughness = mix(roughness, 1, insideBlend);

    vec3 emission = mix(vec3(0), insideTex.rgb, insideBlend);// * 1.25f;


    gAlbedoMetal = vec4(albedo, 0);

    gNormalRough = vec4(normal * 0.5 + 0.5, roughness);

    gEmissionAO  = vec4(emission, 1);

}
