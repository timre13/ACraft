#version 330

out vec4 outColor;

// TODO: Only for testing
in vec3 color;

void main()
{
    outColor = vec4(color, 1.0f);
}
