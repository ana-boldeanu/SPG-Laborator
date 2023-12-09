#version 430

layout(triangles) in;
layout(line_strip, max_vertices = 36) out;

// Uniform properties
uniform mat4 Model;
uniform mat4 View;
uniform mat4 Projection;
uniform mat4 viewMatrices[6];
uniform vec3 camera_forward;

// Input from vertex shader
in vec3 geom_position[3];
in vec3 geom_normal[3];
in vec2 geom_texture_coord[3];

// Output to fragment shader
out vec3 frag_position;
out vec2 frag_texture_coord;

int areSegmentEdgesOppositeOrientation(vec3 P_1_normal, vec3 P_2_normal)
{
    float dot_1 = dot(P_1_normal, normalize(camera_forward));
    float dot_2 = dot(P_2_normal, normalize(camera_forward));

    if ((dot_1 > 0 && dot_2 < 0) || (dot_1 < 0 && dot_2 > 0))
        return 1;
    
    return 0;
}

vec3 getInterpolatedPointOnSegment(vec3 P_1, vec3 P_2, vec3 P_1_normal, vec3 P_2_normal)
{
    float dot_1 = dot(P_1_normal, normalize(camera_forward));
    float dot_2 = dot(P_2_normal, normalize(camera_forward));

    float t = abs(dot_1) / (abs(dot_1) + abs(dot_2));

    return P_1 + (P_2 - P_1) * t;
}

void main()
{
    int i, layer;

    // Compute the position from each camera view in order to render a cubemap in one pass using gl_Layer.
    for (layer = 0; layer < 6; ++layer)
    {
        gl_Layer = layer;
        
        // Find outline segments and emit vertices for their ends
        if (areSegmentEdgesOppositeOrientation(geom_normal[0], geom_normal[1]) == 1)
        {
            vec3 P_01 = getInterpolatedPointOnSegment(geom_position[0], geom_position[1], geom_normal[0], geom_normal[1]);

            frag_position = P_01;
            frag_texture_coord = geom_texture_coord[0];

            gl_Position = Projection * viewMatrices[layer] * Model * vec4(P_01, 1);
            EmitVertex();
        }

        if (areSegmentEdgesOppositeOrientation(geom_normal[1], geom_normal[2]) == 1)
        {
            vec3 P_12 = getInterpolatedPointOnSegment(geom_position[1], geom_position[2], geom_normal[1], geom_normal[2]);

            frag_position = P_12;
            frag_texture_coord = geom_texture_coord[1];

            gl_Position = Projection * viewMatrices[layer] * Model * vec4(P_12, 1);
            EmitVertex();
        }

        if (areSegmentEdgesOppositeOrientation(geom_normal[2], geom_normal[0]) == 1)
        {
            vec3 P_20 = getInterpolatedPointOnSegment(geom_position[2], geom_position[0], geom_normal[2], geom_normal[0]);

            frag_position = P_20;
            frag_texture_coord = geom_texture_coord[2];

            gl_Position = Projection * viewMatrices[layer] * Model * vec4(P_20, 1);
            EmitVertex();
        }

        EndPrimitive();
    }
}
