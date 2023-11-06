#version 460 core

out vec4 FragColor;
in vec2 texCoord;
uniform sampler2D image_texture;
uniform vec2 image_size;

float diff(vec4 a, vec4 b) {
    return abs(a.x - b.x) + abs(a.y - b.y) + abs(a.z - b.z);
}

vec4 sample_offset(vec2 offset) {
    return texture(image_texture, clamp(texCoord + offset, 0.0, 1.0));
}

void main()
{
    vec2 incr = vec2(1.0 / image_size.x, 1.0 / image_size.y);
    vec4 color = texture(image_texture, texCoord);
    float value = diff(sample_offset(vec2(0, incr.y)), sample_offset(vec2(0, -incr.y)))
        + diff(sample_offset(vec2(incr.x, 0)), sample_offset(vec2(-incr.x, 0)));
    
    FragColor = value.xxxx; FragColor.a = 1.0;
}