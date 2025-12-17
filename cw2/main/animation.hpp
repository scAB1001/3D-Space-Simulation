#ifndef ANIMATION_HPP
#define ANIMATION_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

#include "config.hpp"

#include <algorithm>
#include <numbers>

struct Animation
{
    // Precomputed constants
    static constexpr float kTotalAnimationTime = 15.0f;          // Increased to 30s for slower ascent
    static constexpr float kVerticalAscentEnd = 0.25f; // unchanged
    static constexpr float kLaunchTiltEnd     = 0.40f + (.2f);
    static constexpr float kCruisePhaseEnd    = 0.75f;   // 70% for cruise (30% landing)
    static constexpr float kMaxAllowedHeight = 35.0f;            // Maximum height limit
    static constexpr float kMaxVelocity = 15.0f;                 // units per second
    static constexpr float kAccelerationRate = 1.2f;             // acceleration (units/sec^2)
    static constexpr Vec3f kLandingPadOffset = {0.f, 1.0f, 0.f}; // On the pad
    static constexpr float kArcEnd = 0.75f; // End of curved arc, start of landing

    // Flight profile constants
    static constexpr float kVerticalAscentHeight = 10.0f; // Height reached during pure vertical ascent
    static constexpr float kLaunchHeight = 15.0f;         // Maximum height during launch
    static constexpr float kCruiseHeight = 30.0f;         // Peak height during cruise
    static constexpr float kHoverHeight = 10.0f;          // Hover height above landing pad
    static constexpr float kBezierControlOffset = 25.0f;  // Control point offset
    static constexpr float kHorizontalScaleDist = 100.0f; // Distance for height scaling
    static constexpr float kCruiseDecelStart = 0.6f;      // When to start decelerating in cruise
    static constexpr float kLandingHoverFraction = 0.6f;  // 60% of landing is hover

    // Precomputed math constants
    static constexpr float kMaxTiltAngle = Config::kFloatPi / 6.0f;  // 30 degrees
    static constexpr float kMaxPitchAngle = Config::kFloatPi / 4.0f; // 45 degrees

    // Animation Phases
    enum class Phase
    {
        VerticalAscent, // 0% - 20% - Pure vertical ascent
        LaunchTilt,     // 20% - 40% - Tilt toward target while ascending
        Cruise,         // 40% - 70% - Curved trajectory, maintain speed
        Landing         // 70% - 100% - Hover and vertical descent
    } phase = Phase::VerticalAscent;

    // Animation state
    bool isAnimating = false;
    bool isPaused = false;
    float animationTime = 0.0f;
    float currentSpeed = 0.0f;
    float maxHeightReached = 0.0f;

    // Positions
    Vec3f startPosition;
    Vec3f endPosition;
    Vec3f currentPosition;
    Vec3f previousPosition;
    Vec3f velocity = Config::kZeroVec3;

    // Precomputed values for this specific trajectory
    float horizontalDistance = 0.0f;
    float distanceScale = 1.0f;
    float adjustedVerticalHeight = kVerticalAscentHeight;
    float adjustedLaunchHeight = kLaunchHeight;
    float adjustedCruiseHeight = kCruiseHeight;
    float adjustedHoverHeight = kHoverHeight;
    Vec3f targetDirection = Config::kZeroVec3;

    // Bezier control points (precomputed)
    Vec3f bezierP0, bezierP1, bezierP2, bezierP3;
    Vec3f verticalAscentEndPos; // Position at end of pure vertical ascent
    Vec3f launchEndPosition;    // Position at end of launch phase

    // Constructors (declaration only)
    Animation(const Vec3f &start, const Vec3f &end);
    Animation();

    // Public methods (declarations only)
    void precomputeTrajectory();
    float getNormalizedTime() const noexcept;
    Phase getCurrentPhase(float normalizedT) const noexcept;
    float getPhaseProgress() const noexcept;

    static float easeInQuad(float t) noexcept;
    static float easeOutQuad(float t) noexcept;
    static float easeInOutQuad(float t) noexcept;

    Vec3f calculatePosition(float t) const noexcept;
    float calculateSpeed(float t) const noexcept;

    void update(float dt) noexcept;
    void reset();
    void start();
    void togglePause();
};

// Declaration only
Mat44f calculate_rocket_rotation(const Animation &state) noexcept;

#endif // ANIMATION_HPP