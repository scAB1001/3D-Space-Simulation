// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.
#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"
#include "../vmlib/vec4.hpp"

TEST_CASE("Matrix multiplication", "[mat44]")
{
    static constexpr float kEps_ = 1e-6f;

    using namespace Catch::Matchers;

    SECTION("Identity multiplication")
    {
        Mat44f identity = kIdentity44f;
        Mat44f result = identity * identity;

        for (std::size_t i = 0; i < 4; ++i)
        {
            for (std::size_t j = 0; j < 4; ++j)
            {
                if (i == j)
                {
                    REQUIRE_THAT(result[i, j], WithinAbs(1.0f, kEps_));
                }
                else
                {
                    REQUIRE_THAT(result[i, j], WithinAbs(0.0f, kEps_));
                }
            }
        }
    }

    SECTION("Simple 2x scaling multiplication")
    {
        Mat44f scale1 = make_scaling(2.0f, 2.0f, 2.0f);
        Mat44f scale2 = make_scaling(3.0f, 3.0f, 3.0f);
        Mat44f result = scale1 * scale2;

        REQUIRE_THAT(result[0, 0], WithinAbs(6.0f, kEps_));
        REQUIRE_THAT(result[1, 1], WithinAbs(6.0f, kEps_));
        REQUIRE_THAT(result[2, 2], WithinAbs(6.0f, kEps_));
        REQUIRE_THAT(result[3, 3], WithinAbs(1.0f, kEps_));
    }

    SECTION("Translation then rotation")
    {
        Mat44f translation = make_translation({1.0f, 2.0f, 3.0f});
        Mat44f rotation = make_rotation_x(std::numbers::pi_v<float> / 2.0f);
        Mat44f result = translation * rotation;

        // The translation components should remain in the last column
        REQUIRE_THAT(result[0, 3], WithinAbs(1.0f, kEps_));
        REQUIRE_THAT(result[1, 3], WithinAbs(2.0f, kEps_));
        REQUIRE_THAT(result[2, 3], WithinAbs(3.0f, kEps_));
    }

    SECTION("Matrix-vector multiplication - identity")
    {
        Mat44f identity = kIdentity44f;
        Vec4f vector{1.0f, 2.0f, 3.0f, 1.0f};
        Vec4f result = identity * vector;

        REQUIRE_THAT(result.x, WithinAbs(1.0f, kEps_));
        REQUIRE_THAT(result.y, WithinAbs(2.0f, kEps_));
        REQUIRE_THAT(result.z, WithinAbs(3.0f, kEps_));
        REQUIRE_THAT(result.w, WithinAbs(1.0f, kEps_));
    }

    SECTION("Matrix-vector multiplication - translation")
    {
        Mat44f translation = make_translation({2.0f, 3.0f, 4.0f});
        Vec4f point{1.0f, 1.0f, 1.0f, 1.0f}; // Point
        Vec4f result = translation * point;

        REQUIRE_THAT(result.x, WithinAbs(3.0f, kEps_));
        REQUIRE_THAT(result.y, WithinAbs(4.0f, kEps_));
        REQUIRE_THAT(result.z, WithinAbs(5.0f, kEps_));
        REQUIRE_THAT(result.w, WithinAbs(1.0f, kEps_));
    }

    SECTION("Matrix-vector multiplication - scaling")
    {
        Mat44f scaling = make_scaling(2.0f, 3.0f, 4.0f);
        Vec4f vector{1.0f, 2.0f, 3.0f, 1.0f};
        Vec4f result = scaling * vector;

        REQUIRE_THAT(result.x, WithinAbs(2.0f, kEps_));
        REQUIRE_THAT(result.y, WithinAbs(6.0f, kEps_));
        REQUIRE_THAT(result.z, WithinAbs(12.0f, kEps_));
        REQUIRE_THAT(result.w, WithinAbs(1.0f, kEps_));
    }

    SECTION("Matrix-vector multiplication - direction vector (w=0)")
    {
        Mat44f translation = make_translation({2.0f, 3.0f, 4.0f});
        Vec4f direction{1.0f, 0.0f, 0.0f, 0.0f}; // Direction (should not be translated)
        Vec4f result = translation * direction;

        REQUIRE_THAT(result.x, WithinAbs(1.0f, kEps_));
        REQUIRE_THAT(result.y, WithinAbs(0.0f, kEps_));
        REQUIRE_THAT(result.z, WithinAbs(0.0f, kEps_));
        REQUIRE_THAT(result.w, WithinAbs(0.0f, kEps_));
    }
}