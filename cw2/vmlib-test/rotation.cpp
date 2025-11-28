// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.

#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"
#include "../vmlib/vec4.hpp"

TEST_CASE("Rotation matrices", "[mat44]")
{
    static constexpr float kEps_ = 1e-6f;

    using namespace Catch::Matchers;

    SECTION("X-axis rotation - 90 degrees")
    {
        float angle = std::numbers::pi_v<float> / 2.0f;
        Mat44f rotX = make_rotation_x(angle);

        // Test rotating a point on Y-axis to Z-axis
        Vec4f point{0.0f, 1.0f, 0.0f, 1.0f};
        Vec4f result = rotX * point;

        REQUIRE(result.x == Approx(0.0f).margin(kEps_));
        REQUIRE(result.y == Approx(0.0f).margin(kEps_));
        REQUIRE(result.z == Approx(1.0f).margin(kEps_));
        REQUIRE(result.w == Approx(1.0f).margin(kEps_));
    }

    SECTION("X-axis rotation - 180 degrees")
    {
        float angle = std::numbers::pi_v<float>;
        Mat44f rotX = make_rotation_x(angle);

        Vec4f point{0.0f, 1.0f, 0.0f, 1.0f};
        Vec4f result = rotX * point;

        REQUIRE(result.x == Approx(0.0f).margin(kEps_));
        REQUIRE(result.y == Approx(-1.0f).margin(kEps_));
        REQUIRE(result.z == Approx(0.0f).margin(kEps_));
        REQUIRE(result.w == Approx(1.0f).margin(kEps_));
    }

    SECTION("Y-axis rotation - 90 degrees")
    {
        float angle = std::numbers::pi_v<float> / 2.0f;
        Mat44f rotY = make_rotation_y(angle);

        // Test rotating a point on Z-axis to X-axis
        Vec4f point{0.0f, 0.0f, 1.0f, 1.0f};
        Vec4f result = rotY * point;

        REQUIRE(result.x == Approx(1.0f).margin(kEps_));
        REQUIRE(result.y == Approx(0.0f).margin(kEps_));
        REQUIRE(result.z == Approx(0.0f).margin(kEps_));
        REQUIRE(result.w == Approx(1.0f).margin(kEps_));
    }

    SECTION("Y-axis rotation - 180 degrees")
    {
        float angle = std::numbers::pi_v<float>;
        Mat44f rotY = make_rotation_y(angle);

        Vec4f point{1.0f, 0.0f, 0.0f, 1.0f};
        Vec4f result = rotY * point;

        REQUIRE(result.x == Approx(-1.0f).margin(kEps_));
        REQUIRE(result.y == Approx(0.0f).margin(kEps_));
        REQUIRE(result.z == Approx(0.0f).margin(kEps_));
        REQUIRE(result.w == Approx(1.0f).margin(kEps_));
    }

    SECTION("Z-axis rotation - 90 degrees")
    {
        float angle = std::numbers::pi_v<float> / 2.0f;
        Mat44f rotZ = make_rotation_z(angle);

        // Test rotating a point on X-axis to Y-axis
        Vec4f point{1.0f, 0.0f, 0.0f, 1.0f};
        Vec4f result = rotZ * point;

        REQUIRE(result.x == Approx(0.0f).margin(kEps_));
        REQUIRE(result.y == Approx(1.0f).margin(kEps_));
        REQUIRE(result.z == Approx(0.0f).margin(kEps_));
        REQUIRE(result.w == Approx(1.0f).margin(kEps_));
    }

    SECTION("Z-axis rotation - 180 degrees")
    {
        float angle = std::numbers::pi_v<float>;
        Mat44f rotZ = make_rotation_z(angle);

        Vec4f point{1.0f, 1.0f, 0.0f, 1.0f};
        Vec4f result = rotZ * point;

        REQUIRE(result.x == Approx(-1.0f).margin(kEps_));
        REQUIRE(result.y == Approx(-1.0f).margin(kEps_));
        REQUIRE(result.z == Approx(0.0f).margin(kEps_));
        REQUIRE(result.w == Approx(1.0f).margin(kEps_));
    }

    SECTION("Rotation preserves vector length")
    {
        float angle = std::numbers::pi_v<float> / 3.0f; // 60 degrees
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

        REQUIRE(lengthX == Approx(originalLength).margin(kEps_));
        REQUIRE(lengthY == Approx(originalLength).margin(kEps_));
        REQUIRE(lengthZ == Approx(originalLength).margin(kEps_));
    }
}