#ifndef MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
#define MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA

#include <cmath>
#include <cassert>
#include <cstdlib>
#include "../main/config.hpp"

#include "vec3.hpp"
#include "vec4.hpp"

/** Mat44f: 4x4 matrix with floats
 *
 * See vec2f.hpp for discussion. Similar to the implementation, the Mat44f is
 * intentionally kept simple and somewhat bare bones.
 *
 * The matrix is stored in row-major order (careful when passing it to OpenGL).
 *
 * The overloaded operator [] allows access to individual elements. Example:
 *    Mat44f m = ...;
 *    float m12 = m[1,2];
 *    m[0,3] = 3.f;
 *
 * (Multi-dimensionsal subscripts in operator[] is a C++23 feature!)
 *
 * The matrix is arranged as:
 *
 *   ⎛ 0,0  0,1  0,2  0,3 ⎞
 *   ⎜ 1,0  1,1  1,2  1,3 ⎟
 *   ⎜ 2,0  2,1  2,2  2,3 ⎟
 *   ⎝ 3,0  3,1  3,2  3,3 ⎠
 */
struct Mat44f
{
	float v[16];

	constexpr
	float& operator[] (std::size_t aI, std::size_t aJ) noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
	constexpr
	float const& operator[] (std::size_t aI, std::size_t aJ) const noexcept
	{
		assert( aI < 4 && aJ < 4 );
		return v[aI*4 + aJ];
	}
};

// Identity matrix
constexpr Mat44f kIdentity44f = { {
	1.f, 0.f, 0.f, 0.f,
	0.f, 1.f, 0.f, 0.f,
	0.f, 0.f, 1.f, 0.f,
	0.f, 0.f, 0.f, 1.f
} };

// Moved above operator* method to be used there
inline
Mat44f transpose( Mat44f const& aM ) noexcept
{
	Mat44f ret;
	for( std::size_t i = 0; i < 4; ++i )
	{
		for( std::size_t j = 0; j < 4; ++j )
			ret[j,i] = aM[i,j];
	}
	return ret;
}
// Common operators for Mat44f.
// Note that you will need to implement these yourself.

constexpr
Mat44f operator*(Mat44f const& aLeft, Mat44f const& aRight) noexcept
{
	// Q[i,j] = sum(k=0 to 3) L[i,k] * R[k,j]
	Mat44f result{};

	for (std::size_t i = 0; i < 4; ++i)
	{
		for (std::size_t j = 0; j < 4; ++j)
		{
			// Manual dot product Left row i and Right column j
			std::size_t rowBase = i * 4;
			std::size_t colBase = j;

			result[i, j] = aLeft.v[rowBase + 0] * aRight[0, colBase] +
						   aLeft.v[rowBase + 1] * aRight[1, colBase] +
						   aLeft.v[rowBase + 2] * aRight[2, colBase] +
						   aLeft.v[rowBase + 3] * aRight[3, colBase];
		}
	}

	return result;
}

constexpr
Vec4f operator*( Mat44f const& aLeft, Vec4f const& aRight ) noexcept
{
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aLeft;   // Avoid warnings about unused arguments until the function
	// (void)aRight;  // is properly implemented.
	// return { 0.f, 0.f, 0.f, 0.f };

	// result[i] = sum(j=0 to 3) M[i,j] * v[j]
	return Vec4f{
		// x' = m_row0 dot vector
		aLeft[0, 0] * aRight.x + aLeft[0, 1] * aRight.y + aLeft[0, 2] * aRight.z + aLeft[0, 3] * aRight.w,
		// y' = m_row1 dot vector
		aLeft[1, 0] * aRight.x + aLeft[1, 1] * aRight.y + aLeft[1, 2] * aRight.z + aLeft[1, 3] * aRight.w,
		// z' = m_row2 dot vector
		aLeft[2, 0] * aRight.x + aLeft[2, 1] * aRight.y + aLeft[2, 2] * aRight.z + aLeft[2, 3] * aRight.w,
		// w' = m_row3 dot vector
		aLeft[3, 0] * aRight.x + aLeft[3, 1] * aRight.y + aLeft[3, 2] * aRight.z + aLeft[3, 3] * aRight.w
	};
}

