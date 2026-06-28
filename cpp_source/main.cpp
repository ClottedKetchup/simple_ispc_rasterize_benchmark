
#include <iostream>
#include <vector>

#include "simple_test.h"
#include "simple_math.h"
#include "mesh_data.h"

#include "image_io.h"

void simple_hello_world_test() 
{
	const int width = 640;
	const int height = 480;
	const int tile_width = 4;
	const int tile_height = 4;
	const int channels = 3;

	std::vector<Float3> image(width * height, { 0 });
	std::vector<uint8_t> png_out(width * height * channels, { 0 });

	ispc::ispc_test_image((ispc::Float3*)(image.data()), width, height, tile_width, tile_height);

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());
	write_png("D:\\Source_repo\\test_tmp\\ispc_raster\\ispc_hello_world.png", width, height, channels, false, png_out.data());
}

void simple_triangle_test()
{
	const int width = 640;
	const int height = 480;
	const int tile_width = 4;
	const int tile_height = 4;
	const int channels = 3;

	// init test mesh.
	constexpr float triangle_z = 2.f;

	std::vector<Float3> mesh_vertex_positions;
	mesh_vertex_positions.push_back(make_Float3(-1.0f, 0.0f, triangle_z));
	mesh_vertex_positions.push_back(make_Float3(1.0f, 0.0f, triangle_z));
	mesh_vertex_positions.push_back(make_Float3(0.0f, 1.0f, triangle_z));

	std::vector<Float3> mesh_vertex_colors;
	mesh_vertex_colors.push_back(make_Float3(1.0f, 0.0f, 0.0f));
	mesh_vertex_colors.push_back(make_Float3(1.0f, 1.0f, 0.0f));
	mesh_vertex_colors.push_back(make_Float3(1.0f, 0.0f, 0.0f));

	std::vector<Triangle> mesh_triangles;
	mesh_triangles.push_back(Triangle{ 0, 1, 2 });

	mesh_vertex_positions.push_back(make_Float3(0.5f, 0.0f, 2.5f));
	mesh_vertex_positions.push_back(make_Float3(2.5f, 0.0f, 2.5f));
	mesh_vertex_positions.push_back(make_Float3(0.5f, 2.0f, 2.5f));

	mesh_vertex_colors.push_back(make_Float3(0.0f, 0.0f, 1.0f));
	mesh_vertex_colors.push_back(make_Float3(0.0f, 1.0f, 0.0f));
	mesh_vertex_colors.push_back(make_Float3(0.0f, 0.0f, 1.0f));

	mesh_triangles.push_back(Triangle{ 3, 4, 5 });

	Model test_model;
	test_model.add_mesh(mesh_triangles, mesh_vertex_positions, mesh_vertex_colors);

	// init camera
	Camera camera = camera_init_varying();
	camera_look_at(camera, make_Float3(0.0f, 0.0f, triangle_z));
	
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
	ispc::ispc_get_pixel_count(width, 
		height, 

		test_model.triangles.size(),
		(const ispc::Mesh*)test_model.meshes.data(), 
		(const ispc::Triangle*)test_model.triangles.data(), 
		(const ispc::Float3*)test_model.vertex_positions.data(),
		(const ispc::Float3*)test_model.vertex_colors.data(),

		(const ispc::Matrix4x4*)(&world_to_NDC),
		&max_pixel_count);
	
	std::vector<Pixel> pixel_buffer(max_pixel_count, {0.0f, 0.0f, EMPTY_INDEX_32, 0.0f, 0.0f, 0.0f});
	uint32_t pixel_count = 0;

	// init image.
	const uint64_t depth_bits_high = float_to_uint(2.0f);
	const uint64_t depth_bits_low = EMPTY_INDEX_32;
	const uint64_t depth_bits = (depth_bits_high << 32) | (depth_bits_low);
	
	std::vector<Float3> image(width * height, { 0 });
	std::vector<uint64_t> depth_buffer(width * height, { depth_bits });

	std::vector<uint8_t> png_out(width * height * channels, { 0 });

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

		
	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());
	write_png("D:\\Source_repo\\test_tmp\\ispc_raster\\test_depth_test.png", width, height, channels, false, png_out.data());
}

int main()
{
	simple_triangle_test();
	return 0;
}