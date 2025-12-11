#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"
#include "../vmlib/vec4.hpp"

TEST_CASE("Rotation matrices", "[mat44]")
{
    static constexpr float kEps_ = 1e-6f;
    static constexpr float kFloatPi = std::numbers::pi_v<float>;

    using namespace Catch::Matchers;

    SECTION("X-axis rotation - 90 degrees")
    {
        // Tests 90deg X rotation: (0,1,0) -> (0,0,1)
        float angle = kFloatPi / 2.0f;
        Mat44f rotX = make_rotation_x(angle);

        // Test rotating a point on Y-axis to Z-axis
        Vec4f point{0.0f, 1.0f, 0.0f, 1.0f};
        Vec4f result = rotX * point;

        // Expects (0,0,1,1)
        REQUIRE_THAT( (result.x), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.y), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.z), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (result.w), (WithinAbs(1.0f, kEps_)) );
    }

    SECTION("X-axis rotation - 180 degrees")
    {
        float angle = kFloatPi;
        Mat44f rotX = make_rotation_x(angle);

        Vec4f point{0.0f, 1.0f, 0.0f, 1.0f};
        Vec4f result = rotX * point;

        REQUIRE_THAT( (result.x), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.y), (WithinAbs(-1.0f, kEps_)) );
        REQUIRE_THAT( (result.z), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.w), (WithinAbs(1.0f, kEps_)) );
    }

    SECTION("Y-axis rotation - 90 degrees")
    {
        float angle = kFloatPi / 2.0f;
        Mat44f rotY = make_rotation_y(angle);

        // Test rotating a point on Z-axis to X-axis
        Vec4f point{0.0f, 0.0f, 1.0f, 1.0f};
        Vec4f result = rotY * point;

        REQUIRE_THAT( (result.x), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (result.y), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.z), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.w), (WithinAbs(1.0f, kEps_)) );
    }

    SECTION("Y-axis rotation - 180 degrees")
    {
        float angle = kFloatPi;
        Mat44f rotY = make_rotation_y(angle);

        Vec4f point{1.0f, 0.0f, 0.0f, 1.0f};
        Vec4f result = rotY * point;

        REQUIRE_THAT( (result.x), (WithinAbs(-1.0f, kEps_)) );
        REQUIRE_THAT( (result.y), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.z), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.w), (WithinAbs(1.0f, kEps_)) );
    }

    SECTION("Z-axis rotation - 90 degrees")
    {
        float angle = kFloatPi / 2.0f;
        Mat44f rotZ = make_rotation_z(angle);

        // Test rotating a point on X-axis to Y-axis
        Vec4f point{1.0f, 0.0f, 0.0f, 1.0f};
        Vec4f result = rotZ * point;

        REQUIRE_THAT( (result.x), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.y), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (result.z), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.w), (WithinAbs(1.0f, kEps_)) );
    }

    SECTION("Z-axis rotation - 180 degrees")
    {
        float angle = kFloatPi;
        Mat44f rotZ = make_rotation_z(angle);

        Vec4f point{1.0f, 1.0f, 0.0f, 1.0f};
        Vec4f result = rotZ * point;

        REQUIRE_THAT( (result.x), (WithinAbs(-1.0f, kEps_)) );
        REQUIRE_THAT( (result.y), (WithinAbs(-1.0f, kEps_)) );
        REQUIRE_THAT( (result.z), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.w), (WithinAbs(1.0f, kEps_)) );
    }

    SECTION("Rotation preserves vector length")
    {
        // Tests rotation preserves vector length (orthogonal matrices)
        float angle = kFloatPi / 3.0f; // 60 degrees
        Mat44f rotX = make_rotation_x(angle);
        Mat44f rotY = make_rotation_y(angle);
        Mat44f rotZ = make_rotation_z(angle);

        Vec4f vector{1.0f, 2.0f, 3.0f, 0.0f}; // Direction vector

        Vec4f resultX = rotX * vector;
        Vec4f resultY = rotY * vector;
        Vec4f resultZ = rotZ * vector;

        // Calculate lengths
        float originalLength = std::sqrt(1.0f + 4.0f + 9.0f);
        float lengthX = std::sqrt(resultX.x * resultX.x + resultX.y * resultX.y + resultX.z * resultX.z);
        float lengthY = std::sqrt(resultY.x * resultY.x + resultY.y * resultY.y + resultY.z * resultY.z);
        float lengthZ = std::sqrt(resultZ.x * resultZ.x + resultZ.y * resultZ.y + resultZ.z * resultZ.z);

        // Expects originalLength approx newLength
        REQUIRE_THAT( (lengthX), (WithinAbs(originalLength, kEps_)) );
        REQUIRE_THAT( (lengthY), (WithinAbs(originalLength, kEps_)) );
        REQUIRE_THAT( (lengthZ), (WithinAbs(originalLength, kEps_)) );
    }
}