
#include <iostream>
#include <vector>
#include <filesystem>
#include <atomic>
#include <memory>
#include <ppl.h>
#include <cmath>
#include <assert.h>

#include "timer.h"

#include "simple_test.h"
#include "simple_math.h"
#include "mesh_data.h"

#include "image_io.h"

void cpp_hello_world_test(const std::filesystem::path& exe_dir)
{
	const int width = 640;
	const int height = 480;
	const int tile_width = 4;
	const int tile_height = 4;
	const int channels = 3;

	std::vector<Float3> image(width * height, { 0 });
	std::vector<uint8_t> png_out(width * height * channels, { 0 });

	const int tile_count_x = (width + tile_width - 1) / tile_width;
	const int tile_count_y = (height + tile_height - 1) / tile_height;

	Timer timer;
	timer.start();

	concurrency::parallel_for(0, tile_count_y, [&](int tile_y) {
		const int y_start = tile_y * tile_height;
		const int y_end = std::min(y_start + tile_height, height);
		concurrency::parallel_for(0, tile_count_x, [&](int tile_x) {
			const int x_start = tile_x * tile_width;
			const int x_end = std::min(x_start + tile_width, width);
			for (int y = y_start; y < y_end; ++y)
			{
				for (int x = x_start; x < x_end; ++x)
				{
					image[y * width + x] = make_Float3(1.0f, 0.0f, 0.0f);
				}
			}
			});
		});

	double elapsed_ms = timer.stop_ms();
	std::cout << "[cpp_hello_world_test] parallel_for fill: " << elapsed_ms << " ms" << std::endl;

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());

	std::filesystem::path file_name = "cpp_hello_world.png";
	std::filesystem::path full_path = exe_dir / file_name;
	write_png(full_path.generic_string().c_str(), width, height, channels, false, png_out.data());

	std::cout << "[cpp_hello_world_test] total: " << elapsed_ms << " ms" << std::endl;
}

void simple_hello_world_test(const std::filesystem::path& exe_dir)
{
	const int width = 640;
	const int height = 480;
	const int tile_width = 4;
	const int tile_height = 4;
	const int channels = 3;

	std::vector<Float3> image(width * height, { 0 });
	std::vector<uint8_t> png_out(width * height * channels, { 0 });

	Timer timer;
	timer.start();
	ispc::ispc_test_image((ispc::Float3*)(image.data()), width, height, tile_width, tile_height);
	double elapsed_ms = timer.stop_ms();
	std::cout << "[simple_hello_world_test] ispc_test_image: " << elapsed_ms << " ms" << std::endl;

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());

	std::filesystem::path file_name = "ispc_hello_world.png";
	std::filesystem::path full_path = exe_dir / file_name;

	write_png(full_path.generic_string().c_str(), width, height, channels, false, png_out.data());

	std::cout << "[simple_hello_world_test] total: " << elapsed_ms << " ms" << std::endl;
}

