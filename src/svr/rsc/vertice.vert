#version 460 core

layout (location = 0) in vec2 pos;
layout (location = 1) in vec2 inTexCoord;
out vec2 texCoord;

void main() {
    gl_Position = vec4(pos.x, pos.y, 0.0 , 1.0);
    texCoord = inTexCoord;
}