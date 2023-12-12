#version 430

layout(triangles) in;
layout(triangle_strip, max_vertices = 36) out;

// Uniform properties
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 viewMatrices[6];

// Input from vertex shader
in vec3 geom_position[3];
in vec3 geom_normal[3];
in vec2 geom_texture_coord[3];

// Output to fragment shader
out vec3 frag_position;
out vec2 frag_texture_coord;

void main()
{
    int i, layer;

    // Compute the position from each camera view in order to render a cubemap in one pass using gl_Layer.
    for (layer = 0; layer < 6; layer++)
    {
        gl_Layer = layer;

        for (i = 0; i < gl_in.length(); i++)
        {
             frag_position = geom_position[i];
             frag_texture_coord = geom_texture_coord[i];
             gl_Position = Projection * viewMatrices[layer] * gl_in[i].gl_Position;
             EmitVertex();
        }

        EndPrimitive();
    }
}