void cpp_triangle_test(const std::filesystem::path& exe_dir)
{
	const int width = 1024;
	const int height = 1024;
	const int tile_width = 32;
	const int tile_height = 32;
	const int channels = 3;

	// init test mesh: 512 triangles along a spiral, CCW winding
	const int total_triangles = 512;

	std::vector<Float3> mesh_vertex_positions;
	std::vector<Float3> mesh_vertex_colors;
	std::vector<Triangle> mesh_triangles;

	for (int triangle_index = 0; triangle_index < total_triangles; ++triangle_index)
	{
		float fraction = (float)triangle_index / total_triangles;
		float angle = fraction * 6.0f * 6.283185307f;       // 6 turns
		float radius = 0.15f + fraction * 3.2f;
		float center_x = std::cos(angle) * radius;
		float center_y = std::sin(angle) * radius * 0.75f;
		float depth = 1.5f + fraction * 12.0f;
		float size = 0.04f + fraction * 0.08f;

		uint32_t base_index = (uint32_t)mesh_vertex_positions.size();

		// counter-clockwise: p0(bottom-left) → p1(bottom-right) → p2(top)
		mesh_vertex_positions.push_back(make_Float3(center_x - size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x + size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x, center_y + size * 0.9f, depth));

		Float3 palette[] = {
			make_Float3(1.0f, 0.0f, 0.0f),
			make_Float3(0.0f, 1.0f, 0.0f),
			make_Float3(0.0f, 0.4f, 1.0f),
			make_Float3(1.0f, 0.8f, 0.0f),
			make_Float3(0.8f, 0.0f, 0.8f),
			make_Float3(0.0f, 0.8f, 0.8f),
		};
		Float3 color = palette[triangle_index % 6];

		mesh_vertex_colors.push_back(color);
		mesh_vertex_colors.push_back(color);
		mesh_vertex_colors.push_back(color);

		mesh_triangles.push_back(Triangle{ base_index, base_index + 1, base_index + 2 });
	}

	Model test_model;
	test_model.add_mesh(mesh_triangles, mesh_vertex_positions, mesh_vertex_colors);

	// init camera
	Camera camera = camera_init_varying();
	camera_look_at(camera, make_Float3(0.0f, 0.0f, 4.0f));

	camera.aspect = float(width) / float(height);
	camera.fov = 40.0f;
	camera.near = 1.0f;
	camera.far = 100.0f;

	const Matrix4x4 model = matrix4x4_identity_varying();
	const Matrix4x4 camera_to_world = get_camera_to_world_matrix(camera);
	const Matrix4x4 world_to_camera = matrix4x4_inverse(camera_to_world);
	const Matrix4x4 perspective = get_perspective_matrix(camera.fov, camera.aspect, camera.near, camera.far);

	const Matrix4x4 world_to_NDC = perspective * (world_to_camera * model);

	// ====== Stage 1: get_pixel_count ======
	std::atomic<uint32_t> max_pixel_count{ 0 };

	const int triangle_count = (int)test_model.triangles.size();
	const int batch_triangle_count = 16;
	const int task_count = (triangle_count + batch_triangle_count - 1) / batch_triangle_count;

	Timer timer;
	timer.start();

	concurrency::parallel_for(0, task_count, [&](int task_index) {
		const int start = task_index * batch_triangle_count;
		const int end = std::min(start + batch_triangle_count, triangle_count);

		for (int triangle_index = start; triangle_index < end; ++triangle_index)
		{
			const Triangle& triangle = test_model.triangles[triangle_index];
			const Mesh& mesh = test_model.meshes[triangle.mesh_index];

			const Float3* mesh_vertex_positions_ptr = test_model.vertex_positions.data() + mesh.vertex_positions_offset;

			Float3 p0_world = mesh_vertex_positions_ptr[triangle.index_0];
			Float3 p1_world = mesh_vertex_positions_ptr[triangle.index_1];
			Float3 p2_world = mesh_vertex_positions_ptr[triangle.index_2];

			Float4 p0_homo = world_to_NDC * make_Float4(p0_world, 1.0f);
			Float4 p1_homo = world_to_NDC * make_Float4(p1_world, 1.0f);
			Float4 p2_homo = world_to_NDC * make_Float4(p2_world, 1.0f);

			Float4 src_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
			Float4 dst_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];

			src_vertices[0] = p0_homo;
			src_vertices[1] = p1_homo;
			src_vertices[2] = p2_homo;

			int dst_count = 0;
			clip_triangle_near_plane(src_vertices, dst_vertices, &dst_count);
			if (dst_count < 3)
				continue;

			int clipped_triangle_count = dst_count == 3 ? 1 : 2;
			for (int clipped_triangle_index = 0; clipped_triangle_index < clipped_triangle_count; ++clipped_triangle_index)
			{
				int clipped_v0_index = 0;
				int clipped_v1_index = clipped_triangle_index == 0 ? 1 : 2;
				int clipped_v2_index = clipped_triangle_index == 0 ? 2 : 3;

				Float2 p0_NDC_2D = make_Float2(dst_vertices[clipped_v0_index].v[0] / dst_vertices[clipped_v0_index].v[3],
					dst_vertices[clipped_v0_index].v[1] / dst_vertices[clipped_v0_index].v[3]);
				Float2 p1_NDC_2D = make_Float2(dst_vertices[clipped_v1_index].v[0] / dst_vertices[clipped_v1_index].v[3],
					dst_vertices[clipped_v1_index].v[1] / dst_vertices[clipped_v1_index].v[3]);
				Float2 p2_NDC_2D = make_Float2(dst_vertices[clipped_v2_index].v[0] / dst_vertices[clipped_v2_index].v[3],
					dst_vertices[clipped_v2_index].v[1] / dst_vertices[clipped_v2_index].v[3]);

				float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
				if (std::abs(triangle_area) <= 0.0f)
					continue;

				Float2 p0_screen = NDC_to_screen(p0_NDC_2D.v[0], p0_NDC_2D.v[1], width, height);
				Float2 p1_screen = NDC_to_screen(p1_NDC_2D.v[0], p1_NDC_2D.v[1], width, height);
				Float2 p2_screen = NDC_to_screen(p2_NDC_2D.v[0], p2_NDC_2D.v[1], width, height);

				BoundingBox2 box = make_bounding_box2_varying();
				box = extend_bounding_box2(box, p0_screen);
				box = extend_bounding_box2(box, p1_screen);
				box = extend_bounding_box2(box, p2_screen);

				float x_start = std::max(0.0f, box.x_min);
				float x_end = std::min((float)width, box.x_max);
				float y_start = std::max(0.0f, box.y_min);
				float y_end = std::min((float)height, box.y_max);

				int x_count = (int)std::ceil(x_end) - (int)std::floor(x_start);
				int y_count = (int)std::ceil(y_end) - (int)std::floor(y_start);

				max_pixel_count.fetch_add(x_count * y_count, std::memory_order_relaxed);
			}
		}
		});

	double get_pixel_count_ms = timer.stop_ms();
	std::cout << "[cpp_triangle_test] get_pixel_count: " << get_pixel_count_ms << " ms" << std::endl;

	// allocate buffers
	uint32_t final_max_pixel_count = max_pixel_count.load();
	std::vector<Pixel> pixel_buffer(final_max_pixel_count, { 0.0f, 0.0f, EMPTY_INDEX_32, 0.0f, 0.0f, 0.0f });
	std::atomic<uint32_t> pixel_count{ 0 };

	const uint64_t depth_bits_high = float_to_uint(2.0f);
	const uint64_t depth_bits_low = EMPTY_INDEX_32;
	const uint64_t depth_bits_init = (depth_bits_high << 32) | (depth_bits_low);

	auto depth_buffer = std::make_unique<std::atomic<uint64_t>[]>(width * height);
	for (int i = 0; i < width * height; ++i)
		depth_buffer[i].store(depth_bits_init, std::memory_order_relaxed);

	std::vector<Float3> image(width * height, { 0 });
	std::vector<uint8_t> png_out(width * height * channels, { 0 });

	// ====== Stage 2: rasterize_triangle ======
	timer.start();

	concurrency::parallel_for(0, task_count, [&](int task_index) {
		const int start = task_index * batch_triangle_count;
		const int end = std::min(start + batch_triangle_count, triangle_count);

		for (int triangle_index = start; triangle_index < end; ++triangle_index)
		{
			const Triangle& triangle = test_model.triangles[triangle_index];
			const Mesh& mesh = test_model.meshes[triangle.mesh_index];

			const Float3* mesh_vertex_positions_ptr = test_model.vertex_positions.data() + mesh.vertex_positions_offset;

			Float3 p0_world = mesh_vertex_positions_ptr[triangle.index_0];
			Float3 p1_world = mesh_vertex_positions_ptr[triangle.index_1];
			Float3 p2_world = mesh_vertex_positions_ptr[triangle.index_2];

			Float4 p0_homo = world_to_NDC * make_Float4(p0_world, 1.0f);
			Float4 p1_homo = world_to_NDC * make_Float4(p1_world, 1.0f);
			Float4 p2_homo = world_to_NDC * make_Float4(p2_world, 1.0f);

			Float4 src_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
			Float4 dst_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];

			src_vertices[0] = p0_homo;
			src_vertices[1] = p1_homo;
			src_vertices[2] = p2_homo;

			int dst_count = 0;
			clip_triangle_near_plane(src_vertices, dst_vertices, &dst_count);
			if (dst_count < 3)
				continue;

			int clipped_triangle_count = dst_count == 3 ? 1 : 2;
			for (int clipped_triangle_index = 0; clipped_triangle_index < clipped_triangle_count; ++clipped_triangle_index)
			{
				int clipped_v0_index = 0;
				int clipped_v1_index = clipped_triangle_index == 0 ? 1 : 2;
				int clipped_v2_index = clipped_triangle_index == 0 ? 2 : 3;

				Float2 p0_NDC_2D = make_Float2(dst_vertices[clipped_v0_index].v[0] / dst_vertices[clipped_v0_index].v[3],
					dst_vertices[clipped_v0_index].v[1] / dst_vertices[clipped_v0_index].v[3]);
				Float2 p1_NDC_2D = make_Float2(dst_vertices[clipped_v1_index].v[0] / dst_vertices[clipped_v1_index].v[3],
					dst_vertices[clipped_v1_index].v[1] / dst_vertices[clipped_v1_index].v[3]);
				Float2 p2_NDC_2D = make_Float2(dst_vertices[clipped_v2_index].v[0] / dst_vertices[clipped_v2_index].v[3],
					dst_vertices[clipped_v2_index].v[1] / dst_vertices[clipped_v2_index].v[3]);

				float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
				if (std::abs(triangle_area) <= 0.0f)
					continue;

				Float2 p0_screen = NDC_to_screen(p0_NDC_2D.v[0], p0_NDC_2D.v[1], width, height);
				Float2 p1_screen = NDC_to_screen(p1_NDC_2D.v[0], p1_NDC_2D.v[1], width, height);
				Float2 p2_screen = NDC_to_screen(p2_NDC_2D.v[0], p2_NDC_2D.v[1], width, height);

				BoundingBox2 box = make_bounding_box2_varying();
				box = extend_bounding_box2(box, p0_screen);
				box = extend_bounding_box2(box, p1_screen);
				box = extend_bounding_box2(box, p2_screen);

				int x_start = (int)std::floor(std::max(0.0f, box.x_min));
				int x_end = (int)std::ceil(std::min((float)width, box.x_max));
				int y_start = (int)std::floor(std::max(0.0f, box.y_min));
				int y_end = (int)std::ceil(std::min((float)height, box.y_max));

				for (int x = x_start; x < x_end; ++x)
				{
					for (int y = y_start; y < y_end; ++y)
					{
						if (x < 0 || x >= width || y < 0 || y >= height)
							continue;

						Float2 pixel_center = make_Float2((float)x + 0.5f, (float)y + 0.5f);

						if (!point_inside_triangle_int(pixel_center, p0_screen, p1_screen, p2_screen))
							continue;

						Float2 pixel_center_NDC = screen_to_NDC(pixel_center.v[0], pixel_center.v[1], width, height);

						float cross_p0p_p0p1 = cross(pixel_center_NDC - p0_NDC_2D, p1_NDC_2D - p0_NDC_2D);
						float cross_p1p_p1p2 = cross(pixel_center_NDC - p1_NDC_2D, p2_NDC_2D - p1_NDC_2D);
						float cross_p2p_p0p2 = cross(pixel_center_NDC - p2_NDC_2D, p0_NDC_2D - p2_NDC_2D);

						float b0_NDC = std::abs(cross_p1p_p1p2) / std::abs(triangle_area);
						float b1_NDC = std::abs(cross_p2p_p0p2) / std::abs(triangle_area);
						float b2_NDC = std::abs(cross_p0p_p0p1) / std::abs(triangle_area);

						float weighted_b0_NDC = b0_NDC / dst_vertices[clipped_v0_index].v[3];
						float weighted_b1_NDC = b1_NDC / dst_vertices[clipped_v1_index].v[3];
						float weighted_b2_NDC = b2_NDC / dst_vertices[clipped_v2_index].v[3];
						float weighted_b_sum = weighted_b0_NDC + weighted_b1_NDC + weighted_b2_NDC;

						if (std::abs(weighted_b_sum) <= 0.0f)
							continue;

						float inv_weight_sum = 1.0f / weighted_b_sum;

						float b0_world = weighted_b0_NDC * inv_weight_sum;
						float b1_world = weighted_b1_NDC * inv_weight_sum;
						float b2_world = std::max(1.0f - b0_world - b1_world, 0.0f);

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

						uint32_t pixel_index = pixel_count.fetch_add(1, std::memory_order_relaxed);
						if (pixel_index >= final_max_pixel_count)
							continue;

						uint64_t depth_bits_packed = ((uint64_t)float_to_uint(depth) << 32) | pixel_index;

						int image_pixel_index = y * width + x;

						std::atomic<uint64_t>& atomic_depth_ref = depth_buffer[image_pixel_index];
						uint64_t current = atomic_depth_ref.load(std::memory_order_relaxed);
						while (depth_bits_packed < current)
						{
							if (atomic_depth_ref.compare_exchange_weak(current, depth_bits_packed, std::memory_order_relaxed))
								break;
						}

						Float3 interpolated_p = make_Float3(interpolated_x, interpolated_y, interpolated_z);
						Float3 p0_homo_3 = make_Float3(p0_homo.v[0], p0_homo.v[1], p0_homo.v[2]);
						Float3 p1_homo_3 = make_Float3(p1_homo.v[0], p1_homo.v[1], p1_homo.v[2]);
						Float3 p2_homo_3 = make_Float3(p2_homo.v[0], p2_homo.v[1], p2_homo.v[2]);

						float original_triangle_area = length(cross(p1_homo_3 - p0_homo_3, p2_homo_3 - p1_homo_3));
						float original_p0p_cross_p0p1 = length(cross(interpolated_p - p0_homo_3, p1_homo_3 - p0_homo_3));
						float original_p1p_cross_p1p2 = length(cross(interpolated_p - p1_homo_3, p2_homo_3 - p1_homo_3));
						float original_p2p_cross_p0p2 = length(cross(interpolated_p - p2_homo_3, p0_homo_3 - p2_homo_3));

						float original_b0_world = original_p1p_cross_p1p2 / original_triangle_area;
						float original_b1_world = original_p2p_cross_p0p2 / original_triangle_area;
						float original_b2_world = std::max(1.0f - original_b0_world - original_b1_world, 0.0f);

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
		});

	double rasterize_ms = timer.stop_ms();
	std::cout << "[cpp_triangle_test] rasterize_triangle: " << rasterize_ms << " ms" << std::endl;

	// ====== Stage 3: shading_pixel ======
	const int tile_count_x = (width + tile_width - 1) / tile_width;
	const int tile_count_y = (height + tile_height - 1) / tile_height;

	timer.start();

	concurrency::parallel_for(0, tile_count_y, [&](int tile_y) {
		const int y_start = tile_y * tile_height;
		const int y_end = std::min(y_start + tile_height, height);
		concurrency::parallel_for(0, tile_count_x, [&](int tile_x) {
			const int x_start = tile_x * tile_width;
			const int x_end = std::min(x_start + tile_width, width);
			for (int y = y_start; y < y_end; ++y)
			{
				for (int x = x_start; x < x_end; ++x)
				{
					int image_pixel_index = y * width + x;

					uint64_t depth_bits_loaded = depth_buffer[image_pixel_index].load(std::memory_order_relaxed);
					uint32_t pixel_index = (uint32_t)(depth_bits_loaded & 0x00000000FFFFFFFFULL);
					if (pixel_index == EMPTY_INDEX_32)
					{
						image[image_pixel_index] = make_Float3(0.0f);
						continue;
					}

					const Pixel& pixel = pixel_buffer[pixel_index];
					const Triangle& triangle = test_model.triangles[pixel.triangle_index];
					const Mesh& mesh = test_model.meshes[triangle.mesh_index];

					if (mesh.vertex_colors_offset == EMPTY_INDEX_32)
					{
						image[image_pixel_index] = make_Float3(0.0f);
						continue;
					}

					const Float3* mesh_vertex_colors_ptr = test_model.vertex_colors.data() + mesh.vertex_colors_offset;

					Float3 interpolated_color = pixel.b0_world * mesh_vertex_colors_ptr[triangle.index_0] +
						pixel.b1_world * mesh_vertex_colors_ptr[triangle.index_1] +
						pixel.b2_world * mesh_vertex_colors_ptr[triangle.index_2];

					image[image_pixel_index] = interpolated_color;
				}
			}
			});
		});

	double shading_ms = timer.stop_ms();
	std::cout << "[cpp_triangle_test] shading_pixel: " << shading_ms << " ms" << std::endl;

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());

	std::filesystem::path file_name = "cpp_triangle.png";
	std::filesystem::path full_path = exe_dir / file_name;
	write_png(full_path.generic_string().c_str(), width, height, channels, false, png_out.data());

	double total_ms = get_pixel_count_ms + rasterize_ms + shading_ms;
	std::cout << "[cpp_triangle_test] total: " << total_ms << " ms" << std::endl;
}

void simple_triangle_test(const std::filesystem::path& exe_dir)
{
	const int width = 1024;
	const int height = 1024;
	const int tile_width = 32;
	const int tile_height = 32;
	const int channels = 3;

	// init test mesh: 512 triangles along a spiral, CCW winding
	const int total_triangles = 512;

	std::vector<Float3> mesh_vertex_positions;
	std::vector<Float3> mesh_vertex_colors;
	std::vector<Triangle> mesh_triangles;

	for (int triangle_index = 0; triangle_index < total_triangles; ++triangle_index)
	{
		float fraction = (float)triangle_index / total_triangles;
		float angle = fraction * 6.0f * 6.283185307f;       // 6 turns
		float radius = 0.15f + fraction * 3.2f;
		float center_x = std::cos(angle) * radius;
		float center_y = std::sin(angle) * radius * 0.75f;
		float depth = 1.5f + fraction * 12.0f;
		float size = 0.04f + fraction * 0.08f;

		uint32_t base_index = (uint32_t)mesh_vertex_positions.size();

		// counter-clockwise: p0(bottom-left) → p1(bottom-right) → p2(top)
		mesh_vertex_positions.push_back(make_Float3(center_x - size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x + size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x, center_y + size * 0.9f, depth));

		Float3 palette[] = {
			make_Float3(1.0f, 0.0f, 0.0f),
			make_Float3(0.0f, 1.0f, 0.0f),
			make_Float3(0.0f, 0.4f, 1.0f),
			make_Float3(1.0f, 0.8f, 0.0f),
			make_Float3(0.8f, 0.0f, 0.8f),
			make_Float3(0.0f, 0.8f, 0.8f),
		};
		Float3 color = palette[triangle_index % 6];

		mesh_vertex_colors.push_back(color);
		mesh_vertex_colors.push_back(color);
		mesh_vertex_colors.push_back(color);

		mesh_triangles.push_back(Triangle{ base_index, base_index + 1, base_index + 2 });
	}

	Model test_model;
	test_model.add_mesh(mesh_triangles, mesh_vertex_positions, mesh_vertex_colors);

	// init camera
	Camera camera = camera_init_varying();
	camera_look_at(camera, make_Float3(0.0f, 0.0f, 4.0f));

	camera.aspect = float(width) / float(height);
	camera.fov = 40.0f;
	camera.near = 1.0f;
	camera.far = 100.0f;

	const Matrix4x4 model = matrix4x4_identity_varying();
	const Matrix4x4 camera_to_world = get_camera_to_world_matrix(camera);
	const Matrix4x4 world_to_camera = matrix4x4_inverse(camera_to_world);
	const Matrix4x4 perspective = get_perspective_matrix(camera.fov, camera.aspect, camera.near, camera.far);

	const Matrix4x4 world_to_NDC = perspective * (world_to_camera * model);

	uint32_t max_pixel_count = 0;

	Timer timer;
	timer.start();
	ispc::ispc_get_pixel_count(width,
		height,

		test_model.triangles.size(),
		(const ispc::Mesh*)test_model.meshes.data(),
		(const ispc::Triangle*)test_model.triangles.data(),
		(const ispc::Float3*)test_model.vertex_positions.data(),
		(const ispc::Float3*)test_model.vertex_colors.data(),

		(const ispc::Matrix4x4*)(&world_to_NDC),
		&max_pixel_count);
	double get_pixel_count_ms = timer.stop_ms();
	std::cout << "[simple_triangle_test] get_pixel_count: " << get_pixel_count_ms << " ms" << std::endl;

	std::vector<Pixel> pixel_buffer(max_pixel_count, { 0.0f, 0.0f, EMPTY_INDEX_32, 0.0f, 0.0f, 0.0f });
	uint32_t pixel_count = 0;

	// init image.
	const uint64_t depth_bits_high = float_to_uint(2.0f);
	const uint64_t depth_bits_low = EMPTY_INDEX_32;
	const uint64_t depth_bits = (depth_bits_high << 32) | (depth_bits_low);

	std::vector<Float3> image(width * height, { 0 });
	std::vector<uint64_t> depth_buffer(width * height, { depth_bits });

	std::vector<uint8_t> png_out(width * height * channels, { 0 });

	timer.start();
	ispc::ispc_rasterize_triangle(width,
		height,
		depth_buffer.data(),

		test_model.triangles.size(),
		(const ispc::Mesh*)test_model.meshes.data(),
		(const ispc::Triangle*)test_model.triangles.data(),
		(const ispc::Float3*)test_model.vertex_positions.data(),
		(const ispc::Float3*)test_model.vertex_colors.data(),

		(const ispc::Matrix4x4*)(&world_to_NDC),

		max_pixel_count,
		&pixel_count,
		(ispc::Pixel*)pixel_buffer.data());
	double rasterize_ms = timer.stop_ms();
	std::cout << "[simple_triangle_test] rasterize_triangle: " << rasterize_ms << " ms" << std::endl;

	timer.start();
	ispc::ispc_shading_pixel(width,
		height,
		tile_width,
		tile_height,
		depth_buffer.data(),
		(ispc::Float3*)image.data(),

		(const ispc::Mesh*)test_model.meshes.data(),
		(const ispc::Triangle*)test_model.triangles.data(),
		(const ispc::Float3*)test_model.vertex_positions.data(),
		(const ispc::Float3*)test_model.vertex_colors.data(),

		(const ispc::Matrix4x4*)(&world_to_NDC),

		pixel_count,
		(const ispc::Pixel*)pixel_buffer.data());
	double shading_ms = timer.stop_ms();
	std::cout << "[simple_triangle_test] shading_pixel: " << shading_ms << " ms" << std::endl;

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());

	std::filesystem::path file_name = "ispc_triangle.png";
	std::filesystem::path full_path = exe_dir / file_name;
	write_png(full_path.generic_string().c_str(), width, height, channels, false, png_out.data());

	double total_ms = get_pixel_count_ms + rasterize_ms + shading_ms;
	std::cout << "[simple_triangle_test] total: " << total_ms << " ms" << std::endl;
}

