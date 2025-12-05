#ifndef VEC3_HPP_5710DADF_17EF_453C_A9C8_4A73DC66B1CD
#define VEC3_HPP_5710DADF_17EF_453C_A9C8_4A73DC66B1CD
// SOLUTION_TAGS: gl-(ex-[^12]|cw-2|resit)

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
Vec3f calculateSpaceVehiclePosition(float t, float totalTime,
                                   const Vec3f& startPos,
                                   float maxHeight, float horizontalDist,
                                   float acceleration) noexcept
{
    // Normalized time (0 to 1)
    float normalizedT = t / totalTime;

    // Apply acceleration: slow start, faster later
    // Using ease-in cubic: t^3
    float easedT = normalizedT * normalizedT * normalizedT;

    // Horizontal movement (XZ plane) - curved path
    // Parabolic curve: x = t * distance, z = sin(t * 2π) * curveAmplitude
    float x = startPos.x + easedT * horizontalDist;
    float z = startPos.z + std::sin(easedT * 2.0f * 3.14159f) * (horizontalDist * 0.3f);

    // Vertical movement (Y) - parabolic trajectory with acceleration
    // Height follows a quadratic curve: y = -4h(t-0.5)^2 + h
    // Adjusted with acceleration factor
    float verticalT = easedT;
    float height = -4.0f * maxHeight * (verticalT - 0.5f) * (verticalT - 0.5f) + maxHeight;

    // Apply acceleration to height (starts slower)
    height *= std::min(1.0f, acceleration * normalizedT);

    // Ensure we don't go below starting height
    height = std::max(height, startPos.y);

    return Vec3f{x, startPos.y + height, z};
}

#endif // VEC3_HPP_5710DADF_17EF_453C_A9C8_4A73DC66B1CD
