#include <catch2/catch_amalgamated.hpp>

#include <numbers>

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
        REQUIRE_THAT( (translation[0, 0]), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (translation[1, 1]), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (translation[2, 2]), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (translation[3, 3]), (WithinAbs(1.0f, kEps_)) );

        REQUIRE_THAT( (translation[0, 3]), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (translation[1, 3]), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (translation[2, 3]), (WithinAbs(0.0f, kEps_)) );
    }

    SECTION("Positive translation")
    {
        // Tests translation matrix has identity upper 3x3, translation in last column
        Mat44f translation = make_translation({2.0f, 3.0f, 4.0f});

        // Verifies [0,3]=2, [1,3]=3, [2,3]=4
        REQUIRE_THAT( (translation[0, 3]), (WithinAbs(2.0f, kEps_)) );
        REQUIRE_THAT( (translation[1, 3]), (WithinAbs(3.0f, kEps_)) );
        REQUIRE_THAT( (translation[2, 3]), (WithinAbs(4.0f, kEps_)) );

        // Rest should be identity, diagonal=1
        REQUIRE_THAT( (translation[0, 0]), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (translation[1, 1]), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (translation[2, 2]), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (translation[3, 3]), (WithinAbs(1.0f, kEps_)) );
    }

    SECTION("Negative translation")
    {
        Mat44f translation = make_translation({-1.0f, -2.0f, -3.0f});

        REQUIRE_THAT( (translation[0, 3]), (WithinAbs(-1.0f, kEps_)) );
        REQUIRE_THAT( (translation[1, 3]), (WithinAbs(-2.0f, kEps_)) );
        REQUIRE_THAT( (translation[2, 3]), (WithinAbs(-3.0f, kEps_)) );
    }

    SECTION("Translation of point")
    {
        Mat44f translation = make_translation({5.0f, 10.0f, 15.0f});
        Vec4f point{1.0f, 2.0f, 3.0f, 1.0f};
        Vec4f result = translation * point;

        REQUIRE_THAT( (result.x), (WithinAbs(6.0f, kEps_)) );
        REQUIRE_THAT( (result.y), (WithinAbs(12.0f, kEps_)) );
        REQUIRE_THAT( (result.z), (WithinAbs(18.0f, kEps_)) );
        REQUIRE_THAT( (result.w), (WithinAbs(1.0f, kEps_)) );
    }

    SECTION("Translation does not affect direction vectors")
    {
        Mat44f translation = make_translation({5.0f, 10.0f, 15.0f});
        Vec4f direction{1.0f, 0.0f, 0.0f, 0.0f}; // w=0 indicates direction
        Vec4f result = translation * direction;

        REQUIRE_THAT( (result.x), (WithinAbs(1.0f, kEps_)) );
        REQUIRE_THAT( (result.y), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.z), (WithinAbs(0.0f, kEps_)) );
        REQUIRE_THAT( (result.w), (WithinAbs(0.0f, kEps_)) );
    }

    SECTION("Multiple translations")
    {
        // Tests translation composition: T_1 x T_2 = T_12
        Mat44f trans1 = make_translation({1.0f, 2.0f, 3.0f});
        Mat44f trans2 = make_translation({4.0f, 5.0f, 6.0f});
        Mat44f combined = trans1 * trans2;

        // Combined translation should be (5, 7, 9) in last column
        REQUIRE_THAT( (combined[0, 3]), (WithinAbs(5.0f, kEps_)) );
        REQUIRE_THAT( (combined[1, 3]), (WithinAbs(7.0f, kEps_)) );
        REQUIRE_THAT( (combined[2, 3]), (WithinAbs(9.0f, kEps_)) );
    }
}