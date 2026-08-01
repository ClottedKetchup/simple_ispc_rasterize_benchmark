

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
    const uniform int y_start = taskIndex1 * tile_height;
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


task void
task_get_pixel_count(const uniform int image_width, 
   const uniform int image_height, 

   const uniform unsigned int batch_triangle_count,
   
   const uniform unsigned int triangle_count,
   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],
   uniform unsigned int* uniform pixel_counter)
{
    const uniform unsigned int start = taskIndex0 * batch_triangle_count;
    const uniform unsigned int end = min(start + batch_triangle_count, triangle_count);
    
    foreach(triangle_index = start ... end)
    {
        const Triangle triangle = triangles[triangle_index];
        const Mesh mesh = meshes[triangle.mesh_index];

        const Float3* mesh_vertex_positions = vertex_positions + mesh.vertex_positions_offset;
        const Float3* mesh_vertex_colors = vertex_colors + mesh.vertex_colors_offset;

        Float3 p0_world = mesh_vertex_positions[triangle.index_0];
        Float3 p1_world = mesh_vertex_positions[triangle.index_1];
        Float3 p2_world = mesh_vertex_positions[triangle.index_2];

        const Matrix4x4 mvp = (key_varying Matrix4x4)(*world_to_NDC);

        Float4 p0_homo = mvp * make_Float4(p0_world, 1.0f);
        Float4 p1_homo = mvp * make_Float4(p1_world, 1.0f);
        Float4 p2_homo = mvp * make_Float4(p2_world, 1.0f);

        // note: clip against near plane.
        Float4 src_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
        Float4 dst_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
        
        src_vertices[0] = p0_homo;
        src_vertices[1] = p1_homo;
        src_vertices[2] = p2_homo;

        int dst_count = 0;
        clip_triangle_near_plane(src_vertices, dst_vertices, &dst_count);
        if(!(dst_count >= 3)) {
            continue;
        }

        const int clipped_triangle_count = dst_count == 3 ? 1 : 2;
        for(int clipped_triangle_index = 0; clipped_triangle_index < clipped_triangle_count; ++clipped_triangle_index)
        {
            const int clipped_v0_index = 0;
            const int clipped_v1_index = clipped_triangle_index == 0 ? 1 : 2;
            const int clipped_v2_index = clipped_triangle_index == 0 ? 2 : 3;

            const Float2 p0_NDC_2D = make_Float2(dst_vertices[clipped_v0_index].v[0] / dst_vertices[clipped_v0_index].v[3], 
                dst_vertices[clipped_v0_index].v[1] / dst_vertices[clipped_v0_index].v[3]);
            const Float2 p1_NDC_2D = make_Float2(dst_vertices[clipped_v1_index].v[0] / dst_vertices[clipped_v1_index].v[3], 
                dst_vertices[clipped_v1_index].v[1] / dst_vertices[clipped_v1_index].v[3]);
            const Float2 p2_NDC_2D = make_Float2(dst_vertices[clipped_v2_index].v[0] / dst_vertices[clipped_v2_index].v[3], 
                dst_vertices[clipped_v2_index].v[1] / dst_vertices[clipped_v2_index].v[3]);
            
            const float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
            if(!(abs(triangle_area) > 0.0f)) 
            {
                continue;
            }

            const Float2 p0_screen = NDC_to_screen(p0_NDC_2D.v[0], p0_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);
            const Float2 p1_screen = NDC_to_screen(p1_NDC_2D.v[0], p1_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);
            const Float2 p2_screen = NDC_to_screen(p2_NDC_2D.v[0], p2_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);

            BoundingBox2 box = make_bounding_box2_varying();
            box = extend_bounding_box2(box, p0_screen);
            box = extend_bounding_box2(box, p1_screen);
            box = extend_bounding_box2(box, p2_screen);

            float x_start = max(0.0f, box.x_min);
            float x_end = min((key_varying float)image_width, box.x_max);
            float y_start = max(0.0f, box.y_min);
            float y_end = min((key_varying float)image_height, box.y_max);

            int x_count = ((key_varying int)ceil(x_end)) - ((key_varying int)floor(x_start));
            int y_count = ((key_varying int)ceil(y_end)) - ((key_varying int)floor(y_start));

            atomic_add_global(pixel_counter, (key_varying unsigned int)(x_count * y_count));
        }
        
    }

}

