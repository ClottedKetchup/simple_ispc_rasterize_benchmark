#ifndef MESH_DATA_H
#define MESH_DATA_H

#include <vector>

#include "simple_math.h"

struct Model 
{
	std::vector<Mesh> meshes;

	std::vector<Triangle> triangles;
	std::vector<Float3> vertex_positions;
	std::vector<Float3> vertex_colors;

	Model() : meshes{}, triangles{}, vertex_positions{}, vertex_colors{}
	{
	}

	inline void add_mesh(const std::vector<Triangle>& mesh_triangles, 
		const std::vector<Float3>& mesh_vertex_positions,
		const std::vector<Float3>& mesh_vertex_colors)
	{
		if (mesh_triangles.empty() || mesh_vertex_positions.empty())
		{
			return;
		}

		Mesh mesh;
		mesh.triangles_offset = EMPTY_INDEX_32;
		mesh.vertex_positions_offset = EMPTY_INDEX_32;
		mesh.vertex_colors_offset = EMPTY_INDEX_32;

		mesh.triangles_offset = triangles.size();
		triangles.resize(triangles.size() + mesh_triangles.size());
		for (size_t index = 0; index < mesh_triangles.size(); ++index) 
		{
			const Triangle& src_tri = mesh_triangles.at(index);
			Triangle& dst_tri = triangles.at(mesh.triangles_offset + index);
			 
			dst_tri.index_0 = src_tri.index_0;
			dst_tri.index_1 = src_tri.index_1;
			dst_tri.index_2 = src_tri.index_2;
			
			dst_tri.mesh_index = meshes.size();
		}

		mesh.vertex_positions_offset = vertex_positions.size();
		vertex_positions.insert(vertex_positions.end(), mesh_vertex_positions.begin(), mesh_vertex_positions.end());

		if (!mesh_vertex_colors.empty()) 
		{
			mesh.vertex_colors_offset = vertex_colors.size();
			vertex_colors.insert(vertex_colors.end(), mesh_vertex_colors.begin(), mesh_vertex_colors.end());
		}

		meshes.push_back(mesh);
	}
};


#endif // !MESH_DATA_H
