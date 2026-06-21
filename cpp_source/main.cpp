
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
	write_png("D:\\Source_repo\\test_tmp\\ispc_raster\\write_color_out.png", width, height, channels, false, png_out.data());

	ispc::ispc_test_image_operation((ispc::Float3*)(image.data()), width, height, tile_width, tile_height);

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());
	write_png("D:\\Source_repo\\test_tmp\\ispc_raster\\blend_color_out.png", width, height, channels, false, png_out.data());
}

int get_total_bounding_pixel_count(const Mesh& mesh, const int width, const int height, const Matrix4x4& world_to_NDC)
{
	int total_bounding_pixel_count = 0;
	for (const auto& triangle : mesh.triangles) 
	{
		const Float3 world_p0 = mesh.vertex_positions.at(triangle.index_0);
		const Float3 world_p1 = mesh.vertex_positions.at(triangle.index_1);
		const Float3 world_p2 = mesh.vertex_positions.at(triangle.index_2);

		const Float4 p0_NDC = world_to_NDC * make_Float4(world_p0, 1.0f); 
		const Float4 p1_NDC = world_to_NDC * make_Float4(world_p1, 1.0f);
		const Float4 p2_NDC = world_to_NDC * make_Float4(world_p2, 1.0f);

		const Float2 p0_screen = NDC_to_screen(p0_NDC.v[0] / p0_NDC.v[3], p0_NDC.v[1] / p0_NDC.v[3], width, height);
		const float p0_z = p0_NDC.v[2] / p0_NDC.v[3];

		const Float2 p1_screen = NDC_to_screen(p1_NDC.v[0] / p1_NDC.v[3], p1_NDC.v[1] / p1_NDC.v[3], width, height);
		const float p1_z = p1_NDC.v[2] / p1_NDC.v[3];

		const Float2 p2_screen = NDC_to_screen(p2_NDC.v[0] / p2_NDC.v[3], p2_NDC.v[1] / p2_NDC.v[3], width, height);
		const float p2_z = p2_NDC.v[2] / p2_NDC.v[3];

		BoundingBox2 box = make_bounding_box2_varying();
		box = extend_bounding_box2(box, p0_screen);
		box = extend_bounding_box2(box, p1_screen);
		box = extend_bounding_box2(box, p2_screen);

		float x_start = std::max(0.0f, box.x_min);
		float x_end = std::min((float)width, box.x_max);
		float y_start = std::max(0.0f, box.y_min);
		float y_end = std::min((float)height, box.y_max);

		for (int x = (int)floor(x_start); x <= (int)ceil(x_end); ++x) {
			for (int y = (int)floor(y_start); y <= (int)ceil(y_end); ++y) {
				if (x >= 0 && x < width && y >= 0 && y < height) {
					++total_bounding_pixel_count;
				}
			}
		}

	}
	return total_bounding_pixel_count;
}

void simple_triangle_test()
{
	const int width = 640;
	const int height = 480;
	const int tile_width = 4;
	const int tile_height = 4;
	const int channels = 3;

	// init image.
	std::vector<Float3> image(width * height, { 0 });
	std::vector<uint8_t> png_out(width * height * channels, { 0 });

	// init test mesh.
	constexpr float triangle_z = 2.f;
	Mesh test_mesh;
	test_mesh.vertex_positions.push_back(make_Float3(-1.0f, 0.0f, triangle_z));
	test_mesh.vertex_positions.push_back(make_Float3(1.0f, 0.0f, triangle_z + 6.0f));
	test_mesh.vertex_positions.push_back(make_Float3(0.0f, 1.0f, triangle_z + 4.0f));

	test_mesh.triangles.push_back(Triangle{ 0, 1, 2 });

	test_mesh.vertex_colors.push_back(make_Float3(1.0f, 0.0f, 0.0f));
	test_mesh.vertex_colors.push_back(make_Float3(0.0f, 1.0f, 0.0f));
	test_mesh.vertex_colors.push_back(make_Float3(0.0f, 0.0f, 1.0f));

	// init camera
	Camera camera = camera_init_varying();
	camera_look_at(camera, make_Float3(0.0f, 0.0f, triangle_z));
	
	// TODO: clip
	camera.aspect = float(width) / float(height);
	camera.fov = 40.0f;
	camera.near = 1.0f;
	camera.far = 100.0f;

	const Matrix4x4 model = matrix4x4_identity_varying();
	const Matrix4x4 camera_to_world = get_camera_to_world_matrix(camera);
	const Matrix4x4 world_to_camera = matrix4x4_inverse(camera_to_world);
	const Matrix4x4 perspective = get_perspective_matrix(camera.fov, camera.aspect, camera.near, camera.far);
	
	const Matrix4x4 world_to_NDC = perspective * (world_to_camera * model);

	const int total_bounding_pixel_count = get_total_bounding_pixel_count(test_mesh, width, height, world_to_NDC);
	
	std::vector<Pixel> pixel_buffer(total_bounding_pixel_count, {});
	uint64_t raster_pixel_count = 0;

	ispc::ispc_test_render_triangle((ispc::Float3*)(image.data()), 
		width, 
		height, 
		tile_width, 
		tile_height,

		test_mesh.triangles.size(),

		(const ispc::Triangle*)test_mesh.triangles.data(),
		(ispc::Float3*)test_mesh.vertex_positions.data(),
		(ispc::Float3*)test_mesh.vertex_colors.data(),

		(const ispc::Matrix4x4*)(&world_to_NDC),

		(ispc::Pixel*)pixel_buffer.data(),
		&raster_pixel_count);

	image_convert_f32_to_byte((float*)image.data(), width, height, channels, png_out.data());
	write_png("D:\\Source_repo\\test_tmp\\ispc_raster\\test_triangle_image_2.png", width, height, channels, false, png_out.data());
}

int main()
{
	simple_triangle_test();
	return 0;
}