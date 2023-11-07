#version 460 core

out vec4 FragColor;
in vec2 texCoord;
uniform sampler2D image_texture;
uniform vec2 image_size;
uniform float threshold[4];

layout(std430, binding = 0) buffer Output{
    int data[64];
};

void update_data(float gray) {
    int x = int(texCoord.x * 8);
    int y = int(texCoord.y * 8);
    int i = int(clamp(y * 8 + x, 0, 63));
    data[i] = int(max(4 * gray, data[i]));
}

void main() {
    vec4 color = texture(image_texture, texCoord);
    float gray = color.r * 0.6 + color.g * 0.3 + color.b * 0.1;
    gray = gray < threshold[0] ? 0 : gray < threshold[1] ? 0.25 : gray < threshold[2] ? 0.5 : gray < threshold[3] ? 0.75 : 1;
    FragColor = gray.xxxx; FragColor.a = 1.0;
    update_data(gray);
}