// note: pre-calculate pixel count for reserve space.
export void
ispc_get_pixel_count(const uniform int image_width, 
   const uniform int image_height, 
   
   const uniform unsigned int triangle_count,
   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],
   uniform unsigned int* uniform pixel_counter)
{
    const uniform unsigned int batch_triangle_count = 16;
    const uniform unsigned int task_count = (triangle_count + batch_triangle_count - 1) / batch_triangle_count;

    launch [task_count]
    task_get_pixel_count(image_width, 
        image_height, 

        batch_triangle_count,
        
        triangle_count,
        meshes,
        triangles,
        vertex_positions,
        vertex_colors,

        world_to_NDC,
        pixel_counter);
}

task void
task_rasterize_triangle(const uniform int image_width, 
   const uniform int image_height,
   uniform uint64 depth_buffer[], 
   
   const uniform unsigned int batch_triangle_count,
   const uniform unsigned int triangle_count,

   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],

   const uniform unsigned int max_pixel_count,
   uniform unsigned int* uniform pixel_counter,
   uniform Pixel pixel_buffer[])
{
    const uniform unsigned int start = taskIndex0 * batch_triangle_count;
    const uniform unsigned int end = min(start + batch_triangle_count, triangle_count);
    
    foreach(triangle_index = start ... end)
    {
        const Triangle triangle = triangles[triangle_index];
        const Mesh mesh = meshes[triangle.mesh_index];

        const Float3* mesh_vertex_positions = vertex_positions + mesh.vertex_positions_offset;
        const Float3* mesh_vertex_colors = vertex_colors + mesh.vertex_colors_offset;

        Float3 p0_world = mesh_vertex_positions[triangle.index_0];
        Float3 p1_world = mesh_vertex_positions[triangle.index_1];
        Float3 p2_world = mesh_vertex_positions[triangle.index_2];
        
        // note: transform vertices.
        const Matrix4x4 mvp = (key_varying Matrix4x4)(*world_to_NDC);
        Float4 p0_homo = mvp * make_Float4(p0_world, 1.0f);
        Float4 p1_homo = mvp * make_Float4(p1_world, 1.0f);
        Float4 p2_homo = mvp * make_Float4(p2_world, 1.0f);

        // note: clip against near plane.
        Float4 src_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
        Float4 dst_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
        
        src_vertices[0] = p0_homo;
        src_vertices[1] = p1_homo;
        src_vertices[2] = p2_homo;

        int dst_count = 0;
        clip_triangle_near_plane(src_vertices, dst_vertices, &dst_count);
        if(!(dst_count >= 3)) {
            continue;
        }

        // note: rasterize and write pixel to buffer.
        const int clipped_triangle_count = dst_count == 3 ? 1 : 2;
        for(int clipped_triangle_index = 0; clipped_triangle_index < clipped_triangle_count; ++clipped_triangle_index)
        {
            const int clipped_v0_index = 0;
            const int clipped_v1_index = clipped_triangle_index == 0 ? 1 : 2;
            const int clipped_v2_index = clipped_triangle_index == 0 ? 2 : 3;

            const Float2 p0_NDC_2D = make_Float2(dst_vertices[clipped_v0_index].v[0] / dst_vertices[clipped_v0_index].v[3], 
                dst_vertices[clipped_v0_index].v[1] / dst_vertices[clipped_v0_index].v[3]);
            const Float2 p1_NDC_2D = make_Float2(dst_vertices[clipped_v1_index].v[0] / dst_vertices[clipped_v1_index].v[3], 
                dst_vertices[clipped_v1_index].v[1] / dst_vertices[clipped_v1_index].v[3]);
            const Float2 p2_NDC_2D = make_Float2(dst_vertices[clipped_v2_index].v[0] / dst_vertices[clipped_v2_index].v[3], 
                dst_vertices[clipped_v2_index].v[1] / dst_vertices[clipped_v2_index].v[3]);
            
            const float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
            if(!(abs(triangle_area) > 0.0f)) 
            {
                continue;
            }

            const Float2 p0_screen = NDC_to_screen(p0_NDC_2D.v[0], p0_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);
            const Float2 p1_screen = NDC_to_screen(p1_NDC_2D.v[0], p1_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);
            const Float2 p2_screen = NDC_to_screen(p2_NDC_2D.v[0], p2_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);

            BoundingBox2 box = make_bounding_box2_varying();
            box = extend_bounding_box2(box, p0_screen);
            box = extend_bounding_box2(box, p1_screen);
            box = extend_bounding_box2(box, p2_screen);

            const int x_start = (key_varying int)floor(max(0.0f, box.x_min));
            const int x_end = (key_varying int)ceil(min((key_varying float)image_width, box.x_max));
            const int y_start = (key_varying int)floor(max(0.0f, box.y_min));
            const int y_end = (key_varying int)ceil(min((key_varying float)image_height, box.y_max));

            for(int y = y_start; y < y_end; ++y)
            {
                for(int x = x_start; x < x_end; ++x)
                {
                    if(x < 0 || x >= (key_varying int)image_width || y < 0 || y >= (key_varying int)image_height)
                    {
                        continue;
                    }

                    Float2 pixel_center = make_Float2((key_varying float)x + 0.5f, (key_varying float)y + 0.5f);
                    Float2 pixel_center_NDC = screen_to_NDC(pixel_center.v[0], pixel_center.v[1], (key_varying int)image_width, (key_varying int)image_height);

                    if(!point_inside_triangle_int(pixel_center, p0_screen, p1_screen, p2_screen)) 
                    {
                        continue;
                    }

                    float cross_p0p_p0p1 = cross(pixel_center_NDC - p0_NDC_2D, p1_NDC_2D - p0_NDC_2D);
                    float cross_p1p_p1p2 = cross(pixel_center_NDC - p1_NDC_2D, p2_NDC_2D - p1_NDC_2D);
                    float cross_p2p_p0p2 = cross(pixel_center_NDC - p2_NDC_2D, p0_NDC_2D - p2_NDC_2D);

                    float b0_NDC = abs(cross_p1p_p1p2) / abs(triangle_area);
                    float b1_NDC = abs(cross_p2p_p0p2) / abs(triangle_area);
                    float b2_NDC = abs(cross_p0p_p0p1) / abs(triangle_area);

                    float weighted_b0_NDC = b0_NDC / dst_vertices[clipped_v0_index].v[3];
                    float weighted_b1_NDC = b1_NDC / dst_vertices[clipped_v1_index].v[3];
                    float weighted_b2_NDC = b2_NDC / dst_vertices[clipped_v2_index].v[3];
                    float weighted_b_sum = weighted_b0_NDC + weighted_b1_NDC + weighted_b2_NDC;

                    if(!(abs(weighted_b_sum) > 0.0f))
                    {
                        continue;
                    }

                    float inv_weight_sum = 1.0f / weighted_b_sum;

                    float b0_world = weighted_b0_NDC * inv_weight_sum;
                    float b1_world = weighted_b1_NDC * inv_weight_sum;
                    float b2_world = max(1.0f - b0_world - b1_world, 0.0f); 

                    float interpolated_x = dst_vertices[clipped_v0_index].v[0] * b0_world +
                                        dst_vertices[clipped_v1_index].v[0] * b1_world +
                                        dst_vertices[clipped_v2_index].v[0] * b2_world;

                    float interpolated_y = dst_vertices[clipped_v0_index].v[1] * b0_world +
                                        dst_vertices[clipped_v1_index].v[1] * b1_world +
                                        dst_vertices[clipped_v2_index].v[1] * b2_world;

                    float interpolated_z = dst_vertices[clipped_v0_index].v[2] * b0_world +
                                        dst_vertices[clipped_v1_index].v[2] * b1_world +
                                        dst_vertices[clipped_v2_index].v[2] * b2_world;

                    float interpolated_w = dst_vertices[clipped_v0_index].v[3] * b0_world +
                                        dst_vertices[clipped_v1_index].v[3] * b1_world +
                                        dst_vertices[clipped_v2_index].v[3] * b2_world;

                    float depth = interpolated_z / interpolated_w;

                    unsigned int pixel_index = atomic_add_global(pixel_counter, (key_varying unsigned int)1u);
                    assert(pixel_index < max_pixel_count);

                    const uint64 depth_bits_high = float_to_uint(depth);
                    const uint64 depth_bits_low = pixel_index;
                    const uint64 depth_bits = (depth_bits_high << 32) | (depth_bits_low);

                    const int image_pixel_index = y * image_width + x;

                    atomic_min_global(&(depth_buffer[image_pixel_index]), (key_varying uint64)depth_bits);

                    Float3 interpolated_p = make_Float3(interpolated_x, interpolated_y, interpolated_z);
                    Float3 p0_homo_3 = make_Float3(p0_homo.v[0], p0_homo.v[1], p0_homo.v[2]);
                    Float3 p1_homo_3 = make_Float3(p1_homo.v[0], p1_homo.v[1], p1_homo.v[2]);
                    Float3 p2_homo_3 = make_Float3(p2_homo.v[0], p2_homo.v[1], p2_homo.v[2]);
                    
                    float original_triangle_area = length(cross(p1_homo_3 - p0_homo_3, p2_homo_3 - p1_homo_3)); 
                    float original_p0p_x_p0p1 = length(cross(interpolated_p - p0_homo_3, p1_homo_3 - p0_homo_3));
                    float original_p1p_x_p1p2 = length(cross(interpolated_p - p1_homo_3, p2_homo_3 - p1_homo_3));
                    float original_p2p_x_p0p2 = length(cross(interpolated_p - p2_homo_3, p0_homo_3 - p2_homo_3)); 

                    float original_b0_world = original_p1p_x_p1p2 / original_triangle_area;
                    float original_b1_world = original_p2p_x_p0p2 / original_triangle_area;
                    float original_b2_world = max(1.0f - original_b0_world - original_b1_world, 0.0f);

                    pixel_buffer[pixel_index].screen_x = pixel_center.v[0];
                    pixel_buffer[pixel_index].screen_y = pixel_center.v[1];
                    pixel_buffer[pixel_index].triangle_index = triangle_index;
                    pixel_buffer[pixel_index].b0_world = original_b0_world;
                    pixel_buffer[pixel_index].b1_world = original_b1_world;
                    pixel_buffer[pixel_index].b2_world = original_b2_world;
                }
            }
        }
    }

}

