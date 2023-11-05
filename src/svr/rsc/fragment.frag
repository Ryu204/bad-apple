#version 460 core

out vec4 FragColor;
in vec2 texCoord;
uniform sampler2D image_texture;

void main()
{
    FragColor = texture(image_texture, texCoord);
}