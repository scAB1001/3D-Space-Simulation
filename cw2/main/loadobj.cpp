#include "loadobj.hpp"

#include <rapidobj/rapidobj.hpp>

#include "../support/error.hpp"
#include <print>

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
	ret.materialType = 1; // Default to Textured if texcoords are present

	// Check which attributes are present
	bool hasTexcoords = !res.attributes.texcoords.empty();
	bool hasNormals = !res.attributes.normals.empty();

	// Output to console
	std::print("Loaded OBJ file '{}': {} vertices, {} texcoords, {} normals, {} shapes, {} materials\n",
		aPath,
		res.attributes.positions.size() / 3,
		res.attributes.texcoords.size() / 2,
		res.attributes.normals.size() / 3,
		res.shapes.size(),
		res.materials.size()
	);

	// Temporary storage for indexed data
	std::vector<Vec3f> tempPositions;
	std::vector<Vec2f> tempTexcoords;
	std::vector<Vec3f> tempNormals;

	// Extract positions
    for (size_t i = 0; i < res.attributes.positions.size(); i += 3) {
        tempPositions.push_back({
            res.attributes.positions[i],
            res.attributes.positions[i + 1],
            res.attributes.positions[i + 2]
        });
    }

    // Extract texture coordinates
    if (hasTexcoords) {
        for (size_t i = 0; i < res.attributes.texcoords.size(); i += 2) {
            tempTexcoords.push_back({
                res.attributes.texcoords[i],
                res.attributes.texcoords[i + 1]
            });
        }
    }

    // Extract normals
    if (hasNormals) {
        for (size_t i = 0; i < res.attributes.normals.size(); i += 3) {
            tempNormals.push_back({
                res.attributes.normals[i],
                res.attributes.normals[i + 1],
                res.attributes.normals[i + 2]
            });
        }
    }

	/*// Iterate over all shapes and extract their mesh data
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

			// Always triangles, so we can find the face index by dividing the vertex index by three
			auto const &mat = res.materials[shape.mesh.material_ids[i / 3]];
			// Just replicate the material ambient color for each vertex
			ret.colors.emplace_back(Vec3f{
				mat.ambient[0],
				mat.ambient[1],
				mat.ambient[2]
			});
		}
	}*/

	// Process all shapes
    for (auto const& shape : res.shapes) {
        for (std::size_t i = 0; i < shape.mesh.indices.size(); ++i) {
            auto const& idx = shape.mesh.indices[i];

            // Position (always present)
            ret.positions.push_back(tempPositions[idx.position_index]);

            // Texture coordinate (if available)
            if (hasTexcoords && idx.texcoord_index >= 0) {
                ret.texcoords.push_back(tempTexcoords[idx.texcoord_index]);
            } else if (hasTexcoords) {
                // Default UV if index is -1
                ret.texcoords.push_back({0.0f, 0.0f});
            }

            // Normal (if available)
            if (hasNormals && idx.normal_index >= 0) {
                ret.normals.push_back(tempNormals[idx.normal_index]);
            } else if (hasNormals) {
                // Default normal if missing
                ret.normals.push_back({0.0f, 1.0f, 0.0f});
            } else {
                // Generate face normal if no normals in file
                if (i % 3 == 2 && i >= 2) {
                    // Calculate face normal for the last 3 vertices
                    Vec3f v0 = ret.positions[ret.positions.size() - 3];
                    Vec3f v1 = ret.positions[ret.positions.size() - 2];
                    Vec3f v2 = ret.positions[ret.positions.size() - 1];

                    Vec3f edge1 = v1 - v0;
                    Vec3f edge2 = v2 - v0;
                    Vec3f normal = normalize(cross(edge1, edge2));

                    // Apply to all 3 vertices of the triangle
                    ret.normals.push_back(normal);
                    ret.normals.push_back(normal);
                    ret.normals.push_back(normal);
                }
            }

            // Color from material
            if (i / 3 < shape.mesh.material_ids.size()) {
                int matId = shape.mesh.material_ids[i / 3];
                if (matId >= 0 && matId < res.materials.size()) {
                    auto const& mat = res.materials[matId];
                    ret.colors.push_back(Vec3f{
                        mat.ambient[0],
                        mat.ambient[1],
                        mat.ambient[2]
                    });
                } else {
                    ret.colors.push_back({0.5f, 0.5f, 0.5f}); // Default gray
                }
            } else {
                ret.colors.push_back({0.5f, 0.5f, 0.5f}); // Default gray
            }
        }
    }

    // If no texture coordinates were found, switch to colored mode
    if (ret.texcoords.empty()) {
        ret.materialType = 0; // Colored mode
        std::print("No texture coordinates found, using colored mode\n");
    }

    // Ensure all arrays have the same size
    if (ret.normals.empty() && !ret.positions.empty()) {
        // Generate default normals (up)
        ret.normals.resize(ret.positions.size(), {0.0f, 1.0f, 0.0f});
    }

    std::print("Loaded {} vertices\n", ret.positions.size());

	return ret;
}

