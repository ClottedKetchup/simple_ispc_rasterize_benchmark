

#include "simple_math.h"


task void
write_color(uniform Float3 image[], 
   const uniform int image_width, 
   const uniform int image_height, 
   const uniform int tile_width,
   const uniform int tile_height)
{
    const uniform int x_start = taskIndex0 * tile_width;
    const uniform int x_end = min(x_start + tile_width, image_width);
    const uniform int y_start = taskIndex1 * tile_width;
    const uniform int y_end = min(y_start + tile_height, image_height);

    key_uniform const Float3 red_color = make_Float3(1.0f, 0.0f, 0.0f);

    foreach(y_index = y_start ... y_end, x_index = x_start ... x_end)
    {
        int index = y_index * image_width + x_index;

        image[index] = red_color;
    }
}

export void 
ispc_test_image(uniform Float3 image[], 
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

static inline key_uniform int test_qualifier(key_uniform int x, key_uniform int y)
{
    return x + y;
}

static inline key_varying int test_qualifier(key_varying int x, key_varying int y)
{
    return x + y;
}

static key_uniform Float3 test_vec_add(const key_uniform Float3 &a, const key_uniform Float3 &b)
{
    key_uniform Float3 result;
    result.v[0] = a.v[0] + b.v[0];
    result.v[1] = a.v[1] + b.v[1];
    result.v[2] = a.v[2] + b.v[2];
    
    return result;
}

static key_varying Float3 test_vec_add(const key_varying Float3 &a, const key_varying Float3 &b)
{
    key_varying Float3 result;
    result.v[0] = a.v[0] + b.v[0];
    result.v[1] = a.v[1] + b.v[1];
    result.v[2] = a.v[2] + b.v[2];
    
    return result;
}

static void test_set_vec(key_uniform Float3 &color, key_uniform Float3 target)
{
    color.v[0] = target.v[0];
    color.v[1] = target.v[1];
    color.v[2] = target.v[2];
}

static void test_set_vec(key_varying Float3 &color, key_varying Float3 target)
{
    color.v[0] = target.v[0];
    color.v[1] = target.v[1];
    color.v[2] = target.v[2];
}

task void
blend_color(uniform Float3 image[], 
   const uniform int image_width, 
   const uniform int image_height, 
   const uniform int tile_width,
   const uniform int tile_height)
{
    const uniform int x_start = taskIndex0 * tile_width;
    const uniform int x_end = min(x_start + tile_width, image_width);
    const uniform int y_start = taskIndex1 * tile_width;
    const uniform int y_end = min(y_start + tile_height, image_height);

    uniform Float3 red_color = make_Float3(1.0f, 0.0f, 0.0f);
    uniform Float3 blue_color;
    test_set_vec(blue_color, make_Float3(0.0f, 1.0f, 0.0f));

    uniform Float3 pink_color = test_vec_add(red_color, blue_color);

    foreach(y_index = y_start ... y_end, x_index = x_start ... x_end)
    {
        int index = y_index * image_width + x_index;

        Float3 image_color = image[index];

        Float3 color_mix1 = test_vec_add(0.5f * image_color, 0.5f * (key_varying Float3)pink_color);

        Float3 purple_color;
        test_set_vec(purple_color, make_Float3(0.0f, 0.25f, 0.85f));

        Float3 color_mix2 = test_vec_add(0.5f * color_mix1, 0.5f * purple_color);
        
        image[index] = color_mix1; 
    }
}

export void 
ispc_test_image_operation(uniform Float3 image[], 
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



task void
get_triangle_bounding_pixel_count(uniform Float3 image[], 
   const uniform int image_width, 
   const uniform int image_height, 
   const uniform int tile_width,
   const uniform int tile_height,

   const uniform unsigned int batch_triangle_count,
   
   const uniform unsigned int triangle_count,
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],
   
   uniform Pixel* uniform pixel_buffer,
   uniform unsigned int64* uniform pixel_counter)
{
    const uniform int start = taskIndex0 * batch_triangle_count;
    const uniform int end = min(start + batch_triangle_count, triangle_count);
    
    // for this test, just write to pixel buffer.
    foreach(triangle_index = start ... end)
    {
        const Triangle triangle = triangles[triangle_index];

        Float3 world_p0 = vertex_positions[triangle.index_0];
        Float3 world_p1 = vertex_positions[triangle.index_1];
        Float3 world_p2 = vertex_positions[triangle.index_2];

        const Matrix4x4 transform_world_to_NDC = (key_varying Matrix4x4)(*world_to_NDC);

        Float4 p0_NDC = transform_world_to_NDC * make_Float4(world_p0, 1.0f); 
        Float4 p1_NDC = transform_world_to_NDC * make_Float4(world_p1, 1.0f);
        Float4 p2_NDC = transform_world_to_NDC * make_Float4(world_p2, 1.0f); 

        p0_NDC = p0_NDC / p0_NDC.v[3];
        p0_NDC.v[3] = 1.0f;

        p1_NDC = p1_NDC / p1_NDC.v[3];
        p1_NDC.v[3] = 1.0f;

        p2_NDC = p2_NDC / p2_NDC.v[3];
        p2_NDC.v[3] = 1.0f;

        const Float2 p0_NDC_2D = make_Float2(p0_NDC.v[0], p0_NDC.v[1]);
        const Float2 p1_NDC_2D = make_Float2(p1_NDC.v[0], p1_NDC.v[1]);
        const Float2 p2_NDC_2D = make_Float2(p2_NDC.v[0], p2_NDC.v[1]);
        const float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
        if(!abs(triangle_area) > 0.0f){
            continue;
        }

        const Float2 p0_screen = NDC_to_screen(p0_NDC.v[0], p0_NDC.v[1], (key_varying int)image_width, (key_varying int)image_height);
        const Float2 p1_screen = NDC_to_screen(p1_NDC.v[0], p1_NDC.v[1], (key_varying int)image_width, (key_varying int)image_height);
        const Float2 p2_screen = NDC_to_screen(p2_NDC.v[0], p2_NDC.v[1], (key_varying int)image_width, (key_varying int)image_height);

        BoundingBox2 box = make_bounding_box2_varying();
        box = extend_bounding_box2(box, p0_screen);
        box = extend_bounding_box2(box, p1_screen);
        box = extend_bounding_box2(box, p2_screen);

        float x_start = max(0.0f, box.x_min);
        float x_end = min((key_varying float)image_width, box.x_max);
        float y_start = max(0.0f, box.y_min);
        float y_end = min((key_varying float)image_height, box.y_max);

        for (int x = (key_varying int)floor(x_start); x <= (key_varying int)ceil(x_end); ++x) {
            for (int y = (key_varying int)floor(y_start); y <= (key_varying int)ceil(y_end); ++y) {
                if (x >= 0 && x < (key_varying int)image_width && y >= 0 && y < (key_varying int)image_height) {
                    Float2 pixel_center = make_Float2((key_varying float)x + 0.5f, (key_varying float)y + 0.5f);
                    Float2 pixel_center_NDC = screen_to_NDC(pixel_center.v[0], pixel_center.v[1], (key_varying int)image_width, (key_varying int)image_height);
                    if(point_inside_triangle(pixel_center_NDC, p0_NDC_2D, p1_NDC_2D, p2_NDC_2D)) {
                        float cross_p0p_p0p1 = cross(pixel_center_NDC - p0_NDC_2D, p1_NDC_2D - p0_NDC_2D);
                        float cross_p1p_p1p2 = cross(pixel_center_NDC - p1_NDC_2D, p2_NDC_2D - p1_NDC_2D);
                        float cross_p2p_p0p2 = cross(pixel_center_NDC - p2_NDC_2D, p0_NDC_2D - p2_NDC_2D);

                        float b0_NDC = abs(cross_p1p_p1p2) / abs(triangle_area);
                        float b1_NDC = abs(cross_p2p_p0p2) / abs(triangle_area);
                        float b2_NDC = abs(cross_p0p_p0p1) / abs(triangle_area);

                        float weighted_b0_NDC = b0_NDC / world_p0.v[2];
                        float weighted_b1_NDC = b1_NDC / world_p1.v[2];
                        float weighted_b2_NDC = b2_NDC / world_p2.v[2];
                        float weighted_b_sum = weighted_b0_NDC + weighted_b1_NDC + weighted_b2_NDC;

                        float b0_world = weighted_b0_NDC / weighted_b_sum;
                        float b1_world = weighted_b1_NDC / weighted_b_sum;
                        float b2_world = weighted_b2_NDC / weighted_b_sum;

                        Float3 interpolated_color = b0_world * (key_varying Float3)vertex_colors[triangle.index_0] + 
                                                    b1_world * (key_varying Float3)vertex_colors[triangle.index_1] + 
                                                    b2_world * (key_varying Float3)vertex_colors[triangle.index_2];

                        
                        // TODO: depth test.
                        int index = y * image_width + x;
                        image[index] = interpolated_color; 
                    }

                    
                }
            }
        }
    }
}

export void
ispc_test_render_triangle(uniform Float3 image[], 
   const uniform int image_width, 
   const uniform int image_height, 
   const uniform int tile_width,
   const uniform int tile_height, 
   
   const uniform unsigned int triangle_count,
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],
   
   uniform Pixel* uniform pixel_buffer,
   uniform unsigned int64* uniform pixel_counter)
{
    const uniform unsigned int batch_triangle_count = 16;
    const uniform unsigned int task_count = (triangle_count + batch_triangle_count - 1) / batch_triangle_count;

    launch [task_count]
    get_triangle_bounding_pixel_count(image,
        image_width, 
        image_height,
        tile_width,
        tile_height,

        batch_triangle_count,
   
        triangle_count,
        triangles,
        vertex_positions,
        vertex_colors,

        world_to_NDC,
   
        pixel_buffer,
        pixel_counter);
}