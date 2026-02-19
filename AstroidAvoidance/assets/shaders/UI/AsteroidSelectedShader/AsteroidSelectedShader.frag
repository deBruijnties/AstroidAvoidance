#version 430 core

in vec3 vWorldNormal;
in vec3 vWorldPos;
in vec2 vUV;

flat in int vSelected;


uniform vec3 sunLightPos;
uniform sampler2D AstroidTexture;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;



void main()
{
    vec3 N = normalize(vWorldNormal);
    vec3 L = normalize(sunLightPos - vWorldPos);

    float lightDot = clamp(dot(N, L), 0.0, 1.0);

    float night = 1.0 - lightDot;

    vec4 texColor = texture(AstroidTexture, vUV);

    vec4 finalColor = mix(texColor, vec4(0.0, 0.0, 0.0, 1.0), night);

    gPosition = vec4(vWorldPos, 1.0);
    gNormal   = vec4(N, 1.0);
    gAlbedo   = finalColor;
}
