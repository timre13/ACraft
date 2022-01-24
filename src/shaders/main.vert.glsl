#version 330

layout (location = 0) in vec3 inMeshCoord;
layout (location = 1) in vec2 inTexCoord;

out vec2 texCoord;

uniform mat4 inModelMat;
uniform mat4 inViewMat;
uniform mat4 inProjMat;

void main()
{
    texCoord = inTexCoord;
    gl_Position = inProjMat * inViewMat * inModelMat * vec4(inMeshCoord, 1.0f);
}
