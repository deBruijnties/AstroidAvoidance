#version 430 core

in vec2 vUV;
out vec4 FragColor;

uniform sampler2D uTex;
uniform vec3 uColor;

void main()
{
    vec4 tex = texture(uTex, vUV);

    FragColor = vec4(uColor, 1.0) * tex;

    // Optional early discard for fully transparent pixels
    if (FragColor.a <= 0.001)
        discard;
}
