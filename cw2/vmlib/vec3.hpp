#ifndef VEC3_HPP_5710DADF_17EF_453C_A9C8_4A73DC66B1CD
#define VEC3_HPP_5710DADF_17EF_453C_A9C8_4A73DC66B1CD

#include <cmath>
#include <cassert>
#include <cstdlib>

struct Vec3f
{
	float x, y, z;

	constexpr
	float& operator[] (std::size_t aI) noexcept
	{
		assert( aI < 3 );
		return aI[&x]; // This is a bit sketchy.
	}
	constexpr
	float operator[] (std::size_t aI) const noexcept
	{
		assert( aI < 3 );
		return aI[&x]; // This is a bit sketchy.
	}
};


constexpr
Vec3f operator+( Vec3f aVec ) noexcept
{
	return aVec;
}
constexpr
Vec3f operator-( Vec3f aVec ) noexcept
{
	return { -aVec.x, -aVec.y, -aVec.z };
}

constexpr
Vec3f operator+( Vec3f aLeft, Vec3f aRight ) noexcept
{
	return Vec3f{
		aLeft.x + aRight.x,
		aLeft.y + aRight.y,
		aLeft.z + aRight.z
	};
}
constexpr
Vec3f operator-( Vec3f aLeft, Vec3f aRight ) noexcept
{
	return Vec3f{
		aLeft.x - aRight.x,
		aLeft.y - aRight.y,
		aLeft.z - aRight.z
	};
}


constexpr
Vec3f operator*( float aScalar, Vec3f aVec ) noexcept
{
	return Vec3f{
		aScalar * aVec.x,
		aScalar * aVec.y,
		aScalar * aVec.z
	};
}
constexpr
Vec3f operator*( Vec3f aVec, float aScalar ) noexcept
{
	return aScalar * aVec;
}

constexpr
Vec3f operator/( Vec3f aVec, float aScalar ) noexcept
{
	return Vec3f{
		aVec.x / aScalar,
		aVec.y / aScalar,
		aVec.z / aScalar
	};
}


constexpr
Vec3f& operator+=( Vec3f& aLeft, Vec3f aRight ) noexcept
{
	aLeft.x += aRight.x;
	aLeft.y += aRight.y;
	aLeft.z += aRight.z;
	return aLeft;
}
constexpr
Vec3f& operator-=( Vec3f& aLeft, Vec3f aRight ) noexcept
{
	aLeft.x -= aRight.x;
	aLeft.y -= aRight.y;
	aLeft.z -= aRight.z;
	return aLeft;
}

constexpr
Vec3f& operator*=( Vec3f& aLeft, float aRight ) noexcept
{
	aLeft.x *= aRight;
	aLeft.y *= aRight;
	aLeft.z *= aRight;
	return aLeft;
}
constexpr
Vec3f& operator/=( Vec3f& aLeft, float aRight ) noexcept
{
	aLeft.x /= aRight;
	aLeft.y /= aRight;
	aLeft.z /= aRight;
	return aLeft;
}


// A few common functions:

constexpr
float dot( Vec3f aLeft, Vec3f aRight ) noexcept
{
	return aLeft.x * aRight.x
		+ aLeft.y * aRight.y
		+ aLeft.z * aRight.z
	;
}

inline
Vec3f cross( Vec3f aLeft, Vec3f aRight ) noexcept
{
	/* References for code inspiration:
	 * - https://registry.khronos.org/OpenGL-Refpages/gl4/html/cross.xhtml
	 * - https://en.wikipedia.org/wiki/Cross_product
	 */
	return Vec3f{
		aLeft.y * aRight.z - aLeft.z * aRight.y,
		aLeft.z * aRight.x - aLeft.x * aRight.z,
		aLeft.x * aRight.y - aLeft.y * aRight.x
	};
}

inline
float length( Vec3f aVec ) noexcept
{
	// The standard function std::sqrt() is not marked as constexpr. length()
	// calls std::sqrt() unconditionally, so length() cannot be marked
	// constexpr itself.
	return std::sqrt( dot( aVec, aVec ) );
}

inline
Vec3f normalize( Vec3f aVec ) noexcept
{
	auto const l = length( aVec );
	return aVec / l;
}

inline
Vec3f calculate_bezier_position(float t,
								const Vec3f &p0,
								const Vec3f &p1,
								const Vec3f &p2,
								const Vec3f &p3) noexcept
{
	/* References for code inspiration:
	 * - https://en.wikipedia.org/wiki/B%C3%A9zier_curve
	 * - https://javascript.info/bezier-curve

	 * Q[i,j] = sum(k=0 to 3) L[i,k] * R[k,j]
	*/

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

inline
Vec3f mix(const Vec3f &a, const Vec3f &b, float t) noexcept
{
	/* References for code inspiration:
	 * - https://www.tutorialspoint.com/computer_graphics/computer_graphics_linear_interpolation.htm
	 */

	// Validate inputs
	if (!std::isfinite(a.x) || !std::isfinite(a.y) || !std::isfinite(a.z))
	{
		return b;
	}
	if (!std::isfinite(b.x) || !std::isfinite(b.y) || !std::isfinite(b.z))
	{
		return a;
	}
	if (!std::isfinite(t) || t < 0.0f || t > 1.0f)
	{
		return b;
	}

	// Linear interpolation for Vec3f
	Vec3f result = a * (1.0f - t) + b * t;

	if (!std::isfinite(result.x) || !std::isfinite(result.y) || !std::isfinite(result.z))
	{
		return b;
	}

	return result;
}

#endif // VEC3_HPP_5710DADF_17EF_453C_A9C8_4A73DC66B1CD
