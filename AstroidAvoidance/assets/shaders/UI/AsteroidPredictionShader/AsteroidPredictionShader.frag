#version 430 core

in vec2 vUV;
in float value;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;

void main()
{
    // Circle mask
    float d = distance(vUV, vec2(0.5));
    float circle = step(d, 0.5);   // 1 inside the circle, 0 outside

    // Kill fragment outside circle
    if (circle == 0.0)
        discard;

    // Example output using your "value"
    gAlbedo = vec4(1, 1, 1, 1.0);
}
