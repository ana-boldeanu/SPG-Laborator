#version 430

// Input
layout(location = 0) in vec3 world_position;
layout(location = 1) in vec3 world_normal;

// Uniform properties
uniform samplerCube texture_cubemap;
uniform int type;

uniform vec3 camera_position;

// Output
layout(location = 0) out vec4 out_color;


vec3 myReflect()
{
    // TODO(student): Compute the reflection color value

    vec3 r_incidenta = world_position - camera_position;
    vec3 r_reflectata = reflect(r_incidenta, world_normal);
    return texture(texture_cubemap, r_reflectata).xyz;
}


vec3 myRefract(float refractive_index)
{
    // TODO(student): Compute the refraction color value

    vec3 r_incidenta = world_position - camera_position;
    vec3 r_refractata = refract(r_incidenta, world_normal, 1/refractive_index);
    return texture(texture_cubemap, r_refractata).xyz;
}


void main()
{

    if (type == 0)
    {
        out_color = vec4(myReflect(), 0);
    }
    else
    {
        out_color = vec4(myRefract(1.33), 0);
    }
}