// Functions:
Mat44f invert( Mat44f const& aM ) noexcept;

inline
Mat44f make_rotation_x( float aAngle ) noexcept
{
	/* Rotation matrix around X-axis using precomputed sin,cos
	Pr_x =
		[ 1    0    0   0 ]
		[ 0   ca  -sa   0 ]
		[ 0   sa   ca   0 ]
		[ 0    0    0   1 ]
	*/

	// Pre-computations: calculate sin and cos once
	const float cosA = std::cos(aAngle);
	const float sinA = std::sin(aAngle);

	return Mat44f{{
		1.f,	0.f,	0.f,	0.f,
		0.f,	cosA,	-sinA,	0.f,
		0.f,	sinA,	cosA,	0.f,
		0.f,	0.f,	0.f, 	1.f
	}};
}


inline
Mat44f make_rotation_y( float aAngle ) noexcept
{
	/* Rotation matrix around Y-axis using precomputed sin,cos

	Pr_y =
		[  ca   0   sa   0 ]
		[   0   1    0   0 ]
		[ -sa   0   ca   0 ]
		[   0   0    0   1 ]
	*/

	// Pre-computations: calculate sin and cos once
	const float cosA = std::cos(aAngle);
	const float sinA = std::sin(aAngle);

	return Mat44f{{
		cosA,	0.f,	sinA,	0.f,
		0.f,	1.f,	0.f, 	0.f,
		-sinA,	0.f,	cosA,	0.f,
		0.f,	0.f,	0.f, 	1.f
	}};
}

inline
Mat44f make_rotation_z( float aAngle ) noexcept
{
	/* Rotation matrix around Z-axis using precomputed sin,cos

	Pr_z =
		[ ca  -sa   0    0 ]
		[ sa   ca   0    0 ]
		[  0    0   1    0 ]
		[  0    0   0    1 ]
	*/

	// Pre-computations: calculate sin and cos once
	const float cosA = std::cos(aAngle);
	const float sinA = std::sin(aAngle);

	return Mat44f{{
		cosA,   -sinA,   0.f,    0.f,
		sinA,    cosA,   0.f,    0.f,
		0.f,     0.f,    1.f,    0.f,
		0.f,     0.f,    0.f,    1.f
	}};
}

inline
Mat44f make_translation( Vec3f aTranslation ) noexcept
{
	/* Translation matrix
	Pt =
		[ 1  0  0  tx ]
		[ 0  1  0  ty ]
		[ 0  0  1  tz ]
		[ 0  0  0  1  ]
	*/

	return Mat44f{{
		1.f, 0.f, 0.f, aTranslation.x,
		0.f, 1.f, 0.f, aTranslation.y,
		0.f, 0.f, 1.f, aTranslation.z,
		0.f, 0.f, 0.f, 1.f
	}};
}
inline
Mat44f make_scaling( float aSX, float aSY, float aSZ ) noexcept
{
	/*  Uniform scaling matrix
	Ps =
		[ sx   0    0   0 ]
		[ 0   sy    0   0 ]
		[ 0    0   sz   0 ]
		[ 0    0    0   1 ]
	*/
	return Mat44f{{
		aSX, 0.f, 0.f, 0.f,
		0.f, aSY, 0.f, 0.f,
		0.f, 0.f, aSZ, 0.f,
		0.f, 0.f, 0.f, 1.f
	}};
}

inline
Mat44f make_look_at(Vec3f position, Vec3f target, Vec3f worldUp) noexcept
{
	// TODO: CITE LookAt implementation
	// Reference: https://learnopengl.com/Getting-started/Camera
	// Reference: https://www.scratchapixel.com/lessons/mathematics-physics-for-computer-graphics/lookat-function/framing-lookat-function.html

	// Forward vector: points from camera to what it's looking at
	Vec3f f = normalize(target - position);

	// Right vector: positive X in camera space
	Vec3f s = normalize(cross(f, normalize(worldUp)));

	// Real worldUp vector: positive Y in camera space
	Vec3f u = cross(s, f);

	// View matrix in row-major format
	// First 3 columns: camera axes in world space
	// Last column: translation to move camera to origin
	return Mat44f{{
		s.x, 	s.y, 	s.z, 	-dot(s, position),
		u.x, 	u.y, 	u.z, 	-dot(u, position),
		-f.x, 	-f.y,	 -f.z, 	dot(f, position), // Negative because OpenGL looks down -Z
		0.f,	0.f, 	0.f, 	1.f
	}};
}

