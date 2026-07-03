
#include <iostream>
#include <vector>
#include <filesystem>
#include <atomic>
#include <memory>
#include <ppl.h>
#include <cmath>

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
}

void cpp_triangle_test(const std::filesystem::path& exe_dir)
{
	const int width = 640;
	const int height = 480;
	const int tile_width = 4;
	const int tile_height = 4;
	const int channels = 3;

	// init test mesh: 64 triangles in concentric rings, CCW winding
	const int total_triangles = 64;

	std::vector<Float3> mesh_vertex_positions;
	std::vector<Float3> mesh_vertex_colors;
	std::vector<Triangle> mesh_triangles;

	for (int triangle_index = 0; triangle_index < total_triangles; ++triangle_index)
	{
		float angle = (float)triangle_index / total_triangles * 6.283185307f;
		float ring = (float)(triangle_index % 8) / 8.0f;
		float radius = 0.3f + ring * 2.0f;
		float center_x = std::cos(angle) * radius;
		float center_y = std::sin(angle) * radius * 0.75f;
		float depth = 1.5f + (float)triangle_index / total_triangles * 5.0f;

		float size = 0.10f + ring * 0.18f;

		uint32_t base_index = (uint32_t)mesh_vertex_positions.size();

		// counter-clockwise: p0(bottom-left) → p1(bottom-right) → p2(top)
		mesh_vertex_positions.push_back(make_Float3(center_x - size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x + size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x, center_y + size * 0.9f, depth));

		Float3 base_color = make_Float3(
			(std::sin(angle * 1.3f) + 1.0f) * 0.5f,
			(std::cos(angle * 1.7f + 1.2f) + 1.0f) * 0.5f,
			(std::sin(angle + 2.094f) + 1.0f) * 0.5f);

		mesh_vertex_colors.push_back(base_color);
		mesh_vertex_colors.push_back(make_Float3(base_color.v[0] * 0.6f, base_color.v[1] * 0.6f, base_color.v[2] * 0.6f));
		mesh_vertex_colors.push_back(make_Float3(base_color.v[0] * 1.4f, base_color.v[1] * 1.4f, base_color.v[2] * 1.4f));

		mesh_triangles.push_back(Triangle{base_index, base_index + 1, base_index + 2});
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
	std::atomic<uint32_t> max_pixel_count{0};

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
	std::vector<Pixel> pixel_buffer(final_max_pixel_count, {0.0f, 0.0f, EMPTY_INDEX_32, 0.0f, 0.0f, 0.0f});
	std::atomic<uint32_t> pixel_count{0};

	const uint64_t depth_bits_high = float_to_uint(2.0f);
	const uint64_t depth_bits_low = EMPTY_INDEX_32;
	const uint64_t depth_bits_init = (depth_bits_high << 32) | (depth_bits_low);

	auto depth_buffer = std::make_unique<std::atomic<uint64_t>[]>(width * height);
	for (int i = 0; i < width * height; ++i)
		depth_buffer[i].store(depth_bits_init, std::memory_order_relaxed);

	std::vector<Float3> image(width * height, {0});
	std::vector<uint8_t> png_out(width * height * channels, {0});

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
}

void simple_triangle_test(const std::filesystem::path& exe_dir)
{
	const int width = 640;
	const int height = 480;
	const int tile_width = 4;
	const int tile_height = 4;
	const int channels = 3;

	// init test mesh: 64 triangles in concentric rings, CCW winding
	const int total_triangles = 64;

	std::vector<Float3> mesh_vertex_positions;
	std::vector<Float3> mesh_vertex_colors;
	std::vector<Triangle> mesh_triangles;

	for (int triangle_index = 0; triangle_index < total_triangles; ++triangle_index)
	{
		float angle = (float)triangle_index / total_triangles * 6.283185307f;
		float ring = (float)(triangle_index % 8) / 8.0f;
		float radius = 0.3f + ring * 2.0f;
		float center_x = std::cos(angle) * radius;
		float center_y = std::sin(angle) * radius * 0.75f;
		float depth = 1.5f + (float)triangle_index / total_triangles * 5.0f;

		float size = 0.10f + ring * 0.18f;

		uint32_t base_index = (uint32_t)mesh_vertex_positions.size();

		// counter-clockwise: p0(bottom-left) → p1(bottom-right) → p2(top)
		mesh_vertex_positions.push_back(make_Float3(center_x - size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x + size, center_y - size * 0.6f, depth));
		mesh_vertex_positions.push_back(make_Float3(center_x, center_y + size * 0.9f, depth));

		Float3 base_color = make_Float3(
			(std::sin(angle * 1.3f) + 1.0f) * 0.5f,
			(std::cos(angle * 1.7f + 1.2f) + 1.0f) * 0.5f,
			(std::sin(angle + 2.094f) + 1.0f) * 0.5f);

		mesh_vertex_colors.push_back(base_color);
		mesh_vertex_colors.push_back(make_Float3(base_color.v[0] * 0.6f, base_color.v[1] * 0.6f, base_color.v[2] * 0.6f));
		mesh_vertex_colors.push_back(make_Float3(base_color.v[0] * 1.4f, base_color.v[1] * 1.4f, base_color.v[2] * 1.4f));

		mesh_triangles.push_back(Triangle{base_index, base_index + 1, base_index + 2});
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

	std::vector<Pixel> pixel_buffer(max_pixel_count, {0.0f, 0.0f, EMPTY_INDEX_32, 0.0f, 0.0f, 0.0f});
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
}

int main(int argc, char* argv[])
{
	std::filesystem::path exe_path = std::filesystem::absolute(argv[0]);
	std::filesystem::path current_dir = exe_path.parent_path();

	cpp_hello_world_test(current_dir);
	simple_hello_world_test(current_dir);
	cpp_triangle_test(current_dir);
	simple_triangle_test(current_dir);
	return 0;
}