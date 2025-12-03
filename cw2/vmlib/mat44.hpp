#ifndef MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
#define MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
// SOLUTION_TAGS: gl-(ex-[^12]|cw-2|resit)

#include <cmath>
#include <cassert>
#include <cstdlib>

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
	// TODO: your implementation goes here
	// TODO: remove the following when you start your implementation
	//  (void)aLeft;   // Avoid warnings about unused arguments until the function
	//  (void)aRight;  // is properly implemented.
	//  return kIdentity44f;

	// Q[i,j] = sum(k=0 to 3) L[i,k] * R[k,j]
	// Does this need to be initialized to zero first?
	// Does this need to be constexpr?
	Mat44f result{};
	Mat44f rightTransposed = transpose(aRight); // Approach 2

	for (std::size_t i = 0; i < 4; ++i)
	{
		for (std::size_t j = 0; j < 4; ++j)
		{
			// Approach 1: Manual dot product Left row i and Right column j
			// std::size_t rowBase = i * 4;
			// std::size_t colBase = j;

			// result[i, j] = aLeft.v[rowBase + 0] * aRight[0, colBase] +
			// 			   aLeft.v[rowBase + 1] * aRight[1, colBase] +
			// 			   aLeft.v[rowBase + 2] * aRight[2, colBase] +
			// 			   aLeft.v[rowBase + 3] * aRight[3, colBase];

			// Approach 2: Use a transposed aRight to access rows with a single index
			for (std::size_t k = 0; k < 4; ++k)
			{
				result[i, j] += aLeft[i, k] * rightTransposed[j, k];
			}
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
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aAngle; // Avoid warnings about unused arguments until the function
	//               // is properly implemented.
	// return kIdentity44f;

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
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aAngle; // Avoid warnings about unused arguments until the function
	//               // is properly implemented.
	// return kIdentity44f;

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
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aAngle; // Avoid warnings about unused arguments until the function
	//               // is properly implemented.
	// return kIdentity44f;

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
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aTranslation; // Avoid warnings about unused arguments until the function
	//                     // is properly implemented.
	// return kIdentity44f;

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
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aSX;  // Avoid warnings about unused arguments until the function
	// (void)aSY;  // is properly implemented.
	// (void)aSZ;
	// return kIdentity44f;

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
	//TODO: your implementation goes here
	//TODO: remove the following when you start your implementation
	// (void)aFovInRadians; // Avoid warnings about unused arguments until the function
	// (void)aAspect;       // is properly implemented.
	// (void)aNear;
	// (void)aFar;
	// return kIdentity44f;

	/*
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

#endif // MAT44_HPP_E7187A26_469E_48AD_A3D2_63150F05A4CA
