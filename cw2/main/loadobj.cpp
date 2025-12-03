#include "loadobj.hpp"

#include <rapidobj/rapidobj.hpp>

#include "../support/error.hpp"

SimpleMeshData load_wavefront_obj( char const* aPath )
{
	// Ask reapidobj to load the requested file
	auto res = rapidobj::ParseFile(aPath);
	if ( res.error )
	{
		throw Error("Unable to load OBJ file '{}': {} (line {}: '{}')",
			aPath,
			res.error.code.message(),
			res.error.line_num,
			res.error.line);
	}
	/*
	 * OBJ files can define faces that are not triangles. However, OpenGL will only render triangles (and lines
	 * 	and points), so we must triangulate any faces that are not already triangles.
	 * 	Fortunately, rapidobj can do this for us: re-parse the file with triangulation enabled.
	*/
	rapidobj::Triangulate( res );

	// Convert OBJ data->SimpleMeshData. Effectively it becomes 'triangle soup' as it ignores any indexing.
	SimpleMeshData ret;

	// Pre-allocate for performance
	std::size_t totalVertices = 0;
	for (auto const &shape : res.shapes)
		totalVertices += shape.mesh.indices.size();

	ret.positions.reserve(totalVertices);
	ret.colors.reserve(totalVertices);
	ret.normals.reserve(totalVertices); // IMPORTANT: Reserve for normals

	// Iterate over all shapes and extract their mesh data
	for ( auto const& shape : res.shapes )
	{
		// For each index in the shape's mesh, extract the corresponding position (and other attributes if needed)
		for (std::size_t i = 0; i < shape.mesh.indices.size(); ++i)
		{
			// Define current index of the shape
			auto const& idx = shape.mesh.indices[i];

			std::size_t posIdx = static_cast<std::size_t>( idx.position_index ) * 3;
			ret.positions.emplace_back( Vec3f{
				res.attributes.positions[ posIdx + 0 ],
				res.attributes.positions[ posIdx + 1 ],
				res.attributes.positions[ posIdx + 2 ] } );

			// Normal (CRITICAL FIX)
			if (idx.normal_index >= 0)
			{
				std::size_t normIdx = static_cast<std::size_t>(idx.normal_index) * 3;
				ret.normals.emplace_back(Vec3f{
					res.attributes.normals[normIdx + 0],
					res.attributes.normals[normIdx + 1],
					res.attributes.normals[normIdx + 2]});
			}
			else
			{
				// Generate a default normal if missing
				ret.normals.emplace_back(Vec3f{0.f, 1.f, 0.f});
			}

			// Always triangles, so we can find the face index by dividing the vertex index by three
			auto const &mat = res.materials[shape.mesh.material_ids[i / 3]];
			// Just replicate the material ambient color for each vertex
			ret.colors.emplace_back(Vec3f{
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]
			});
		}
	}

	return ret;
}

