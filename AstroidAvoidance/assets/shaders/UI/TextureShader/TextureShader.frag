#version 430 core

in vec2 vUV;

uniform sampler2D uTexture; // your texture

out vec4 FragColor;

void main()
{
    FragColor = texture(uTexture, vUV);
    //FragColor = vec4(1,0,0,1);
    
    // Optional: discard transparent pixels
    if (FragColor.a < 0.01)
        discard;
}