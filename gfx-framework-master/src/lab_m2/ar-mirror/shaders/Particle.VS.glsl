#version 430

// Input
layout(location = 0) in vec3 v_position;
layout(location = 1) in vec3 v_normal;
layout(location = 2) in vec2 v_texture_coord;

// Uniform properties
uniform mat4 Model;
uniform vec3 generator_position;
uniform vec3 bezier_points[20]; // 5 bezier curves, each defined by 4 control points
uniform float deltaTime;

out float vert_lifetime;
out float vert_iLifetime;

struct Particle
{
    vec4 position;
    vec4 iPosition;
    vec4 speed;
    float delay;
    float iDelay;
    float lifetime;
    float iLifetime;
};


layout(std430, binding = 0) buffer particles {
    Particle data[];
};


float rand(vec2 co)
{
    return fract(sin(dot(co.xy, vec2(12.9898, 78.233))) * 43758.5453);
}


void main()
{
    // Get the particle's speed
    float speed = data[gl_VertexID].speed.x;
    
    // Calculate remaining particle delay
    float delay = data[gl_VertexID].delay;

    delay -= deltaTime * speed;
    if (delay > 0) 
    {
        // Continue without updating position
        data[gl_VertexID].delay = delay;
        gl_Position = Model * vec4(generator_position, 1);
        return;
    }
    
    // Update the particle's lifetime, as it should decrease each frame with deltaTime
    float lifetime = data[gl_VertexID].lifetime;
    float initial_lifetime = data[gl_VertexID].iLifetime;

    lifetime -= deltaTime / 2;

    // Compute the next position on the bezier curve, using interpolation via the particle's lifetime.
    int curveID = gl_VertexID % 5;

    float t = 1 - lifetime / initial_lifetime;          // t == 0 when lifetime == initial_lifetime

    vec3 position = pow((1 - t), 3) * bezier_points[curveID * 4]
        + 3 * pow((1 - t), 2) * t * bezier_points[curveID * 4 + 1]
        + 3 * pow((1 - t), 2) * t * t * bezier_points[curveID * 4 + 2]
        + t * t * t * bezier_points[curveID * 4 + 3];

    if (lifetime < 0)
    {
        position = (data[gl_VertexID].iPosition).xyz;
        lifetime = data[gl_VertexID].iLifetime;
        delay = data[gl_VertexID].iDelay;
    }

    data[gl_VertexID].lifetime = lifetime;

    vert_lifetime = lifetime;
    vert_iLifetime = data[gl_VertexID].iLifetime;

    gl_Position = Model * vec4(position + generator_position, 1);
}
