#version 430 core
layout(location=0) in vec3 aPos;
layout(location=1) in vec3 aNormal;
layout(location=3) in vec2 aUV;

out vec3 vWorldNormal;
out vec3 vWorldPos;
out vec2 vUV;

flat out int vSelected;


struct InstanceData {
    mat4 model;     // 64 bytes
    int selected;   // 4 bytes
    int pad0;       // 4 bytes
    int pad1;       // 4 bytes
    int pad2;       // 4 bytes
};


layout(std430, binding=3) buffer InstanceMatrices {
    InstanceData models[];
};

uniform mat4 view;
uniform mat4 projection;

void main() {

    mat4 model = models[gl_InstanceID].model;

    vSelected = models[gl_InstanceID].selected;

    // correct simple normal transform (ONLY works if no scale)
    vWorldNormal = normalize(mat3(model) * aNormal);

    // world position
    vWorldPos = (model * vec4(aPos,1.0)).xyz;

    vUV = aUV;

    gl_Position = projection * view * vec4(vWorldPos, 1.0);
}
