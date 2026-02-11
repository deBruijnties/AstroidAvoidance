#version 430 core

layout(location = 0) in vec3 aPos;   // quad vertices centered at 0
layout(location = 3) in vec2 aUV;


uniform vec3 mousePosition;          // NDC (-1..1)

out vec2 vUV;

void main()
{
    vec2 pos = aPos.xy + mousePosition.xy;

    gl_Position = vec4(pos - vec2(-.05f,.05f), 0.0, 1.0);

    vUV = aUV;
}
