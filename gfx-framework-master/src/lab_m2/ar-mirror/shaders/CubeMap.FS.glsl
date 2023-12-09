#version 430

// Input
layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;

// Uniform properties
uniform samplerCube texture_cubemap;
uniform vec3 camera_position;

// Output
layout(location = 0) out vec4 out_color;


vec3 myReflect()
{
    // Compute the reflection color value

    vec3 incident_ray = world_position - camera_position;
    vec3 reflected_ray = reflect(incident_ray, world_normal);

    return texture(texture_cubemap, reflected_ray).xyz;
}

void main()
{
    out_color = vec4(myReflect(), 0);
}