export void
ispc_rasterize_triangle(const uniform int image_width, 
   const uniform int image_height,
   uniform uint64 depth_buffer[], 
   
   const uniform unsigned int triangle_count,

   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],

   const uniform unsigned int max_pixel_count,
   uniform unsigned int* uniform pixel_counter,
   uniform Pixel pixel_buffer[])
{
    const uniform unsigned int batch_triangle_count = 16;
    const uniform unsigned int task_count = (triangle_count + batch_triangle_count - 1) / batch_triangle_count;

    launch [task_count]
    task_rasterize_triangle(image_width, 
        image_height,
        depth_buffer, 
        
        batch_triangle_count,
        triangle_count,

        meshes,
        triangles,
        vertex_positions,
        vertex_colors,

        world_to_NDC,

        max_pixel_count,
        pixel_counter,
        pixel_buffer);
}

task void
task_shading_pixel(const uniform int image_width, 
   const uniform int image_height,
   const uniform int tile_width,
   const uniform int tile_height,

   const uniform uint64 depth_buffer[],
   uniform Float3 image[],
   
   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],

   const uniform unsigned int pixel_count,
   const uniform Pixel pixel_buffer[])
{
    const uniform int x_start = taskIndex0 * tile_width;
    const uniform int x_end = min(x_start + tile_width, image_width);
    const uniform int y_start = taskIndex1 * tile_height;
    const uniform int y_end = min(y_start + tile_height, image_height);

    foreach(y_index = y_start ... y_end, x_index = x_start ... x_end)
    {
        const int image_pixel_index = y_index * image_width + x_index;

        const uint64 depth_bits = depth_buffer[image_pixel_index];
        const uint64 depth_bits_low = depth_bits & 0x00000000FFFFFFFF;
        const uint32 pixel_index = depth_bits_low;
        if(pixel_index == EMPTY_INDEX_32)
        {
            image[image_pixel_index] = make_Float3(0.0f);
            continue;
        }

        assert(pixel_index < pixel_count);

        const Pixel pixel = pixel_buffer[pixel_index];

        const Triangle triangle = triangles[pixel.triangle_index];
        const Mesh mesh = meshes[triangle.mesh_index];

        if(mesh.vertex_colors_offset == EMPTY_INDEX_32)
        {
            image[image_pixel_index] = make_Float3(0.0f);
            continue;
        }

        const Float3* mesh_vertex_positions = vertex_positions + mesh.vertex_positions_offset;
        const Float3* mesh_vertex_colors = vertex_colors + mesh.vertex_colors_offset;

        Float3 interpolated_color = pixel.b0_world * (key_varying Float3)mesh_vertex_colors[triangle.index_0] + 
                                    pixel.b1_world * (key_varying Float3)mesh_vertex_colors[triangle.index_1] + 
                                    pixel.b2_world * (key_varying Float3)mesh_vertex_colors[triangle.index_2];

        image[image_pixel_index] = interpolated_color;
    }
}


