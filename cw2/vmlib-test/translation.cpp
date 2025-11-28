// You will need to define your own tests. Refer to CW1 or Exercise G.3 for
// examples.

#include <catch2/catch_amalgamated.hpp>

#include "../vmlib/mat44.hpp"
#include "../vmlib/vec4.hpp"

TEST_CASE("Translation matrices", "[mat44]")
{
    static constexpr float kEps_ = 1e-6f;

    using namespace Catch::Matchers;

    SECTION("Zero translation")
    {
        Mat44f translation = make_translation({0.0f, 0.0f, 0.0f});

        // Should be identity matrix
        REQUIRE(translation[0, 0] == Approx(1.0f).margin(kEps_));
        REQUIRE(translation[1, 1] == Approx(1.0f).margin(kEps_));
        REQUIRE(translation[2, 2] == Approx(1.0f).margin(kEps_));
        REQUIRE(translation[3, 3] == Approx(1.0f).margin(kEps_));

        REQUIRE(translation[0, 3] == Approx(0.0f).margin(kEps_));
        REQUIRE(translation[1, 3] == Approx(0.0f).margin(kEps_));
        REQUIRE(translation[2, 3] == Approx(0.0f).margin(kEps_));
    }

    SECTION("Positive translation")
    {
        Mat44f translation = make_translation({2.0f, 3.0f, 4.0f});

        REQUIRE(translation[0, 3] == Approx(2.0f).margin(kEps_));
        REQUIRE(translation[1, 3] == Approx(3.0f).margin(kEps_));
        REQUIRE(translation[2, 3] == Approx(4.0f).margin(kEps_));

        // Rest should be identity
        REQUIRE(translation[0, 0] == Approx(1.0f).margin(kEps_));
        REQUIRE(translation[1, 1] == Approx(1.0f).margin(kEps_));
        REQUIRE(translation[2, 2] == Approx(1.0f).margin(kEps_));
        REQUIRE(translation[3, 3] == Approx(1.0f).margin(kEps_));
    }

    SECTION("Negative translation")
    {
        Mat44f translation = make_translation({-1.0f, -2.0f, -3.0f});

        REQUIRE(translation[0, 3] == Approx(-1.0f).margin(kEps_));
        REQUIRE(translation[1, 3] == Approx(-2.0f).margin(kEps_));
        REQUIRE(translation[2, 3] == Approx(-3.0f).margin(kEps_));
    }

    SECTION("Translation of point")
    {
        Mat44f translation = make_translation({5.0f, 10.0f, 15.0f});
        Vec4f point{1.0f, 2.0f, 3.0f, 1.0f};
        Vec4f result = translation * point;

        REQUIRE(result.x == Approx(6.0f).margin(kEps_));
        REQUIRE(result.y == Approx(12.0f).margin(kEps_));
        REQUIRE(result.z == Approx(18.0f).margin(kEps_));
        REQUIRE(result.w == Approx(1.0f).margin(kEps_));
    }

    SECTION("Translation does not affect direction vectors")
    {
        Mat44f translation = make_translation({5.0f, 10.0f, 15.0f});
        Vec4f direction{1.0f, 0.0f, 0.0f, 0.0f}; // w=0 indicates direction
        Vec4f result = translation * direction;

        REQUIRE(result.x == Approx(1.0f).margin(kEps_));
        REQUIRE(result.y == Approx(0.0f).margin(kEps_));
        REQUIRE(result.z == Approx(0.0f).margin(kEps_));
        REQUIRE(result.w == Approx(0.0f).margin(kEps_));
    }

    SECTION("Multiple translations")
    {
        Mat44f trans1 = make_translation({1.0f, 2.0f, 3.0f});
        Mat44f trans2 = make_translation({4.0f, 5.0f, 6.0f});
        Mat44f combined = trans1 * trans2;

        // Combined translation should be (5, 7, 9)
        REQUIRE(combined[0, 3] == Approx(5.0f).margin(kEps_));
        REQUIRE(combined[1, 3] == Approx(7.0f).margin(kEps_));
        REQUIRE(combined[2, 3] == Approx(9.0f).margin(kEps_));
    }
}