void cpp_tiled_shading_test(const std::filesystem::path& exe_dir)
{
	const int width = 1024;
	const int height = 1024;
	const int tile_width = 32;
	const int tile_height = 32;
	const int channels = 3;

	const int tile_count_x = (width + tile_width - 1) / tile_width;
	const int tile_count_y = (height + tile_height - 1) / tile_height;
	const int total_tile_count = tile_count_x * tile_count_y;

	// init test mesh: 512 triangles along a spiral, CCW winding
	const int total_triangles = 512;

	std::vector<Float3> mesh_vertex_positions;
	std::vector<Float3> mesh_vertex_colors;
	std::vector<Triangle> mesh_triangles;

	for (int triangle_index = 0; triangle_index < total_triangles; ++triangle_index)
	{
		float fraction = (float)triangle_index / total_triangles;
		float angle = fraction * 6.0f * 6.283185307f;       // 6 turns
		float radius = 0.15f + fraction * 3.2f;
		float center_x = std::cos(angle) * radius;
		float center_y = std::sin(angle) * radius * 0.75f;
		float depth = 1.5f + fraction * 12.0f;
		float size = 0.04f + fraction * 0.08f;

		uint32_t base_index = (uint32_t)mesh_vertex_positions.size();

		// counter-clockwise: p0(bottom-left) → p1(bottom-right) → p2(top)
		mesh_vertex_positions.push_back(make_Float3(center_x - size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x + size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x, center_y + size * 0.9f, depth));

		Float3 palette[] = {
			make_Float3(1.0f, 0.0f, 0.0f),
			make_Float3(0.0f, 1.0f, 0.0f),
			make_Float3(0.0f, 0.4f, 1.0f),
			make_Float3(1.0f, 0.8f, 0.0f),
			make_Float3(0.8f, 0.0f, 0.8f),
			make_Float3(0.0f, 0.8f, 0.8f),
		};
		Float3 color = palette[triangle_index % 6];

		mesh_vertex_colors.push_back(color);
		mesh_vertex_colors.push_back(color);
		mesh_vertex_colors.push_back(color);

		mesh_triangles.push_back(Triangle{ base_index, base_index + 1, base_index + 2 });
	}

	Model test_model;
	test_model.add_mesh(mesh_triangles, mesh_vertex_positions, mesh_vertex_colors);

	// init camera
	Camera camera = camera_init_varying();
	camera_look_at(camera, make_Float3(0.0f, 0.0f, 4.0f));

	camera.aspect = float(width) / float(height);
	camera.fov = 40.0f;
	camera.near = 1.0f;
	camera.far = 100.0f;

	const Matrix4x4 model = matrix4x4_identity_varying();
	const Matrix4x4 camera_to_world = get_camera_to_world_matrix(camera);
	const Matrix4x4 world_to_camera = matrix4x4_inverse(camera_to_world);
	const Matrix4x4 perspective = get_perspective_matrix(camera.fov, camera.aspect, camera.near, camera.far);

	const Matrix4x4 world_to_NDC = perspective * (world_to_camera * model);

	Timer timer;

	// step 1: calculate triangle count in each tile
	const int triangle_count = (int)test_model.triangles.size();

	std::vector<unsigned int> tile_triangle_counts(total_tile_count, 0);
	std::vector<TileIndexRange> triangle_tile_ranges(test_model.triangles.size(), { 0, -1, 0, -1 });
	std::vector<BoundingBox2> triangle_screen_boundings(test_model.triangles.size(), make_bounding_box2_varying());

	timer.start();

	const int batch_triangle_count = 16;
	const int task_count = (triangle_count + batch_triangle_count - 1) / batch_triangle_count;

	concurrency::parallel_for(0, task_count, [&](int task_index) {
		const int start = task_index * batch_triangle_count;
		const int end = std::min(start + batch_triangle_count, triangle_count);

		for (int triangle_index = start; triangle_index < end; ++triangle_index)
		{
			const Triangle& triangle = test_model.triangles[triangle_index];
			const Mesh& mesh = test_model.meshes[triangle.mesh_index];

			const Float3* mesh_vertex_positions_ptr = test_model.vertex_positions.data() + mesh.vertex_positions_offset;

			Float3 p0_world = mesh_vertex_positions_ptr[triangle.index_0];
			Float3 p1_world = mesh_vertex_positions_ptr[triangle.index_1];
			Float3 p2_world = mesh_vertex_positions_ptr[triangle.index_2];

			Float4 p0_homo = world_to_NDC * make_Float4(p0_world, 1.0f);
			Float4 p1_homo = world_to_NDC * make_Float4(p1_world, 1.0f);
			Float4 p2_homo = world_to_NDC * make_Float4(p2_world, 1.0f);

			// note: clip against near plane.
			Float4 src_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
			Float4 dst_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];

			src_vertices[0] = p0_homo;
			src_vertices[1] = p1_homo;
			src_vertices[2] = p2_homo;

			int dst_count = 0;
			clip_triangle_near_plane(src_vertices, dst_vertices, &dst_count);
			if (dst_count < 3)
				continue;

			BoundingBox2 triangle_screen_box = make_bounding_box2_varying();
			BoundingBox2 tile_index_box = make_bounding_box2_varying();

			int clipped_triangle_count = dst_count == 3 ? 1 : 2;
			for (int clipped_triangle_index = 0; clipped_triangle_index < clipped_triangle_count; ++clipped_triangle_index)
			{
				int clipped_v0_index = 0;
				int clipped_v1_index = clipped_triangle_index == 0 ? 1 : 2;
				int clipped_v2_index = clipped_triangle_index == 0 ? 2 : 3;

				Float2 p0_NDC_2D = make_Float2(dst_vertices[clipped_v0_index].v[0] / dst_vertices[clipped_v0_index].v[3],
					dst_vertices[clipped_v0_index].v[1] / dst_vertices[clipped_v0_index].v[3]);
				Float2 p1_NDC_2D = make_Float2(dst_vertices[clipped_v1_index].v[0] / dst_vertices[clipped_v1_index].v[3],
					dst_vertices[clipped_v1_index].v[1] / dst_vertices[clipped_v1_index].v[3]);
				Float2 p2_NDC_2D = make_Float2(dst_vertices[clipped_v2_index].v[0] / dst_vertices[clipped_v2_index].v[3],
					dst_vertices[clipped_v2_index].v[1] / dst_vertices[clipped_v2_index].v[3]);

				float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
				if (std::abs(triangle_area) <= 0.0f)
					continue;

				Float2 p0_screen = NDC_to_screen(p0_NDC_2D.v[0], p0_NDC_2D.v[1], width, height);
				Float2 p1_screen = NDC_to_screen(p1_NDC_2D.v[0], p1_NDC_2D.v[1], width, height);
				Float2 p2_screen = NDC_to_screen(p2_NDC_2D.v[0], p2_NDC_2D.v[1], width, height);

				triangle_screen_box = extend_bounding_box2(triangle_screen_box, p0_screen);
				triangle_screen_box = extend_bounding_box2(triangle_screen_box, p1_screen);
				triangle_screen_box = extend_bounding_box2(triangle_screen_box, p2_screen);

				Float2 p0_tile_coordinate = make_Float2(p0_screen.v[0] / (float)tile_width, p0_screen.v[1] / (float)tile_height);
				Float2 p1_tile_coordinate = make_Float2(p1_screen.v[0] / (float)tile_width, p1_screen.v[1] / (float)tile_height);
				Float2 p2_tile_coordinate = make_Float2(p2_screen.v[0] / (float)tile_width, p2_screen.v[1] / (float)tile_height);

				tile_index_box = extend_bounding_box2(tile_index_box, p0_tile_coordinate);
				tile_index_box = extend_bounding_box2(tile_index_box, p1_tile_coordinate);
				tile_index_box = extend_bounding_box2(tile_index_box, p2_tile_coordinate);
			}

			triangle_screen_boundings[triangle_index] = triangle_screen_box;

			int tile_x_start = std::max((int)std::floor(tile_index_box.x_min), 0);
			int tile_x_end = std::min((int)std::floor(tile_index_box.x_max), tile_count_x - 1);
			int tile_y_start = std::max((int)std::floor(tile_index_box.y_min), 0);
			int tile_y_end = std::min((int)std::floor(tile_index_box.y_max), tile_count_y - 1);

			TileIndexRange tile_range;
			tile_range.tile_x_start = tile_x_start;
			tile_range.tile_x_end = tile_x_end;
			tile_range.tile_y_start = tile_y_start;
			tile_range.tile_y_end = tile_y_end;

			triangle_tile_ranges[triangle_index] = tile_range;

			for (int y = tile_y_start; y <= tile_y_end; ++y)
			{
				for (int x = tile_x_start; x <= tile_x_end; ++x)
				{
					int tile_index = y * tile_count_x + x;
					std::atomic_fetch_add((std::atomic_uint32_t*)(&tile_triangle_counts[tile_index]), 1u);
				}
			}
		}
		});

	double get_tile_triangle_count_ms = timer.stop_ms();
	std::cout << "[cpp_tiled_shading_test] get_tile_triangle_count: " << get_tile_triangle_count_ms << " ms" << std::endl;

	// step 2: detect triangle belong to which tile
	timer.start();
	std::vector<uint32_t> tile_triangle_counters(total_tile_count, 0);
	std::vector<unsigned int> tile_triangle_data_offsets(total_tile_count, 0);

	unsigned int total_tile_triangle_count = 0;
	for (size_t tile_index = 0; tile_index < total_tile_count; ++tile_index)
	{
		tile_triangle_data_offsets[tile_index] = total_tile_triangle_count;
		total_tile_triangle_count += tile_triangle_counts[tile_index];
	}

	std::vector<unsigned int> tile_triangle_data(total_tile_triangle_count, EMPTY_INDEX_32);

	concurrency::parallel_for(0u, (uint32_t)test_model.triangles.size(), [&](uint32_t triangle_index)
		{
			const int tile_x_start = triangle_tile_ranges.at(triangle_index).tile_x_start;
			const int tile_x_end = triangle_tile_ranges.at(triangle_index).tile_x_end;
			const int tile_y_start = triangle_tile_ranges.at(triangle_index).tile_y_start;
			const int tile_y_end = triangle_tile_ranges.at(triangle_index).tile_y_end;

			for (int y = tile_y_start; y <= tile_y_end; ++y)
			{
				for (int x = tile_x_start; x <= tile_x_end; ++x)
				{
					int tile_index = y * tile_count_x + x;

					uint32_t data_offset = tile_triangle_data_offsets.at(tile_index);
					uint32_t tile_triangle_index = std::atomic_fetch_add((std::atomic_uint32_t*)(&tile_triangle_counters[tile_index]), 1u);
					assert(tile_triangle_index < tile_triangle_counts.at(tile_index));

					tile_triangle_data.at(data_offset + tile_triangle_index) = triangle_index;
				}
			}
		});
	double build_tile_data_ms = timer.stop_ms();
	std::cout << "[cpp_tiled_shading_test] build_tile_data: " << build_tile_data_ms << " ms" << std::endl;

	// step 3: shade each tile
	timer.start();
	std::vector<Float3> image(width * height, { 0 });
	std::vector<float> depth_buffer(width * height, 2.0f);

	std::vector<uint8_t> png_out(width * height * channels, { 0 });

	concurrency::parallel_for(0, total_tile_count, [&](int tile_index) {
		const int tile_triangle_count = tile_triangle_counts[tile_index];
		const int tile_data_offset = tile_triangle_data_offsets[tile_index];

		const int tile_x = tile_index % tile_count_x;
		const int tile_y = tile_index / tile_count_x;

		const int x_start = tile_x * tile_width;
		const int x_end = std::min(x_start + tile_width, width);
		const int y_start = tile_y * tile_height;
		const int y_end = std::min(y_start + tile_height, height);

		BoundingBox2 tile_screen_box = make_bounding_box2_varying();
		tile_screen_box.x_min = (float)x_start;
		tile_screen_box.x_max = (float)x_end;
		tile_screen_box.y_min = (float)y_start;
		tile_screen_box.y_max = (float)y_end;

		for (int tile_triangle_index = 0; tile_triangle_index < tile_triangle_count; ++tile_triangle_index)
		{
			int triangle_index = tile_triangle_data[tile_data_offset + tile_triangle_index];
			if (!(triangle_index < triangle_count))
				continue;

			const Triangle& triangle = test_model.triangles[triangle_index];
			const Mesh& mesh = test_model.meshes[triangle.mesh_index];

			const Float3* mesh_vertex_positions_ptr = test_model.vertex_positions.data() + mesh.vertex_positions_offset;
			const Float3* mesh_vertex_colors_ptr = test_model.vertex_colors.data() + mesh.vertex_colors_offset;

			Float3 p0_world = mesh_vertex_positions_ptr[triangle.index_0];
			Float3 p1_world = mesh_vertex_positions_ptr[triangle.index_1];
			Float3 p2_world = mesh_vertex_positions_ptr[triangle.index_2];

			Float4 p0_homo = world_to_NDC * make_Float4(p0_world, 1.0f);
			Float4 p1_homo = world_to_NDC * make_Float4(p1_world, 1.0f);
			Float4 p2_homo = world_to_NDC * make_Float4(p2_world, 1.0f);

			Float4 src_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];
			Float4 dst_vertices[MAX_CLIPPED_TRIANGLE_VERTICES];

			src_vertices[0] = p0_homo;
			src_vertices[1] = p1_homo;
			src_vertices[2] = p2_homo;

			int dst_count = 0;
			clip_triangle_near_plane(src_vertices, dst_vertices, &dst_count);
			if (dst_count < 3)
				continue;

			int clipped_triangle_count = dst_count == 3 ? 1 : 2;
			for (int clipped_triangle_index = 0; clipped_triangle_index < clipped_triangle_count; ++clipped_triangle_index)
			{
				int clipped_v0_index = 0;
				int clipped_v1_index = clipped_triangle_index == 0 ? 1 : 2;
				int clipped_v2_index = clipped_triangle_index == 0 ? 2 : 3;

				Float2 p0_NDC_2D = make_Float2(dst_vertices[clipped_v0_index].v[0] / dst_vertices[clipped_v0_index].v[3],
					dst_vertices[clipped_v0_index].v[1] / dst_vertices[clipped_v0_index].v[3]);
				Float2 p1_NDC_2D = make_Float2(dst_vertices[clipped_v1_index].v[0] / dst_vertices[clipped_v1_index].v[3],
					dst_vertices[clipped_v1_index].v[1] / dst_vertices[clipped_v1_index].v[3]);
				Float2 p2_NDC_2D = make_Float2(dst_vertices[clipped_v2_index].v[0] / dst_vertices[clipped_v2_index].v[3],
					dst_vertices[clipped_v2_index].v[1] / dst_vertices[clipped_v2_index].v[3]);

				float triangle_area = cross(p1_NDC_2D - p0_NDC_2D, p2_NDC_2D - p1_NDC_2D);
				if (std::abs(triangle_area) <= 0.0f)
					continue;

				Float2 p0_screen = NDC_to_screen(p0_NDC_2D.v[0], p0_NDC_2D.v[1], width, height);
				Float2 p1_screen = NDC_to_screen(p1_NDC_2D.v[0], p1_NDC_2D.v[1], width, height);
				Float2 p2_screen = NDC_to_screen(p2_NDC_2D.v[0], p2_NDC_2D.v[1], width, height);

				BoundingBox2 clipped_triangle_box = make_bounding_box2_varying();
				clipped_triangle_box = extend_bounding_box2(clipped_triangle_box, p0_screen);
				clipped_triangle_box = extend_bounding_box2(clipped_triangle_box, p1_screen);
				clipped_triangle_box = extend_bounding_box2(clipped_triangle_box, p2_screen);

				BoundingBox2 range_box = bounding_box2_intersect(tile_screen_box, clipped_triangle_box);
				int pixel_range_x_start = (int)std::floor(range_box.x_min);
				int pixel_range_x_end = (int)std::ceil(range_box.x_max);
				int pixel_range_y_start = (int)std::floor(range_box.y_min);
				int pixel_range_y_end = (int)std::ceil(range_box.y_max);

				for (int y = pixel_range_y_start; y < pixel_range_y_end; ++y)
				{
					for (int x = pixel_range_x_start; x < pixel_range_x_end; ++x)
					{
						if (x < 0 || x >= width || y < 0 || y >= height)
							continue;

						Float2 pixel_center = make_Float2((float)x + 0.5f, (float)y + 0.5f);
						Float2 pixel_center_NDC = screen_to_NDC(pixel_center.v[0], pixel_center.v[1], width, height);
						if (!point_inside_triangle_int(pixel_center, p0_screen, p1_screen, p2_screen))
							continue;

						float cross_p0p_p0p1 = cross(pixel_center_NDC - p0_NDC_2D, p1_NDC_2D - p0_NDC_2D);
						float cross_p1p_p1p2 = cross(pixel_center_NDC - p1_NDC_2D, p2_NDC_2D - p1_NDC_2D);
						float cross_p2p_p0p2 = cross(pixel_center_NDC - p2_NDC_2D, p0_NDC_2D - p2_NDC_2D);

						float b0_NDC = std::abs(cross_p1p_p1p2) / std::abs(triangle_area);
						float b1_NDC = std::abs(cross_p2p_p0p2) / std::abs(triangle_area);
						float b2_NDC = std::abs(cross_p0p_p0p1) / std::abs(triangle_area);

						float weighted_b0_NDC = b0_NDC / dst_vertices[clipped_v0_index].v[3];
						float weighted_b1_NDC = b1_NDC / dst_vertices[clipped_v1_index].v[3];
						float weighted_b2_NDC = b2_NDC / dst_vertices[clipped_v2_index].v[3];
						float weighted_b_sum = weighted_b0_NDC + weighted_b1_NDC + weighted_b2_NDC;

						if (!(std::abs(weighted_b_sum) > 0.0f))
							continue;

						int image_pixel_index = y * width + x;

						float inverse_weight_sum = 1.0f / weighted_b_sum;

						float b0_world = weighted_b0_NDC * inverse_weight_sum;
						float b1_world = weighted_b1_NDC * inverse_weight_sum;
						float b2_world = std::max(1.0f - b0_world - b1_world, 0.0f);

						float interpolated_z = dst_vertices[clipped_v0_index].v[2] * b0_world +
							dst_vertices[clipped_v1_index].v[2] * b1_world +
							dst_vertices[clipped_v2_index].v[2] * b2_world;

						float interpolated_w = dst_vertices[clipped_v0_index].v[3] * b0_world +
							dst_vertices[clipped_v1_index].v[3] * b1_world +
							dst_vertices[clipped_v2_index].v[3] * b2_world;

						float depth = interpolated_z / interpolated_w;

						if (depth < depth_buffer[image_pixel_index])
						{
							depth_buffer[image_pixel_index] = depth;

							Float3 interpolated_color = make_Float3(
								b0_world * mesh_vertex_colors_ptr[triangle.index_0].v[0] + b1_world * mesh_vertex_colors_ptr[triangle.index_1].v[0] + b2_world * mesh_vertex_colors_ptr[triangle.index_2].v[0],
								b0_world * mesh_vertex_colors_ptr[triangle.index_0].v[1] + b1_world * mesh_vertex_colors_ptr[triangle.index_1].v[1] + b2_world * mesh_vertex_colors_ptr[triangle.index_2].v[1],
								b0_world * mesh_vertex_colors_ptr[triangle.index_0].v[2] + b1_world * mesh_vertex_colors_ptr[triangle.index_1].v[2] + b2_world * mesh_vertex_colors_ptr[triangle.index_2].v[2]);

							image[image_pixel_index] = interpolated_color;
						}
					}
				}
			}
		}
		});
	double tile_shading_ms = timer.stop_ms();
	std::cout << "[cpp_tiled_shading_test] tile_shading: " << tile_shading_ms << " ms" << std::endl;

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());

	std::filesystem::path file_name = "cpp_tiled_triangle.png";
	std::filesystem::path full_path = exe_dir / file_name;
	write_png(full_path.generic_string().c_str(), width, height, channels, false, png_out.data());

	double total_ms = get_tile_triangle_count_ms + build_tile_data_ms + tile_shading_ms;
	std::cout << "[cpp_tiled_shading_test] total: " << total_ms << " ms" << std::endl;
}