export void
ispc_shading_pixel(const uniform int image_width, 
   const uniform int image_height,
   const uniform int tile_width,
   const uniform int tile_height,

   const uniform uint64 depth_buffer[],
   uniform Float3 image[],
   
   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],

   const uniform unsigned int pixel_count,
   const uniform Pixel pixel_buffer[])
{
    const uniform int task_x_count = (image_width + tile_width - 1) / tile_width;
    const uniform int task_y_count = (image_height + tile_height - 1) / tile_height;

    launch [task_x_count, task_y_count]
    task_shading_pixel(image_width, 
        image_height,
        tile_width,
        tile_height,

        depth_buffer,
        image,
        
        meshes,
        triangles,
        vertex_positions,
        vertex_colors,

        world_to_NDC,

        pixel_count,
        pixel_buffer);
}


task void
task_get_tile_triangle_count(const uniform int image_width, 
   const uniform int image_height,
   const uniform int tile_width,
   const uniform int tile_height,
   const uniform int tile_count_x,
   const uniform int tile_count_y, 

   const uniform unsigned int batch_triangle_count,
   
   const uniform unsigned int triangle_count,
   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],
   uniform unsigned int tile_triangle_counts[],
   uniform TileIndexRange triangle_tile_ranges[],
   uniform BoundingBox2 triangle_screen_boundings[])
{
    const uniform unsigned int start = taskIndex0 * batch_triangle_count;
    const uniform unsigned int end = min(start + batch_triangle_count, triangle_count);
    
    foreach(triangle_index = start ... end)
    {
        const Triangle triangle = triangles[triangle_index];
        const Mesh mesh = meshes[triangle.mesh_index];

        const Float3* mesh_vertex_positions = vertex_positions + mesh.vertex_positions_offset;
        const Float3* mesh_vertex_colors = vertex_colors + mesh.vertex_colors_offset;

        Float3 p0_world = mesh_vertex_positions[triangle.index_0];
        Float3 p1_world = mesh_vertex_positions[triangle.index_1];
        Float3 p2_world = mesh_vertex_positions[triangle.index_2];

        const Matrix4x4 mvp = (key_varying Matrix4x4)(*world_to_NDC);

        Float4 p0_homo = mvp * make_Float4(p0_world, 1.0f);
        Float4 p1_homo = mvp * make_Float4(p1_world, 1.0f);
        Float4 p2_homo = mvp * make_Float4(p2_world, 1.0f);

        // note: clip against near plane.
        Float4 src_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
        Float4 dst_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
        
        src_vertices[0] = p0_homo;
        src_vertices[1] = p1_homo;
        src_vertices[2] = p2_homo;

        int dst_count = 0;
        clip_triangle_near_plane(src_vertices, dst_vertices, &dst_count);
        if(!(dst_count >= 3)) {
            continue;
        }

        BoundingBox2 triangle_screen_box = make_bounding_box2_varying();
        BoundingBox2 tile_index_box = make_bounding_box2_varying();

        const int clipped_triangle_count = dst_count == 3 ? 1 : 2;
        for(int clipped_triangle_index = 0; clipped_triangle_index < clipped_triangle_count; ++clipped_triangle_index)
        {
            const int clipped_v0_index = 0;
            const int clipped_v1_index = clipped_triangle_index == 0 ? 1 : 2;
            const int clipped_v2_index = clipped_triangle_index == 0 ? 2 : 3;

            const Float2 p0_NDC_2D = make_Float2(dst_vertices[clipped_v0_index].v[0] / dst_vertices[clipped_v0_index].v[3], 
                dst_vertices[clipped_v0_index].v[1] / dst_vertices[clipped_v0_index].v[3]);
            const Float2 p1_NDC_2D = make_Float2(dst_vertices[clipped_v1_index].v[0] / dst_vertices[clipped_v1_index].v[3], 
                dst_vertices[clipped_v1_index].v[1] / dst_vertices[clipped_v1_index].v[3]);
            const Float2 p2_NDC_2D = make_Float2(dst_vertices[clipped_v2_index].v[0] / dst_vertices[clipped_v2_index].v[3], 
                dst_vertices[clipped_v2_index].v[1] / dst_vertices[clipped_v2_index].v[3]);
            
            const float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
            if(!(abs(triangle_area) > 0.0f)) 
            {
                continue;
            }

            const Float2 p0_screen = NDC_to_screen(p0_NDC_2D.v[0], p0_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);
            const Float2 p1_screen = NDC_to_screen(p1_NDC_2D.v[0], p1_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);
            const Float2 p2_screen = NDC_to_screen(p2_NDC_2D.v[0], p2_NDC_2D.v[1], (key_varying int)image_width, (key_varying int)image_height);

            triangle_screen_box = extend_bounding_box2(triangle_screen_box, p0_screen);
            triangle_screen_box = extend_bounding_box2(triangle_screen_box, p1_screen);
            triangle_screen_box = extend_bounding_box2(triangle_screen_box, p2_screen);

            const Float2 p0_tile_coordinate = make_Float2(p0_screen.v[0] / (key_varying float)tile_width, p0_screen.v[1] / (key_varying float)tile_height);
            const Float2 p1_tile_coordinate = make_Float2(p1_screen.v[0] / (key_varying float)tile_width, p1_screen.v[1] / (key_varying float)tile_height);
            const Float2 p2_tile_coordinate = make_Float2(p2_screen.v[0] / (key_varying float)tile_width, p2_screen.v[1] / (key_varying float)tile_height);
            
            tile_index_box = extend_bounding_box2(tile_index_box, p0_tile_coordinate);
            tile_index_box = extend_bounding_box2(tile_index_box, p1_tile_coordinate);
            tile_index_box = extend_bounding_box2(tile_index_box, p2_tile_coordinate);
        }

        triangle_screen_boundings[triangle_index] = triangle_screen_box;

        int tile_x_start = max((int)floor(tile_index_box.x_min), 0);
        int tile_x_end = min((int)floor(tile_index_box.x_max), tile_count_x - 1);
        int tile_y_start = max((int)floor(tile_index_box.y_min), 0);
        int tile_y_end = min((int)floor(tile_index_box.y_max), tile_count_y - 1);

        TileIndexRange tile_range;
        tile_range.tile_x_start = tile_x_start;
        tile_range.tile_x_end = tile_x_end;
        tile_range.tile_y_start = tile_y_start;
        tile_range.tile_y_end = tile_y_end;

        triangle_tile_ranges[triangle_index] = tile_range;

        for(int y = tile_y_start; y <= tile_y_end; ++y)
        {
            for(int x = tile_x_start; x <= tile_x_end; ++x)
            {
                int tile_index = y * tile_count_x + x;
                atomic_add_global(&(tile_triangle_counts[tile_index]), (key_varying unsigned int)(1));
            }
        }
    }
}

