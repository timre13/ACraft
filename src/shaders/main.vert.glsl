#version 330

layout (location = 0) in vec3 inMeshCoord;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 instModelPos;

out vec2 texCoord;

uniform mat4 inViewMat;
uniform mat4 inProjMat;

#define MODEL_POS_MULTIPLIER 2.0f

void main()
{
    texCoord = inTexCoord;
    gl_Position = inProjMat * inViewMat * vec4(
            inMeshCoord.x + instModelPos.x*MODEL_POS_MULTIPLIER,
            inMeshCoord.y + instModelPos.y*MODEL_POS_MULTIPLIER,
            inMeshCoord.z + instModelPos.z*MODEL_POS_MULTIPLIER,
            1.0f);
}
