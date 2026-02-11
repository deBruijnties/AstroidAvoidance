#version 430 core

in vec2 vUV;

layout(location = 0) out vec4 gPosition;
layout(location = 1) out vec4 gNormal;
layout(location = 2) out vec4 gAlbedo;


uniform sampler2D mouseTexture;

void main()
{
vec4 color = texture(mouseTexture, vUV);
    gAlbedo = color;
    if (color.a < 0.01)
        discard; // critical for correct transparency behaviour!
}
