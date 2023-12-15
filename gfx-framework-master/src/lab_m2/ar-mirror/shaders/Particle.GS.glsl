#version 430

// Input and output topologies
layout(points) in;
layout(triangle_strip, max_vertices = 36) out;

// Uniform properties
uniform mat4 viewMatrices[6];
uniform mat4 Projection;
uniform vec3 mirror_position;
uniform float offset;

in float vert_lifetime[1];
in float vert_iLifetime[1];

// Output
layout(location = 0) out vec2 texture_coord;
layout(location = 1) out float geom_lifetime;
layout(location = 2) out float geom_iLifetime;

// Local variables
vec3 vpos = gl_in[0].gl_Position.xyz;
vec3 forward = normalize(mirror_position - vpos);
vec3 up, right;

void EmitPoint(vec2 offset, int layer)
{
    vec3 pos = right * offset.x + up * offset.y + vpos;
    gl_Position = Projection * viewMatrices[layer] * vec4(pos, 1.0);
    EmitVertex();
}


void main()
{
    float ds = offset;

    // Create a quad based on `triangle_strip`.
    //
    //  2---------3
    //  | \       |
    //  |    \    |
    //  |       \ |
    //  0---------1
    //
    // Triangles: (0, 1, 2), (1, 2, 3)
    // Before emitting a vertex, specify its texture coordinates and pass on the values of vert_lifetime[0] and 
    // vert_ilifetime[0] in geom_lifetime and geom_ilifetime

    vec3 up_vals[6] = {vec3(0.0f,-1.0f, 0.0f),
        vec3(0.0f,-1.0f, 0.0f),
        vec3(0.0f, 0.0f, 1.0f),
        vec3(0.0f, 0.0f,-1.0f),
        vec3(0.0f,-1.0f, 0.0f),
        vec3(0.0f,-1.0f, 0.0f), };

    for (int layer = 0; layer < 6; ++layer)
    {
        gl_Layer = layer;
        
        up = up_vals[layer];
        right = normalize(cross(forward, up));
        
        geom_lifetime = vert_lifetime[0];
        geom_iLifetime = vert_iLifetime[0];
        texture_coord = vec2(0, 0);
        EmitPoint(vec2(-offset, -offset), layer);
    
        geom_lifetime = vert_lifetime[0];
        geom_iLifetime = vert_iLifetime[0];
        texture_coord = vec2(1, 0);
        EmitPoint(vec2(offset, -offset), layer);
    
        geom_lifetime = vert_lifetime[0];
        geom_iLifetime = vert_iLifetime[0];
        texture_coord = vec2(0, 1);
        EmitPoint(vec2(-offset, offset), layer);
    
        geom_lifetime = vert_lifetime[0];
        geom_iLifetime = vert_iLifetime[0];
        texture_coord = vec2(1, 1);
        EmitPoint(vec2(offset, offset), layer);

        EndPrimitive();
    }
}