inline
Mat44f make_perspective_projection( float aFovInRadians, float aAspect, float aNear, float aFar ) noexcept
{
	/* TODO: CITE lecture content for matrix but not implementation
		aspect: aspect ratio (width/height)
		fov: field of view (angle in degrees, e.g. 60 degrees)
		f: far distance (e.g. 100)
		n: near distance (e.g., 0.1)

	Pm =
		[ sx   0    0    0 ]
		[ 0    s    0    0 ]
		[ 0    0    a    b ]
		[ 0    0   -1    0 ]
	*/

	// Pre-computations
	// Do any of these need to be constexpr?
	const float s = 1.f / std::tan(aFovInRadians / 2.f);
	const float invAspect = 1.f / aAspect;
	const float sx = s * invAspect;

	const float negInvDepthRange = -1.f / (aFar - aNear);
	const float a = (aFar + aNear) * negInvDepthRange;
	const float b = 2.f * aFar * aNear * negInvDepthRange;

	return Mat44f{{
		sx,		0.f,	0.f,	0.f,
		0.f,	s,		0.f,	0.f,
		0.f,	0.f,	a,		b,
		0.f,	0.f,	-1.f,	0.f
	}};
}

inline
Mat44f make_proj_camera_world(const Mat44f &projectView, const Mat44f &model2world) noexcept
{
	return projectView * model2world;
}

// Smooth acceleration function using cubic ease-in
inline float smooth_accelerate(float t) noexcept
{
	// Cubic ease-in: starts slow, accelerates
	return t * t * t;
}

// Improved smooth acceleration with longer initial phase
inline float smooth_accelerate_slow(float t) noexcept
{
	// Quintic ease-in: much slower initial acceleration
	// t^5 gives a very slow start, perfect for rocket launch
	return t * t * t * t * t;
}

// Bezier curve for smooth path
inline Vec3f calculate_bezier_position(float t,
									   const Vec3f &p0,
									   const Vec3f &p1,
									   const Vec3f &p2,
									   const Vec3f &p3) noexcept
{
	// Cubic Bezier curve: B(t) = (1-t)³P₀ + 3(1-t)²tP₁ + 3(1-t)t²P₂ + t³P₃
	float u = 1.0f - t;
	float uu = u * u;
	float uuu = uu * u;
	float tt = t * t;
	float ttt = tt * t;

	return uuu * p0 +
		   3.0f * uu * t * p1 +
		   3.0f * u * tt * p2 +
		   ttt * p3;
}

// Smooth acceleration function for rocket launch
inline float rocket_acceleration_curve(float t) noexcept
{
	// Quintic ease-in for very slow start
	if (t < 0.5f)
	{
		return t * t * t * t * t * 32.0f; // Very slow initial acceleration
	}
	else
	{
		// Smoother transition to full speed
		return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
	}
}

// Smooth deceleration for landing
inline float rocket_deceleration_curve(float t) noexcept
{
	// Cubic ease-out for smooth landing
	return 1.0f - (1.0f - t) * (1.0f - t) * (1.0f - t);
}