export void
ispc_get_tile_triangle_count(const uniform int image_width, 
   const uniform int image_height,
   const uniform int tile_width,
   const uniform int tile_height,
   const uniform int tile_count_x,
   const uniform int tile_count_y,
   
   const uniform unsigned int triangle_count,
   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],
   uniform unsigned int tile_triangle_counts[],
   uniform TileIndexRange triangle_tile_ranges[],
   uniform BoundingBox2 triangle_screen_boundings[])
{
    const uniform unsigned int batch_triangle_count = 16;
    const uniform unsigned int task_count = (triangle_count + batch_triangle_count - 1) / batch_triangle_count;

    launch [task_count]
    task_get_tile_triangle_count(image_width, 
        image_height,
        tile_width,
        tile_height,
        tile_count_x,
        tile_count_y, 

        batch_triangle_count,
        
        triangle_count,
        meshes,
        triangles,
        vertex_positions,
        vertex_colors,

        world_to_NDC,
        tile_triangle_counts,
        triangle_tile_ranges,
        triangle_screen_boundings);
}

task void 
task_simple_tile_shading(const uniform int image_width, 
   const uniform int image_height,
   const uniform int tile_width,
   const uniform int tile_height,
   const uniform int tile_count_x,
   const uniform int tile_count_y,

   uniform float depth_buffer[],
   uniform Float3 image[],
   
   const uniform unsigned int triangle_count,
   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],
   const uniform unsigned int tile_triangle_counts[],
   const uniform unsigned int tile_triangle_data_offsets[],
   const uniform BoundingBox2 triangle_screen_boundings[],
   const uniform unsigned int tile_triangle_data[])
{
    const uniform int tile_index = taskIndex1 * tile_count_x + taskIndex0;
    const uniform int tile_triangle_count = tile_triangle_counts[tile_index];
    const uniform int tile_data_offset = tile_triangle_data_offsets[tile_index];

    const uniform int x_start = taskIndex0 * tile_width;
    const uniform int x_end = min(x_start + tile_width, image_width);
    const uniform int y_start = taskIndex1 * tile_height;
    const uniform int y_end = min(y_start + tile_height, image_height);

    uniform BoundingBox2 tile_screen_box = make_bounding_box2_uniform();
    tile_screen_box.x_min = (key_uniform float)x_start;
    tile_screen_box.x_max = (key_uniform float)x_end;
    tile_screen_box.y_min = (key_uniform float)y_start;
    tile_screen_box.y_max = (key_uniform float)y_end;

    for(uniform int tile_triangle_index = 0; tile_triangle_index < tile_triangle_count; ++tile_triangle_index)
    {
        const uniform int triangle_index = tile_triangle_data[tile_data_offset + tile_triangle_index];
        if(!(triangle_index < triangle_count)){
            continue;
        }

        const uniform Triangle triangle = triangles[triangle_index];
        const uniform Mesh mesh = meshes[triangle.mesh_index];

        const uniform Float3* uniform mesh_vertex_positions = vertex_positions + mesh.vertex_positions_offset;
        const uniform Float3* uniform mesh_vertex_colors = vertex_colors + mesh.vertex_colors_offset;

        const uniform Float3 p0_world = mesh_vertex_positions[triangle.index_0];
        const uniform Float3 p1_world = mesh_vertex_positions[triangle.index_1];
        const uniform Float3 p2_world = mesh_vertex_positions[triangle.index_2];
        
        // note: transform vertices.
        const uniform Matrix4x4 mvp = (*world_to_NDC);
        uniform Float4 p0_homo = mvp * make_Float4(p0_world, 1.0f);
        uniform Float4 p1_homo = mvp * make_Float4(p1_world, 1.0f);
        uniform Float4 p2_homo = mvp * make_Float4(p2_world, 1.0f);

        // note: clip against near plane.
        uniform Float4 src_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
        uniform Float4 dst_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
        
        src_vertices[0] = p0_homo;
        src_vertices[1] = p1_homo;
        src_vertices[2] = p2_homo;

        uniform int dst_count = 0;
        clip_triangle_near_plane(src_vertices, dst_vertices, &dst_count);
        if(!(dst_count >= 3)) {
            continue;
        }


        const uniform int clipped_triangle_count = dst_count == 3 ? 1 : 2;
        for(uniform int clipped_triangle_index = 0; clipped_triangle_index < clipped_triangle_count; ++clipped_triangle_index)
        {
            const uniform int clipped_v0_index = 0;
            const uniform int clipped_v1_index = clipped_triangle_index == 0 ? 1 : 2;
            const uniform int clipped_v2_index = clipped_triangle_index == 0 ? 2 : 3;

            const uniform Float2 p0_NDC_2D = make_Float2(dst_vertices[clipped_v0_index].v[0] / dst_vertices[clipped_v0_index].v[3], 
                dst_vertices[clipped_v0_index].v[1] / dst_vertices[clipped_v0_index].v[3]);
            const uniform Float2 p1_NDC_2D = make_Float2(dst_vertices[clipped_v1_index].v[0] / dst_vertices[clipped_v1_index].v[3], 
                dst_vertices[clipped_v1_index].v[1] / dst_vertices[clipped_v1_index].v[3]);
            const uniform Float2 p2_NDC_2D = make_Float2(dst_vertices[clipped_v2_index].v[0] / dst_vertices[clipped_v2_index].v[3], 
                dst_vertices[clipped_v2_index].v[1] / dst_vertices[clipped_v2_index].v[3]);
            
            const uniform float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
            if(!(abs(triangle_area) > 0.0f)) {
                continue;
            }

            const uniform Float2 p0_screen = NDC_to_screen(p0_NDC_2D.v[0], p0_NDC_2D.v[1], image_width, image_height);
            const uniform Float2 p1_screen = NDC_to_screen(p1_NDC_2D.v[0], p1_NDC_2D.v[1], image_width, image_height);
            const uniform Float2 p2_screen = NDC_to_screen(p2_NDC_2D.v[0], p2_NDC_2D.v[1], image_width, image_height);

            uniform BoundingBox2 clipped_triangle_box = make_bounding_box2_uniform();
            clipped_triangle_box = extend_bounding_box2(clipped_triangle_box, p0_screen);
            clipped_triangle_box = extend_bounding_box2(clipped_triangle_box, p1_screen);
            clipped_triangle_box = extend_bounding_box2(clipped_triangle_box, p2_screen);

            uniform BoundingBox2 range_box = bounding_box2_intersect(tile_screen_box, clipped_triangle_box);
            uniform int pixel_range_x_start = (key_uniform int)floor(range_box.x_min);
            uniform int pixel_range_x_end = (key_uniform int)ceil(range_box.x_max);
            uniform int pixel_range_y_start = (key_uniform int)floor(range_box.y_min);
            uniform int pixel_range_y_end = (key_uniform int)ceil(range_box.y_max);

            foreach(x = pixel_range_x_start ... pixel_range_x_end, y = pixel_range_y_start ... pixel_range_y_end)
            {
                if(x < 0 || x >= (key_varying int)image_width || y < 0 || y >= (key_varying int)image_height) {
                    continue;
                }

                Float2 pixel_center = make_Float2((key_varying float)x + 0.5f, (key_varying float)y + 0.5f);
                Float2 pixel_center_NDC = screen_to_NDC(pixel_center.v[0], pixel_center.v[1], (key_varying int)image_width, (key_varying int)image_height);
                if(!point_inside_triangle_int(pixel_center, (key_varying Float2)p0_screen, (key_varying Float2)p1_screen, (key_varying Float2)p2_screen)) {
                    continue;
                }

                float cross_p0p_p0p1 = cross(pixel_center_NDC - (key_varying Float2)p0_NDC_2D, (key_varying Float2)p1_NDC_2D - (key_varying Float2)p0_NDC_2D);
                float cross_p1p_p1p2 = cross(pixel_center_NDC - (key_varying Float2)p1_NDC_2D, (key_varying Float2)p2_NDC_2D - (key_varying Float2)p1_NDC_2D);
                float cross_p2p_p0p2 = cross(pixel_center_NDC - (key_varying Float2)p2_NDC_2D, (key_varying Float2)p0_NDC_2D - (key_varying Float2)p2_NDC_2D);

                float b0_NDC = abs(cross_p1p_p1p2) / abs((key_varying float)triangle_area);
                float b1_NDC = abs(cross_p2p_p0p2) / abs((key_varying float)triangle_area);
                float b2_NDC = abs(cross_p0p_p0p1) / abs((key_varying float)triangle_area);

                float weighted_b0_NDC = b0_NDC / (key_varying float)(dst_vertices[clipped_v0_index].v[3]);
                float weighted_b1_NDC = b1_NDC / (key_varying float)(dst_vertices[clipped_v1_index].v[3]);
                float weighted_b2_NDC = b2_NDC / (key_varying float)(dst_vertices[clipped_v2_index].v[3]);
                float weighted_b_sum = weighted_b0_NDC + weighted_b1_NDC + weighted_b2_NDC;

                if(!(abs(weighted_b_sum) > 0.0f)) {
                    continue;
                }

                int image_pixel_index = y * image_width + x;

                float inv_weight_sum = 1.0f / weighted_b_sum;

                float b0_world = weighted_b0_NDC * inv_weight_sum;
                float b1_world = weighted_b1_NDC * inv_weight_sum;
                float b2_world = max(1.0f - b0_world - b1_world, 0.0f); 

                float interpolated_z = (key_varying float)dst_vertices[clipped_v0_index].v[2] * b0_world +
                                    (key_varying float)dst_vertices[clipped_v1_index].v[2] * b1_world +
                                    (key_varying float)dst_vertices[clipped_v2_index].v[2] * b2_world;

                float interpolated_w = (key_varying float)dst_vertices[clipped_v0_index].v[3] * b0_world +
                                    (key_varying float)dst_vertices[clipped_v1_index].v[3] * b1_world +
                                    (key_varying float)dst_vertices[clipped_v2_index].v[3] * b2_world;

                float depth = interpolated_z / interpolated_w;

                if(depth < depth_buffer[image_pixel_index])
                {
                    depth_buffer[image_pixel_index] = depth;
                    
                    Float3 interpolated_color = b0_world * (key_varying Float3)mesh_vertex_colors[triangle.index_0] + 
                                    b1_world * (key_varying Float3)mesh_vertex_colors[triangle.index_1] + 
                                    b2_world * (key_varying Float3)mesh_vertex_colors[triangle.index_2];

                     image[image_pixel_index] = interpolated_color;
                }
            }
        }
        
    }
}

