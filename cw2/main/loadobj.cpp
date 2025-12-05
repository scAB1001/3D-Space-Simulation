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

    // TODO: Remove later. Debug output
    // std::print("Loaded OBJ file '{}': {} vertices, {} texcoords, {} normals, {} shapes, {} materials\n",
	// 	aPath,
	// 	res.attributes.positions.size() / 3,
	// 	res.attributes.texcoords.size() / 2,
	// 	res.attributes.normals.size() / 3,
	// 	res.shapes.size(),
	// 	res.materials.size()
	// );

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

    // TODO: Remove later. Debug output
    // std::print("Loaded {} vertices\n", ret.positions.size());

	return ret;
}

/*
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
*/

SimpleMeshData load_wavefront_obj_with_mtl(char const *objPath, std::vector<Material> &materials)
{
    auto res = rapidobj::ParseFile(objPath);
    if (res.error)
    {
        throw Error("Unable to load OBJ file '{}': {}", objPath, res.error.code.message());
    }

    rapidobj::Triangulate(res);

    SimpleMeshData ret;

    // Check if we have BOTH texture coordinates AND texture references in MTL
    bool hasTexcoords = !res.attributes.texcoords.empty();
    bool hasTextureInMTL = false;

    for (const auto &mat : res.materials)
    {
        if (!mat.diffuse_texname.empty() || !mat.ambient_texname.empty())
        {
            hasTextureInMTL = true;
            break;
        }
    }

    // Set materialType based on what we actually have
    // 0 = colored (using MTL diffuse colors)
    // 1 = textured (using texture file from MTL)
    ret.materialType = (hasTexcoords && hasTextureInMTL) ? 1 : 0;

    // TODO: Remove later. Debug output
    // std::print("Loading OBJ: {}\n", objPath);
    // std::print("  Has texcoords: {}\n", hasTexcoords);
    // std::print("  Has texture in MTL: {}\n", hasTextureInMTL);
    // std::print("  Material type: {} (0=colored, 1=textured)\n", ret.materialType);
    // std::print("  Number of materials: {}\n", res.materials.size());

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

        // TODO: Remove later. Debug output
        // std::print("  Material '{}': Kd=({:.3f},{:.3f},{:.3f})\n",
        //            mat.name, mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
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

    // Extract texture coordinates (ALWAYS extract if they exist)
    if (hasTexcoords)
    {
        for (size_t i = 0; i < res.attributes.texcoords.size(); i += 2)
        {
            // Flip V coordinate for OpenGL
            tempTexcoords.push_back({res.attributes.texcoords[i],
                                     1.0f - res.attributes.texcoords[i + 1]});
        }
    }

    // Extract normals if they exist
    bool hasNormals = !res.attributes.normals.empty();
    if (hasNormals)
    {
        for (size_t i = 0; i < res.attributes.normals.size(); i += 3)
        {
            tempNormals.push_back({res.attributes.normals[i],
                                   res.attributes.normals[i + 1],
                                   res.attributes.normals[i + 2]});
        }
    }

    // Process shapes with correct material tracking
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

    // Track current triangle for material lookup
    size_t currentTriangle = 0;

    for (auto const &shape : res.shapes)
    {
        // TODO: Remove later. Debug output
        // std::print("  Processing shape '{}' with {} indices\n",
        //            shape.name, shape.mesh.indices.size());

        // Track material IDs for this shape
        const auto &material_ids = shape.mesh.material_ids;
        // TODO: Remove later. Debug output
        // std::print("    Shape has {} material assignments\n", material_ids.size());

        for (std::size_t i = 0; i < shape.mesh.indices.size(); ++i)
        {
            auto const &idx = shape.mesh.indices[i];

            // Position (always required)
            ret.positions.push_back(tempPositions[idx.position_index]);

            // Texture coordinate (if available)
            if (hasTexcoords && idx.texcoord_index >= 0)
            {
                ret.texcoords.push_back(tempTexcoords[idx.texcoord_index]);
            }
            else if (hasTexcoords)
            {
                // If OBJ has texcoords but this vertex doesn't, use (0,0)
                ret.texcoords.push_back({0.0f, 0.0f});
            }

            // Normal (if available, otherwise we'll generate later)
            if (hasNormals && idx.normal_index >= 0)
            {
                ret.normals.push_back(tempNormals[idx.normal_index]);
            }
            else if (hasNormals)
            {
                // Default normal if index is -1
                ret.normals.push_back({0.0f, 1.0f, 0.0f});
            }

            // Correct material lookup
            // Determine which triangle we're in (0, 1, 2 = triangle 0; 3, 4, 5 = triangle 1, etc.)
            size_t triangleIndex = currentTriangle / 3;
            int materialIndex = -1;

            if (triangleIndex < material_ids.size())
            {
                materialIndex = material_ids[triangleIndex];
            }

            // Assign color based on material
            if (materialIndex >= 0 && materialIndex < (int)res.materials.size())
            {
                auto const &mat = res.materials[materialIndex];
                // Use DIFFUSE color from MTL (Kd)
                ret.colors.push_back(Vec3f{
                    mat.diffuse[0],
                    mat.diffuse[1],
                    mat.diffuse[2]});

                // TODO: Remove later. Debug output
                if (i < 3) // Debug: show first triangle's material
                {
                    // std::print("    Vertex {}: material index {}, color=({:.3f},{:.3f},{:.3f})\n",
                    //            i, materialIndex, mat.diffuse[0], mat.diffuse[1], mat.diffuse[2]);
                }
            }
            else
            {
                // Default gray color for unmaterialed vertices
                ret.colors.push_back({0.7f, 0.7f, 0.7f});
            }

            currentTriangle++;
        }
    }

    // Generate normals if missing
    if (ret.normals.empty() && !ret.positions.empty())
    {
        // TODO: Remove later. Debug output
        // std::print("  Generating normals (none in file)\n");
        ret.calculate_normals();
    }

    // Validate data consistency
    // TODO: Remove later. Debug output
    // std::print("  Final validation:\n");
    // std::print("    Positions: {}\n", ret.positions.size());
    // std::print("    Colors: {}\n", ret.colors.size());
    // std::print("    Normals: {}\n", ret.normals.size());
    if (hasTexcoords)
        // std::print("    Texcoords: {}\n", ret.texcoords.size());

    // Ensure all arrays have same size
    if (ret.colors.size() != ret.positions.size())
    {
        std::print("  WARNING: Color count mismatch! Fixing...\n");
        ret.colors.resize(ret.positions.size(), {0.7f, 0.7f, 0.7f});
    }

    if (ret.normals.size() != ret.positions.size())
    {
        std::print("  WARNING: Normal count mismatch! Fixing...\n");
        ret.normals.resize(ret.positions.size(), {0.0f, 1.0f, 0.0f});
    }

    // TODO: Remove later. Debug output
    // std::println("  Loaded {} vertices", ret.positions.size());

    return ret;
}