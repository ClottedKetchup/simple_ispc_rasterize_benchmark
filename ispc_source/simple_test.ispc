
#include "simple_math.h"

export void simple(uniform float vin[], uniform float vout[],
                   uniform int count) {
    foreach (index = 0 ... count) 
    {
        float v = vin[index];
        if (v < 3.)
            v = v * v;
        else
            v = sqrt(v);
        vout[index] = v;
    }
}

task void
write_color(uniform float3 image[], 
   const uniform int image_width, 
   const uniform int image_height, 
   const uniform int tile_width,
   const uniform int tile_height)
{
    const uniform int x_start = taskIndex0 * tile_width;
    const uniform int x_end = min(x_start + tile_width, image_width);
    const uniform int y_start = taskIndex1 * tile_width;
    const uniform int y_end = min(y_start + tile_height, image_height);

    foreach(y_index = y_start ... y_end, x_index = x_start ... x_end)
    {
        int index = y_index * image_width + x_index;

        float3 color;
        color.x = 1.0f;
        color.y = 1.0f;
        color.z = 1.0f;
        
        image[index] = color; 
    }
}

export void 
ispc_test_image(uniform float3 image[], 
   const uniform int image_width, 
   const uniform int image_height, 
   const uniform int tile_width,
   const uniform int tile_height)
{
    const uniform int task_x_count = (image_width + tile_width - 1) / tile_width;
    const uniform int task_y_count = (image_height + tile_height - 1) / tile_height;

    launch [task_x_count, task_y_count]
    write_color(image, image_width, image_height, tile_width, tile_height);
}

task void
blend_color(uniform float3 image[], 
   const uniform int image_width, 
   const uniform int image_height, 
   const uniform int tile_width,
   const uniform int tile_height)
{
    const uniform int x_start = taskIndex0 * tile_width;
    const uniform int x_end = min(x_start + tile_width, image_width);
    const uniform int y_start = taskIndex1 * tile_width;
    const uniform int y_end = min(y_start + tile_height, image_height);

    foreach(y_index = y_start ... y_end, x_index = x_start ... x_end)
    {
        int index = y_index * image_width + x_index;

        float3 a_color = image[index];

        float3 b_color;
        b_color.x = 0.8;
        b_color.y = 0.25;
        b_color.z = 0.45;

        float3 result_col;
        result_col.x = a_color.x * 0.5f + b_color.x * 0.5f;
        result_col.y = a_color.y * 0.5f + b_color.y * 0.5f;
        result_col.z = a_color.z * 0.5f + b_color.z * 0.5f;
        
        image[index] = result_col; 
    }
}

export void 
ispc_test_image_operation(uniform float3 image[], 
   const uniform int image_width, 
   const uniform int image_height, 
   const uniform int tile_width,
   const uniform int tile_height)
{
    const uniform int task_x_count = (image_width + tile_width - 1) / tile_width;
    const uniform int task_y_count = (image_height + tile_height - 1) / tile_height;

    launch [task_x_count, task_y_count]
    blend_color(image, image_width, image_height, tile_width, tile_height);
}