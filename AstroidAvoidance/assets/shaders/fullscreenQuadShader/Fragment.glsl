#version 330 core

out vec4 FragColor;

in vec2 TexCoord;

uniform sampler2D Tex;



void main()
{
    // Load G-buffer textures
    vec3 color = texture(Tex, TexCoord).xyz;
    
    FragColor = vec4(color, 1.0);
    //FragColor = vec4(TexCoord,0.0, 1.0); // test UV
}
