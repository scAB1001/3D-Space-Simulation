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

// Helper function implementation
inline Vec3f calculate_optimal_side_position(const Vec3f &vehiclePos,
                                             const Vec3f &flightDir,
                                             float sideDistance,
                                             float height,
                                             float rearDistance,
                                             bool useRightSide)
{
    // Ensure flight direction is valid
    Vec3f normalizedFlightDir = length(flightDir) > 0.001f ? normalize(flightDir) : Vec3f{1.f, 0.f, 0.f};

    // Calculate perpendicular direction (side view)
    Vec3f sideDir = normalize(cross(normalizedFlightDir, Vec3f{0.f, 1.f, 0.f}));
    if (length(sideDir) < 0.001f)
        sideDir = Vec3f{0.f, 0.f, 1.f}; // Fallback

    // Apply side multiplier
    float sideMultiplier = useRightSide ? 1.0f : -1.0f;

    // Calculate offset: side + height + slightly behind
    Vec3f offset =
        sideDir * (sideDistance * sideMultiplier) +
        Vec3f{0.f, height, 0.f} +
        -normalizedFlightDir * rearDistance;

    return vehiclePos + offset;
}


inline
Vec3f mix(const Vec3f &a, const Vec3f &b, float t) noexcept
{
	// DEBUG
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

	Vec3f result = a * (1.0f - t) + b * t;

	if (!std::isfinite(result.x) || !std::isfinite(result.y) || !std::isfinite(result.z))
	{
		return b;
	}

	return result;
}

#endif // VEC3_HPP_5710DADF_17EF_453C_A9C8_4A73DC66B1CD
