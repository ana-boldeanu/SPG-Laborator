#version 430

layout(location = 0) out vec4 out_color;

uniform sampler2D texture_1;
uniform samplerCube texture_cubemap;
uniform int draw_cubemap;
uniform int draw_outlines;

in vec3 frag_position;
in vec2 frag_texture_coord;

void main()
{
    vec3 color = vec3(0);

    if (draw_cubemap == 0)
        color = texture(texture_1, frag_texture_coord).xyz;

    if (draw_cubemap == 1)
        color = texture(texture_cubemap, normalize(frag_position)).xyz;

    // Overwrite the color if drawing outlines
    out_color = draw_outlines == 1 ? vec4(1, 0, 0, 1) : vec4(color, 1);
}
