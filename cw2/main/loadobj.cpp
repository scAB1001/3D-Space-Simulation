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
	rapidobj::Triangulate( res );

	// Convert OBJ data->SimpleMeshData.
	SimpleMeshData ret;
	ret.materialType = 1; // Default to Textured if texcoords are present

	// Check which attributes are present
	bool hasTexcoords = !res.attributes.texcoords.empty();
	bool hasNormals = !res.attributes.normals.empty();

	// Temporary storage for indexed data
	std::vector<Vec3f> tempPositions;
	std::vector<Vec2f> tempTexcoords;
	std::vector<Vec3f> tempNormals;

	// Extract positions
    for (size_t i = 0; i < res.attributes.positions.size(); i += 3)
    {
        tempPositions.push_back({
            res.attributes.positions[i],
            res.attributes.positions[i + 1],
            res.attributes.positions[i + 2]
        });
    }

    // Extract texture coordinates
    if (hasTexcoords)
    {
        for (size_t i = 0; i < res.attributes.texcoords.size(); i += 2)
        {
            tempTexcoords.push_back({
                res.attributes.texcoords[i],
                res.attributes.texcoords[i + 1]
            });
        }
    }

    // Extract normals
    if (hasNormals)
    {
        for (size_t i = 0; i < res.attributes.normals.size(); i += 3)
        {
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
            if (hasTexcoords && idx.texcoord_index >= 0)
            {
                ret.texcoords.push_back(tempTexcoords[idx.texcoord_index]);
            }
            else if (hasTexcoords)
            {
                // Default UV if index is -1
                ret.texcoords.push_back({0.0f, 0.0f});
            }

            // Normal (if available)
            if (hasNormals && idx.normal_index >= 0)
            {
                ret.normals.push_back(tempNormals[idx.normal_index]);
            }
            else if (hasNormals)
            {
                // Default normal if missing
                ret.normals.push_back({0.0f, 1.0f, 0.0f});
            }
            else
            {
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
            if (i / 3 < shape.mesh.material_ids.size())
            {
                int matId = shape.mesh.material_ids[i / 3];
                if (matId >= 0)
                {
                    size_t matIndex = static_cast<size_t>(matId);
                    if (matIndex < res.materials.size())
                    {
                        auto const &mat = res.materials[matIndex];
                        ret.colors.push_back(Vec3f{
                            mat.ambient[0],
                            mat.ambient[1],
                            mat.ambient[2]});
                    }
                    else
                    {
                        ret.colors.push_back({0.5f, 0.5f, 0.5f});
                    }
                }
                else
                {
                    // matId is negative (no material)
                    ret.colors.push_back({0.5f, 0.5f, 0.5f});
                }
            }
            else
            {
                ret.colors.push_back({0.5f, 0.5f, 0.5f});
            }
        }
    }

    // If no texture coordinates were found, switch to colored mode
    if (ret.texcoords.empty()) {
        ret.materialType = 0; // Colored mode
    }

    // Ensure all arrays have the same size
    if (ret.normals.empty() && !ret.positions.empty()) {
        // Generate default normals (up)
        ret.normals.resize(ret.positions.size(), {0.0f, 1.0f, 0.0f});
    }

	return ret;
}

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
        // Track material IDs for this shape
        const auto &material_ids = shape.mesh.material_ids;
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
        ret.calculate_normals();
    }

    // Validate data consistency
    if (ret.colors.size() != ret.positions.size())
    {
        ret.colors.resize(ret.positions.size(), {0.7f, 0.7f, 0.7f});
    }

    if (ret.normals.size() != ret.positions.size())
    {
        ret.normals.resize(ret.positions.size(), {0.0f, 1.0f, 0.0f});
    }

    return ret;
}