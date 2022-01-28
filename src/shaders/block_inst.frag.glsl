#version 330

out vec4 outColor;

in vec2 texCoord;
in float texLayerI;

uniform sampler2DArray textures;

void main()
{
    outColor = texture(textures, vec3(texCoord, texLayerI));
}
