#version 330

layout (location = 0) in vec3 inMeshCoord;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in mat4 instModelMat;
//location 3 is also `instModelMat` (2nd row)
//location 4 is also `instModelMat` (3rd row)
//location 5 is also `instModelMat` (4th row)


out vec2 texCoord;

uniform mat4 inViewMat;
uniform mat4 inProjMat;

void main()
{
    texCoord = inTexCoord;
    gl_Position = inProjMat * inViewMat * instModelMat * vec4(inMeshCoord, 1.0f);
}
