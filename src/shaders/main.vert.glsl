#version 330

layout (location = 0) in vec3 inMeshCoord;
layout (location = 1) in vec2 inTexCoord;

// TODO: Only for testing
out vec3 color;

void main()
{
    color = inMeshCoord;
    gl_Position = vec4(inMeshCoord, 1.0f);
}
