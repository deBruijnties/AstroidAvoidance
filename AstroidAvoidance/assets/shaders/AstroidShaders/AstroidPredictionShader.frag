#version 430 core

// G-buffer outputs
layout (location = 0) out vec4 gAlbedoMetal; // RGB = albedo, A = metallic

void main()
{
    // Color Prediction dots Plain White as it will be rendered unlit the metallic property will be ignored
    gAlbedoMetal = vec4(1,1,1,1);
}