// Continuous rocket trajectory with proper phase transitions
inline Vec3f calculate_rocket_position(float t, float totalTime,
									   const Vec3f &start, const Vec3f &end,
									   float &outSpeed,
									   float acceleration,
									   float maxSpeed) noexcept
{
	// Normalized time (0 to 1)
	float normalizedT = t / totalTime;

	// Phase boundaries (40% launch, 30% cruise, 30% landing)
	const float launchEnd = 0.4f;
	const float cruiseEnd = 0.7f;

	// Calculate continuous trajectory using a single parametric equation
	Vec3f position;
	float launchHeight = 15.0f; // Maximum height during launch
	float cruiseHeight = 35.0f; // Peak height during cruise
	float hoverHeight = 10.0f;	// Hover height above landing pad

	// PHASE 1: LAUNCH (0% - 40%)
	if (normalizedT < launchEnd)
	{
		float phaseT = normalizedT / launchEnd;

		// Very slow start with quintic easing
		float easedT = rocket_acceleration_curve(phaseT * 0.5f) * 2.0f;

		// Purely vertical launch with slight horizontal drift
		float verticalProgress = easedT;
		float horizontalProgress = easedT * easedT * 0.3f; // Slower horizontal movement

		position.x = start.x + (end.x - start.x) * horizontalProgress;
		position.y = start.y + launchHeight * verticalProgress;
		position.z = start.z + (end.z - start.z) * horizontalProgress;

		// Very slow acceleration during launch
		outSpeed = std::min(outSpeed + acceleration * 0.003f, maxSpeed * 0.4f);
	}
	// PHASE 2: CRUISE (40% - 70%)
	else if (normalizedT < cruiseEnd)
	{
		float phaseT = (normalizedT - launchEnd) / (cruiseEnd - launchEnd);

		// Cubic Bezier curve for smooth arc
		Vec3f p0 = start + Vec3f{0.f, launchHeight, 0.f}; // End of launch phase
		Vec3f p1 = p0 + normalize(end - start) * 20.0f + Vec3f{0.f, 20.f, 0.f};
		Vec3f p2 = end + Vec3f{0.f, cruiseHeight, 0.f} - normalize(end - start) * 20.0f;
		Vec3f p3 = end + Vec3f{0.f, hoverHeight, 0.f}; // Start of landing phase

		// Ease-in-out for cruise phase
		float easedT = phaseT < 0.5f ? 2.0f * phaseT * phaseT : 1.0f - pow(-2.0f * phaseT + 2.0f, 2.0f) / 2.0f;

		position = calculate_bezier_position(easedT, p0, p1, p2, p3);

		// Maintain speed during cruise, slight deceleration toward end
		float cruiseSpeed = maxSpeed;
		if (phaseT > 0.6f)
		{
			// Start decelerating in late cruise
			float decel = (phaseT - 0.6f) / 0.4f;
			cruiseSpeed = maxSpeed * (1.0f - decel * 0.3f);
		}
		outSpeed = cruiseSpeed;
	}
	// PHASE 3: LANDING (70% - 100%)
	else
	{
		float phaseT = (normalizedT - cruiseEnd) / (1.0f - cruiseEnd);

		// Split landing into two sub-phases
		if (phaseT < 0.6f) // 60%: Hover and orient vertically
		{
			float hoverT = phaseT / 0.6f;
			float easedT = rocket_deceleration_curve(hoverT);

			// Hover above target while descending slowly
			float height = hoverHeight * (1.0f - easedT * 0.7f);
			position = end + Vec3f{0.f, height, 0.f};

			// Significant deceleration
			outSpeed = maxSpeed * 0.5f * (1.0f - hoverT);
		}
		else // 40%: Final vertical descent
		{
			float descentT = (phaseT - 0.6f) / 0.4f;
			float easedT = rocket_deceleration_curve(descentT);

			// Final vertical descent to pad
			float startHeight = hoverHeight * 0.3f; // From hover height
			float height = startHeight * (1.0f - easedT);
			position = end + Vec3f{0.f, height, 0.f};

			// Very slow final descent
			outSpeed = maxSpeed * 0.1f * (1.0f - descentT);
		}
	}

	return position;
}

