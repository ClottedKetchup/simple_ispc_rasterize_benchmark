#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <vector>

#include "simple_math.h"

struct Mesh 
{
	std::vector<Triangle> triangles;
	std::vector<Float3> vertex_positions;
	std::vector<Float3> vertex_colors;

	Mesh() : triangles{}, vertex_positions{}, vertex_colors{}
	{
	}
};


#endif // !MESH_DATA_H
