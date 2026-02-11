#version 410 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D hdrColor;
uniform float threshold = 1;

void main()
{
    vec3 color = texture(hdrColor, TexCoord).rgb;
    float brightness = max(max(color.r, color.g), color.b);
    if (brightness > threshold)
        FragColor = vec4(color, 1.0);
    else
        FragColor = vec4(0.0);
}