// Improved rocket rotation that matches flight phases
inline Mat44f calculate_rocket_rotation(const Vec3f &currentPos,
										const Vec3f &previousPos,
										const Vec3f &velocity,
										float normalizedT) noexcept
{
	// Use normalized time directly for smooth transitions
	const float launchEnd = 0.4f;
	const float cruiseEnd = 0.7f;

	// Base orientation (vertical)
	Mat44f rotation = kIdentity44f;

	if (length(velocity) > 0.001f)
	{
		Vec3f forwardDir = normalize(velocity);

		// PHASE 1: LAUNCH - gradually tilt forward
		if (normalizedT < launchEnd)
		{
			float phaseT = normalizedT / launchEnd;

			// Start vertical, gradually tilt toward destination
			Vec3f toTarget = normalize(Vec3f{
				Config::kLandingPad2Pos.x - Config::kLandingPad1Pos.x,
				0.f,
				Config::kLandingPad2Pos.z - Config::kLandingPad1Pos.z});

			// Calculate tilt angle (0 to 30 degrees)
			float maxTilt = Config::kFloatPi / 6.0f; // 30 degrees
			float tiltAngle = phaseT * maxTilt;

			// Blend between vertical and tilted orientation
			Vec3f vertical = {0.f, 1.f, 0.f};
			// Vec3f tilted = normalize(vertical + toTarget * tan(tiltAngle));

			// Calculate rotation to achieve this direction
			float yaw = atan2(toTarget.x, toTarget.z);
			rotation = make_rotation_y(yaw) * make_rotation_x(-tiltAngle);
		}
		// PHASE 2: CRUISE - follow velocity direction
		else if (normalizedT < cruiseEnd)
		{
			float phaseT = (normalizedT - launchEnd) / (cruiseEnd - launchEnd);

			// Follow velocity direction during cruise
			float yaw = atan2(forwardDir.x, forwardDir.z);
			float pitch = -asin(forwardDir.y);

			// Gradually reduce tilt as we approach landing phase
			float maxPitch = Config::kFloatPi / 4.0f; // 45 degrees max
			if (pitch > maxPitch)
				pitch = maxPitch;
			if (pitch < -maxPitch)
				pitch = -maxPitch;

			// Reduce pitch toward end of cruise (preparing for landing)
			if (phaseT > 0.6f)
			{
				float reduce = (phaseT - 0.6f) / 0.4f;
				pitch *= (1.0f - reduce * 0.8f);
			}

			rotation = make_rotation_y(yaw) * make_rotation_x(pitch);
		}
		// PHASE 3: LANDING - return to vertical
		else
		{
			float phaseT = (normalizedT - cruiseEnd) / (1.0f - cruiseEnd);

			// First part: orient vertically while hovering
			if (phaseT < 0.6f)
			{
				float orientT = phaseT / 0.6f;

				// Blend from cruise orientation to vertical
				float yaw = atan2(forwardDir.x, forwardDir.z);
				float currentPitch = -asin(forwardDir.y);
				float targetPitch = 0.0f; // Vertical

				float pitch = currentPitch * (1.0f - orientT) + targetPitch * orientT;
				rotation = make_rotation_y(yaw) * make_rotation_x(pitch);
			}
			// Second part: maintain vertical during descent
			else
			{
				// Pure vertical orientation
				rotation = kIdentity44f; // Already vertical
			}
		}
	}

	return rotation;
}

// Helper function to calculate smooth speed based on phase
inline float calculate_rocket_speed(float normalizedT,
									float &currentSpeed,
									float acceleration,
									float maxSpeed) noexcept
{
	const float launchEnd = 0.4f;
	const float cruiseEnd = 0.7f;

	if (normalizedT < launchEnd)
	{
		// Slow acceleration during launch
		float phaseT = normalizedT / launchEnd;
		float targetSpeed = maxSpeed * 0.4f * phaseT * phaseT; // Quadratic acceleration
		currentSpeed = currentSpeed + (targetSpeed - currentSpeed) * 0.1f;
	}
	else if (normalizedT < cruiseEnd)
	{
		// Maintain speed during cruise, slight deceleration at end
		float phaseT = (normalizedT - launchEnd) / (cruiseEnd - launchEnd);
		if (phaseT < 0.8f)
		{
			currentSpeed = maxSpeed;
		}
		else
		{
			// Start decelerating in last 20% of cruise
			float decel = (phaseT - 0.8f) / 0.2f;
			currentSpeed = maxSpeed * (1.0f - decel * 0.3f);
		}
	}
	else
	{
		// Significant deceleration during landing
		float phaseT = (normalizedT - cruiseEnd) / (1.0f - cruiseEnd);
		if (phaseT < 0.6f)
		{
			// Rapid deceleration during hover
			currentSpeed = maxSpeed * 0.5f * (1.0f - phaseT * 1.5f);
		}
		else
		{
			// Very slow final descent
			float descentT = (phaseT - 0.6f) / 0.4f;
			currentSpeed = maxSpeed * 0.1f * (1.0f - descentT);
		}
	}

	return currentSpeed;
}

#endif // MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
