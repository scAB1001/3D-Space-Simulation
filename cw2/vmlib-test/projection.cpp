#include <catch2/catch_amalgamated.hpp>

#include <numbers>

#include "../vmlib/mat44.hpp"

TEST_CASE( "Perspective projection", "[mat44]" )
{
	static constexpr float kEps_ = 1e-6f;

	using namespace Catch::Matchers;

	// "Standard" projection matrix presented in the exercises. Assumes
	// standard window size (e.g., 1280x720).
	//
	// Field of view (FOV) = 60 degrees
	// Window size is 1280x720 and we defined the aspect ratio as w/h
	// Near plane at 0.1 and far at 100
	SECTION( "Standard" )
	{
		auto const proj = make_perspective_projection(
			60.f * std::numbers::pi_v<float> / 180.f,
			1280/float(720),
			0.1f, 100.f
		);

		REQUIRE_THAT( (proj[0,0]), WithinAbs( 0.974279, kEps_ ) );
		REQUIRE_THAT( (proj[0,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (proj[0,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (proj[0,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (proj[1,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (proj[1,1]), WithinAbs( 1.732051f, kEps_ ) );
		REQUIRE_THAT( (proj[1,2]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (proj[1,3]), WithinAbs( 0.f, kEps_ ) );

		REQUIRE_THAT( (proj[2,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (proj[2,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (proj[2,2]), WithinAbs( -1.002002f, kEps_ ) );
		REQUIRE_THAT( (proj[2,3]), WithinAbs( -0.200200f, kEps_ ) );

		REQUIRE_THAT( (proj[3,0]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (proj[3,1]), WithinAbs( 0.f, kEps_ ) );
		REQUIRE_THAT( (proj[3,2]), WithinAbs( -1.f, kEps_ ) );
		REQUIRE_THAT( (proj[3,3]), WithinAbs( 0.f, kEps_ ) );
	}

	// SECTION("Square aspect ratio")
    // {
    //     auto const proj = make_perspective_projection(
    //         std::numbers::pi_v<float> / 2.0f, // 90 degrees
    //         1.0f, // Square aspect
    //         0.1f, 100.f
    //     );

    //     // For square aspect, sx should equal sy
    //     REQUIRE_THAT(proj[0, 0], WithinAbs(proj[1, 1], kEps_));
    //     REQUIRE_THAT(proj[0, 0], WithinAbs(1.0f, kEps_)); // tan(45°) = 1
    // }

    // SECTION("Wide aspect ratio")
    // {
    //     auto const proj = make_perspective_projection(
    //         60.f * std::numbers::pi_v<float> / 180.f,
    //         2.0f, // Wide aspect (2:1)
    //         0.1f, 100.f
    //     );

    //     // sx should be half of sy for 2:1 aspect
    //     REQUIRE_THAT(proj[0, 0] * 2.0f, WithinAbs(proj[1, 1], kEps_));
    // }

    // SECTION("Near and far plane effects")
    // {
    //     auto const proj = make_perspective_projection(
    //         std::numbers::pi_v<float> / 3.0f, // 60 degrees
    //         1.0f,
    //         1.0f, 10.f // Different near/far
    //     );

    //     // a = -(far + near) / (far - near)
    //     REQUIRE_THAT(proj[2, 2], WithinAbs(-1.222222f, kEps_));
    //     // b = -2 * far * near / (far - near)
    //     REQUIRE_THAT(proj[2, 3], WithinAbs(-2.222222f, kEps_));
    // }

    // SECTION("Extreme field of view")
    // {
    //     auto const proj = make_perspective_projection(
    //         120.f * std::numbers::pi_v<float> / 180.f, // Very wide FOV
    //         1.0f,
    //         0.1f, 100.f
    //     );

    //     // With wide FOV, the scale factor should be smaller
    //     REQUIRE(proj[0, 0] < 1.0f);
    //     REQUIRE(proj[1, 1] < 1.0f);
    // }
}
