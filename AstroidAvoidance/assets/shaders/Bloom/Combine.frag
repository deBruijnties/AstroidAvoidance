#version 410 core

in vec2 TexCoord;
out vec4 FragColor;

uniform sampler2D scene;
uniform sampler2D bloomBlur;
uniform float bloomIntensity;

void main()
{
    vec3 sceneCol = texture(scene, TexCoord).rgb;
    vec3 bloomCol = texture(bloomBlur, TexCoord).rgb;

    FragColor = vec4(sceneCol + bloomCol * bloomIntensity, 1.0);
}