// SimpleMeshData load_wavefront_obj(char const *aPath)
// {
//     return load_wavefront_obj_with_mtl(aPath, std::vector<Material>());
// }

SimpleMeshData load_wavefront_obj_with_mtl(char const *objPath, std::vector<Material> &materials)
{
    auto res = rapidobj::ParseFile(objPath);
    if (res.error)
    {
        throw Error("Unable to load OBJ file '{}': {}", objPath, res.error.code.message());
    }

    rapidobj::Triangulate(res);

    SimpleMeshData ret;

    // Check what data we have
    bool hasTexcoords = !res.attributes.texcoords.empty();
    bool hasNormals = !res.attributes.normals.empty();

    ret.materialType = hasTexcoords ? 1 : 0;

    std::print( "Loading OBJ: {}\n", objPath );
    std::print( "  Has texcoords: {}\n", hasTexcoords );
    std::print( "  Has normals: {}\n", hasNormals );
    std::print( "  Number of materials: {}\n", res.materials.size() );

    // Convert rapidobj materials to our Material structure
    materials.clear();
    for (const auto &mat : res.materials)
    {
        Material ourMat;
        ourMat.name = mat.name;
        ourMat.ambient = {mat.ambient[0], mat.ambient[1], mat.ambient[2]};
        ourMat.diffuse = {mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]};
        ourMat.specular = {mat.specular[0], mat.specular[1], mat.specular[2]};
        ourMat.shininess = mat.shininess;
        ourMat.alpha = mat.dissolve;
        materials.push_back(ourMat);
    }

    // Temporary indexed data
    std::vector<Vec3f> tempPositions;
    std::vector<Vec2f> tempTexcoords;
    std::vector<Vec3f> tempNormals;

    // Extract positions
    for (size_t i = 0; i < res.attributes.positions.size(); i += 3)
    {
        tempPositions.push_back({res.attributes.positions[i],
                                 res.attributes.positions[i + 1],
                                 res.attributes.positions[i + 2]});
    }

    // Extract texture coordinates
    if (hasTexcoords)
    {
        for (size_t i = 0; i < res.attributes.texcoords.size(); i += 2)
        {
            tempTexcoords.push_back({
                res.attributes.texcoords[i],
                1.0f - res.attributes.texcoords[i + 1] // Flip V coordinate
            });
        }
    }

    // Extract normals
    if (hasNormals)
    {
        for (size_t i = 0; i < res.attributes.normals.size(); i += 3)
        {
            tempNormals.push_back({res.attributes.normals[i],
                                   res.attributes.normals[i + 1],
                                   res.attributes.normals[i + 2]});
        }
    }

    // Process all shapes
    size_t totalVertices = 0;
    for (const auto &shape : res.shapes)
    {
        totalVertices += shape.mesh.indices.size();
    }

    // Reserve space
    ret.positions.reserve(totalVertices);
    if (hasTexcoords)
        ret.texcoords.reserve(totalVertices);
    ret.normals.reserve(totalVertices);
    ret.colors.reserve(totalVertices);

    for (auto const &shape : res.shapes)
    {
        for (std::size_t i = 0; i < shape.mesh.indices.size(); ++i)
        {
            auto const &idx = shape.mesh.indices[i];

            // Position
            ret.positions.push_back(tempPositions[idx.position_index]);

            // Texture coordinate
            if (hasTexcoords && idx.texcoord_index >= 0)
            {
                ret.texcoords.push_back(tempTexcoords[idx.texcoord_index]);
            }
            else if (hasTexcoords)
            {
                ret.texcoords.push_back({0.0f, 0.0f});
            }

            // Normal
            if (hasNormals && idx.normal_index >= 0)
            {
                ret.normals.push_back(tempNormals[idx.normal_index]);
            }
            else if (hasNormals)
            {
                ret.normals.push_back({0.0f, 1.0f, 0.0f});
            }

            // Color from material (use diffuse color)
            int materialIndex = (i / 3 < shape.mesh.material_ids.size())
                                    ? shape.mesh.material_ids[i / 3]
                                    : -1;

            if (materialIndex >= 0 && materialIndex < res.materials.size())
            {
                auto const &mat = res.materials[materialIndex];
                ret.colors.push_back(Vec3f{
                    mat.diffuse[0],
                    mat.diffuse[1],
                    mat.diffuse[2]});
            }
            else
            {
                // Default color
                ret.colors.push_back({0.7f, 0.7f, 0.7f});
            }
        }
    }

    // Generate normals if missing
    if (ret.normals.empty() && !ret.positions.empty())
    {
        ret.normals.resize(ret.positions.size(), {0.0f, 1.0f, 0.0f});
    }

    std::println( "  Loaded {} vertices", ret.positions.size() );

    return ret;
}