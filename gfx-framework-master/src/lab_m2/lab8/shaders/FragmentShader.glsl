#version 410

// Input
layout(location = 0) in vec2 texture_coord;

// Uniform properties
uniform sampler2D textureImage;
uniform ivec2 screenSize;
uniform int flipVertical;
uniform int outputMode = 2; // 0: original, 1: grayscale, 2: blur

// Output
layout(location = 0) out vec4 out_color;

// Local variables
vec2 textureCoord = vec2(texture_coord.x, (flipVertical != 0) ? 1 - texture_coord.y : texture_coord.y); // Flip texture
vec4 global_color;

vec4 grayscale(int use_global_color)
{
    vec4 color = global_color;
    
    if (use_global_color == 0)
        color = texture(textureImage, textureCoord);

    float gray = 0.21 * color.r + 0.71 * color.g + 0.07 * color.b; 
    return vec4(gray, gray, gray,  0);
}


vec4 blur(int use_global_color, int blurRadius)
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 sum = vec4(0);
    for(int i = -blurRadius; i <= blurRadius; i++)
    {
        for(int j = -blurRadius; j <= blurRadius; j++)
        {
            sum += texture(textureImage, textureCoord + vec2(i, j) * texelSize);
        }
    }
        
    float samples = pow((2 * blurRadius + 1), 2);

    vec4 color = sum / samples;

    if (use_global_color == 1)
        global_color = color;

    return color;
}


vec4 blur_sobel(int si, int sj)
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 sum = vec4(0);
            
    for(int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            sum += texture(textureImage, textureCoord + vec2(i + si, j + sj) * texelSize);
        }
    }
    
    float samples = pow((2 * 1 + 1), 2);
    return sum / samples;
}


vec4 median(int radius)
{
    vec2 texelSize = 1.0f / screenSize;
    vec4 neighbours[100];

    for (int i = -radius/2; i <= radius/2; ++i)
    {
        for (int j = -radius/2; j <= radius/2; ++j)
        {
            neighbours[radius * (i + radius/2) + (j + radius/2)] = texture(textureImage, textureCoord + vec2(i, j) * texelSize);
        }   
    }

    // Bubble-sort the array to get the median value
    for (int i = 0; i < radius * radius - 1; i++)
    {
        for (int j = i + 1; j < radius * radius; j++) 
        {
            float intensity_1 = 0.21 * neighbours[i].r + 0.71 * neighbours[i].g + 0.07 * neighbours[i].b;
            float intensity_2 = 0.21 * neighbours[j].r + 0.71 * neighbours[j].g + 0.07 * neighbours[j].b;
            
            if (intensity_1 < intensity_2)
            {
                vec4 temp_tex = neighbours[i];
                neighbours[i] = neighbours[j];
                neighbours[j] = temp_tex;
           }
        }
    }

    return neighbours[radius * radius / 2];
}

vec4 sobel()
{
    vec2 texelSize = 1.0f / screenSize;
    
    // Apply blur on neighbours
    vec3 neighbours[9];
    for (int i = -1; i <= 1; i++)
    {
        for(int j = -1; j <= 1; j++)
        {
            neighbours[3 * (i + 1) + (j + 1)] = blur_sobel(i, j).rgb;
        }
    }

    // Get grayscale values
    float greys[9];
    for (int i = 0; i < 9; i++)
    {
        greys[i] = 0.21 * neighbours[i].r + 0.71 * neighbours[i].g + 0.07 * neighbours[i].b;   
    }

    float dx = abs(-greys[0] + greys[2] - 2 * greys[3] + 2 * greys[5] - greys[6] + greys[8]);
    float dy = abs(greys[0] + 2 * greys[1] + greys[2] - greys[6] - 2 * greys[7] - greys[8]);
    float d = dx + dy;

    if (d > 0.4)
        d = 1.0;
    else
        d = 0.0;

    return vec4(d, d, d, 0);
}

void main()
{
    switch (outputMode)
    {
        case 1:
        {
            out_color = grayscale(0);
            break;
        }

        case 2:
        {
            out_color = blur(0, 3);
            break;
        }

        case 3:
        {
            out_color = median(7);
            break;
        }

        case 4:
        {
            out_color = sobel();
            break;
        }

        default:
            out_color = texture(textureImage, textureCoord);
            break;
    }
}