void simple_tiled_shading_test(const std::filesystem::path& exe_dir)
{
	const int width = 1024;
	const int height = 1024;
	const int tile_width = 32;
	const int tile_height = 32;
	const int channels = 3;

	const int tile_count_x = (width + tile_width - 1) / tile_width;
	const int tile_count_y = (height + tile_height - 1) / tile_height;
	const int total_tile_count = tile_count_x * tile_count_y;

	// init test mesh: 512 triangles along a spiral, CCW winding
	const int total_triangles = 512;

	std::vector<Float3> mesh_vertex_positions;
	std::vector<Float3> mesh_vertex_colors;
	std::vector<Triangle> mesh_triangles;

	for (int triangle_index = 0; triangle_index < total_triangles; ++triangle_index)
	{
		float fraction = (float)triangle_index / total_triangles;
		float angle = fraction * 6.0f * 6.283185307f;       // 6 turns
		float radius = 0.15f + fraction * 3.2f;
		float center_x = std::cos(angle) * radius;
		float center_y = std::sin(angle) * radius * 0.75f;
		float depth = 1.5f + fraction * 12.0f;
		float size = 0.04f + fraction * 0.08f;

		uint32_t base_index = (uint32_t)mesh_vertex_positions.size();

		// counter-clockwise: p0(bottom-left) → p1(bottom-right) → p2(top)
		mesh_vertex_positions.push_back(make_Float3(center_x - size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x + size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x, center_y + size * 0.9f, depth));

		Float3 palette[] = {
			make_Float3(1.0f, 0.0f, 0.0f),
			make_Float3(0.0f, 1.0f, 0.0f),
			make_Float3(0.0f, 0.4f, 1.0f),
			make_Float3(1.0f, 0.8f, 0.0f),
			make_Float3(0.8f, 0.0f, 0.8f),
			make_Float3(0.0f, 0.8f, 0.8f),
		};
		Float3 color = palette[triangle_index % 6];

		mesh_vertex_colors.push_back(color);
		mesh_vertex_colors.push_back(color);
		mesh_vertex_colors.push_back(color);

		mesh_triangles.push_back(Triangle{ base_index, base_index + 1, base_index + 2 });
	}

	Model test_model;
	test_model.add_mesh(mesh_triangles, mesh_vertex_positions, mesh_vertex_colors);

	// init camera
	Camera camera = camera_init_varying();
	camera_look_at(camera, make_Float3(0.0f, 0.0f, 4.0f));

	camera.aspect = float(width) / float(height);
	camera.fov = 40.0f;
	camera.near = 1.0f;
	camera.far = 100.0f;

	const Matrix4x4 model = matrix4x4_identity_varying();
	const Matrix4x4 camera_to_world = get_camera_to_world_matrix(camera);
	const Matrix4x4 world_to_camera = matrix4x4_inverse(camera_to_world);
	const Matrix4x4 perspective = get_perspective_matrix(camera.fov, camera.aspect, camera.near, camera.far);

	const Matrix4x4 world_to_NDC = perspective * (world_to_camera * model);

	Timer timer;

	// step 1: calculate triangle count in each tile.
	std::vector<unsigned int> tile_triangle_counts(total_tile_count, 0);
	std::vector<TileIndexRange> triangle_tile_ranges(test_model.triangles.size(), { 0, -1, 0, -1 });
	std::vector<BoundingBox2> triangle_screen_boundings(test_model.triangles.size(), make_bounding_box2_varying());

	timer.start();
	ispc::ispc_get_tile_triangle_count(width,
		height,
		tile_width,
		tile_height,
		tile_count_x,
		tile_count_y,

		test_model.triangles.size(),
		(const ispc::Mesh*)test_model.meshes.data(),
		(const ispc::Triangle*)test_model.triangles.data(),
		(const ispc::Float3*)test_model.vertex_positions.data(),
		(const ispc::Float3*)test_model.vertex_colors.data(),

		(const ispc::Matrix4x4*)(&world_to_NDC),
		tile_triangle_counts.data(),
		(ispc::TileIndexRange*)triangle_tile_ranges.data(),
		(ispc::BoundingBox2*)triangle_screen_boundings.data());
	double get_tile_triangle_count_ms = timer.stop_ms();
	std::cout << "[simple_tiled_shading_test] get_tile_triangle_count: " << get_tile_triangle_count_ms << " ms" << std::endl;

	// step 2: detect triangle belong to which tile.
	timer.start();
	std::vector<uint32_t> tile_triangle_counters(total_tile_count, 0);
	std::vector<unsigned int> tile_triangle_data_offsets(total_tile_count, 0);

	unsigned int total_tile_triangle_count = 0;
	for (size_t tile_index = 0; tile_index < total_tile_count; ++tile_index)
	{
		tile_triangle_data_offsets[tile_index] = total_tile_triangle_count;
		total_tile_triangle_count += tile_triangle_counts[tile_index];
	}

	std::vector<unsigned int> tile_triangle_data(total_tile_triangle_count, EMPTY_INDEX_32);

	concurrency::parallel_for(0u, (uint32_t)test_model.triangles.size(), [&triangle_tile_ranges, &tile_triangle_data_offsets, &tile_triangle_counters, &tile_count_x, &tile_count_y, &tile_triangle_data, &tile_triangle_counts](uint32_t triangle_index)
		{
			const int tile_x_start = triangle_tile_ranges.at(triangle_index).tile_x_start;
			const int tile_x_end = triangle_tile_ranges.at(triangle_index).tile_x_end;
			const int tile_y_start = triangle_tile_ranges.at(triangle_index).tile_y_start;
			const int tile_y_end = triangle_tile_ranges.at(triangle_index).tile_y_end;

			for (int y = tile_y_start; y <= tile_y_end; ++y)
			{
				for (int x = tile_x_start; x <= tile_x_end; ++x)
				{
					int tile_index = y * tile_count_x + x;

					uint32_t data_offset = tile_triangle_data_offsets.at(tile_index);
					uint32_t tile_tri_index = std::atomic_fetch_add((std::atomic_uint32_t*)(&tile_triangle_counters[tile_index]), 1u);
					assert(tile_tri_index < tile_triangle_counts.at(tile_index));

					tile_triangle_data.at(data_offset + tile_tri_index) = triangle_index;
				}
			}
		});
	double build_tile_data_ms = timer.stop_ms();
	std::cout << "[simple_tiled_shading_test] build_tile_data: " << build_tile_data_ms << " ms" << std::endl;

	// step 3: shade each tile.
	timer.start();
	std::vector<Float3> image(width * height, { 0 });
	std::vector<float> depth_buffer(width * height, 2.0f);

	std::vector<uint8_t> png_out(width * height * channels, { 0 });

	ispc::ispc_simple_tile_shading(width,
		height,
		tile_width,
		tile_height,
		tile_count_x,
		tile_count_y,

		depth_buffer.data(),
		(ispc::Float3*)image.data(),

		test_model.triangles.size(),
		(const ispc::Mesh*)test_model.meshes.data(),
		(const ispc::Triangle*)test_model.triangles.data(),
		(const ispc::Float3*)test_model.vertex_positions.data(),
		(const ispc::Float3*)test_model.vertex_colors.data(),

		(const ispc::Matrix4x4*)(&world_to_NDC),
		tile_triangle_counts.data(),
		tile_triangle_data_offsets.data(),
		(ispc::BoundingBox2*)triangle_screen_boundings.data(),
		tile_triangle_data.data());
	double tile_shading_ms = timer.stop_ms();
	std::cout << "[simple_tiled_shading_test] tile_shading: " << tile_shading_ms << " ms" << std::endl;

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());

	std::filesystem::path file_name = "ispc_tiled_triangle.png";
	std::filesystem::path full_path = exe_dir / file_name;
	write_png(full_path.generic_string().c_str(), width, height, channels, false, png_out.data());

	double total_ms = get_tile_triangle_count_ms + build_tile_data_ms + tile_shading_ms;
	std::cout << "[simple_tiled_shading_test] total: " << total_ms << " ms" << std::endl;
}

int main(int argc, char* argv[])
{
	std::filesystem::path exe_path = std::filesystem::absolute(argv[0]);
	std::filesystem::path current_dir = exe_path.parent_path();

	cpp_hello_world_test(current_dir);
	simple_hello_world_test(current_dir);
	cpp_triangle_test(current_dir);
	simple_triangle_test(current_dir);
	cpp_tiled_shading_test(current_dir);
	simple_tiled_shading_test(current_dir);

	return 0;
}