export void
ispc_simple_tile_shading(const uniform int image_width, 
   const uniform int image_height,
   const uniform int tile_width,
   const uniform int tile_height,
   const uniform int tile_count_x,
   const uniform int tile_count_y,

   uniform float depth_buffer[],
   uniform Float3 image[],
   
   const uniform unsigned int triangle_count,
   const uniform Mesh meshes[],
   const uniform Triangle triangles[],
   const uniform Float3 vertex_positions[],
   const uniform Float3 vertex_colors[],

   const uniform Matrix4x4 world_to_NDC[],
   const uniform unsigned int tile_triangle_counts[],
   const uniform unsigned int tile_triangle_data_offsets[],
   const uniform BoundingBox2 triangle_screen_boundings[],
   const uniform unsigned int tile_triangle_data[])
{
    const uniform int task_x_count = tile_count_x;
    const uniform int task_y_count = tile_count_y;

    launch [task_x_count, task_y_count]
    task_simple_tile_shading(image_width, 
        image_height,
        tile_width,
        tile_height,
        tile_count_x,
        tile_count_y,

        depth_buffer,
        image,
        
        triangle_count,
        meshes,
        triangles,
        vertex_positions,
        vertex_colors,

        world_to_NDC,
        tile_triangle_counts,
        tile_triangle_data_offsets,
        triangle_screen_boundings,
        tile_triangle_data);

}