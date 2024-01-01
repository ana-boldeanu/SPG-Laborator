#version 410

// Input
layout(location = 0) in vec2 texture_coord;

// Uniform properties
uniform sampler2D textureImage;

// Output
layout(location = 0) out vec4 out_color;

// Flip texture
vec2 flipped_texture_coord = vec2(texture_coord.x, 1 - texture_coord.y);

void main()
{
    out_color = texture(textureImage, flipped_texture_coord);
}
