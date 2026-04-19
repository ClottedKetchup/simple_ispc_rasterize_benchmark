
#include <iostream>
#include <vector>
#include "simple_test.h"
#include "simple_math.h"

int main()
{
	std::vector<float> v_in{ 1, 2, 3, 4, 5, 6, 7, 8, 9 };
	std::vector<float> v_out(v_in.size());
	ispc::simple(v_in.data(), v_out.data(), v_in.size());

	for (auto& digit : v_out)
		printf("%.4f ", digit);
	printf("\n");

	const int width = 640;
	const int height = 480;
	const int tile_width = 4;
	const int tile_height = 4;
	std::vector<float3> image(width * height, { 0 });
	ispc::ispc_test_image((ispc::float3*)(image.data()), width, height, tile_width, tile_height);

	int debug_point = 0;

	ispc::ispc_test_image_operation((ispc::float3*)(image.data()), width, height, tile_width, tile_height);

	int debug_point_2 = 0;

	return 0;
}