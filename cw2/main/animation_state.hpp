#ifndef ANIMATION_STATE_HPP
#define ANIMATION_STATE_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

#include "config.hpp"

struct AnimationState
{
    // Precomputed constants
    static constexpr float kTotalAnimationTime = 25.0f;          // Total duration (25 seconds)
    static constexpr float kLaunchPhaseEnd = 0.4f;               // 40% for launch
    static constexpr float kCruisePhaseEnd = 0.7f;               // 70% for cruise (30% landing)
    static constexpr float kMaxAllowedHeight = 35.0f;            // Maximum height limit
    static constexpr float kMaxSpeed = 25.0f;                    // units per second
    static constexpr float kAccelerationRate = 1.5f;             // acceleration (units/sec²)
    static constexpr Vec3f kLandingPadOffset = {0.f, 1.0f, 0.f}; // On the pad

    // Flight profile constants
    static constexpr float kLaunchHeight = 15.0f;         // Maximum height during launch
    static constexpr float kCruiseHeight = 30.0f;         // Peak height during cruise
    static constexpr float kHoverHeight = 10.0f;          // Hover height above landing pad
    static constexpr float kBezierControlOffset = 20.0f;  // Control point offset
    static constexpr float kHorizontalScaleDist = 100.0f; // Distance for height scaling
    static constexpr float kCruiseDecelStart = 0.6f;      // When to start decelerating in cruise
    static constexpr float kLandingHoverFraction = 0.6f;  // 60% of landing is hover

    // Precomputed math constants
    static constexpr float kMaxTiltAngle = Config::kFloatPi / 6.0f;  // 30 degrees
    static constexpr float kMaxPitchAngle = Config::kFloatPi / 4.0f; // 45 degrees
    static constexpr float kAccelCurveMultiplier = 32.0f;            // Quintic multiplier
    static constexpr float kLaunchAccelFactor = 0.003f;              // Launch acceleration
    static constexpr float kMaxLaunchSpeed = kMaxSpeed * 0.4f;       // Max speed in launch
    static constexpr float kCruiseDecelFactor = 0.3f;                // Cruise deceleration
    static constexpr float kHoverSpeedFactor = 0.5f;                 // Hover speed
    static constexpr float kDescentSpeedFactor = 0.1f;               // Final descent speed

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
    Vec3f velocity = Config::kZeroVec3; // Fixed: Use direct initialization

    // Precomputed values for this specific trajectory
    float horizontalDistance = 0.0f;
    float distanceScale = 1.0f;
    float adjustedLaunchHeight = kLaunchHeight;
    float adjustedCruiseHeight = kCruiseHeight;
    float adjustedHoverHeight = kHoverHeight;
    Vec3f targetDirection = Config::kZeroVec3;

    // Animation Phase
    enum class Phase
    {
        Launch, // 0% - 40% - Vertical takeoff, slow acceleration
        Cruise, // 40% - 70% - Curved trajectory, maintain speed
        Landing // 70% - 100% - Hover and vertical descent
    } phase = Phase::Launch;

    // Constructor - precomputes trajectory constants
    AnimationState(const Vec3f &start, const Vec3f &end)
        : startPosition(start)
        , endPosition(end)
        , currentPosition(start)
        , previousPosition(start)
    {
        precomputeTrajectory();
    }

    // Default constructor (for initialization)
    AnimationState()
        : startPosition(Config::World::kLandingPad1Pos + kLandingPadOffset)
        , endPosition(Config::World::kLandingPad2Pos + kLandingPadOffset)
        , currentPosition(Config::World::kLandingPad1Pos + kLandingPadOffset)
        , previousPosition(Config::World::kLandingPad1Pos + kLandingPadOffset)
    {
        precomputeTrajectory();
    }

    // Precompute trajectory constants once
    void precomputeTrajectory()
    {
        // Calculate horizontal distance and direction
        Vec3f horizontalVec = endPosition - startPosition;
        horizontalVec.y = 0.f;
        horizontalDistance = length(horizontalVec);
        if (horizontalDistance > 0.001f)
        {
            targetDirection = horizontalVec / horizontalDistance;
        }

        // Scale heights based on horizontal distance
        distanceScale = std::min(horizontalDistance / kHorizontalScaleDist, 1.0f);
        adjustedLaunchHeight = kLaunchHeight * distanceScale;
        adjustedCruiseHeight = std::min(kCruiseHeight * distanceScale, kMaxAllowedHeight);
        adjustedHoverHeight = kHoverHeight * distanceScale;
    }

    // Helper methods
    float getNormalizedTime() const noexcept
    {
        return animationTime / kTotalAnimationTime;
    }

    Phase getCurrentPhase(float normalizedT) const noexcept
    {
        if (normalizedT < kLaunchPhaseEnd)
            return Phase::Launch;
        else if (normalizedT < kCruisePhaseEnd)
            return Phase::Cruise;
        else
            return Phase::Landing;
    }

    // TODO: Remove later. For debugging.
    const char *getPhaseName() const noexcept
    {
        switch (phase)
        {
        case Phase::Launch:
            return "LAUNCH";
        case Phase::Cruise:
            return "CRUISE";
        case Phase::Landing:
            return "LANDING";
        default:
            return "UNKNOWN";
        }
    }

    // TODO: Remove later. For debugging.
    const char *getPhaseNameSpecific(Phase phase) const noexcept
    {
        switch (phase)
        {
            using enum Phase;
        case Launch:
            return "LAUNCH";
        case Cruise:
            return "CRUISE";
        case Landing:
            return "LANDING";
        default:
            return "UNKNOWN";
        }
    }

    float getPhaseProgress() const noexcept
    {
        float normalizedT = getNormalizedTime();
        switch (phase)
        {
        case Phase::Launch:
            return normalizedT / kLaunchPhaseEnd;
        case Phase::Cruise:
            return (normalizedT - kLaunchPhaseEnd) / (kCruisePhaseEnd - kLaunchPhaseEnd);
        case Phase::Landing:
            return (normalizedT - kCruisePhaseEnd) / (1.0f - kCruisePhaseEnd);
        default:
            return 0.0f;
        }
    }

    // TODO: Remove later. For debugging.
    void printCoordinates(Vec3f position) const
    {
        std::print("position: ({:.2f}, {:.2f}, {:.2f})\n",
                   position.x,
                   position.y,
                   position.z);
    }

    // Reset all animation state
    void reset()
    {
        isAnimating = false;
        isPaused = false;
        animationTime = 0.0f;
        currentPosition = startPosition;
        previousPosition = startPosition;
        velocity = Config::kZeroVec3;
        currentSpeed = 0.0f;
        phase = Phase::Launch;
        maxHeightReached = 0.0f;
    }

    // Start animation
    void start()
    {
        isAnimating = true;
        isPaused = false;
        animationTime = 0.0f;
        currentSpeed = 0.0f;
        currentPosition = startPosition;
        previousPosition = startPosition;
        phase = Phase::Launch;
        maxHeightReached = 0.0f;
    }

    // Toggle pause
    void togglePause()
    {
        isPaused = !isPaused;
    }
};

// Precomputed animation curves (fast lookups)
namespace AnimationCurves
{
    // Fast quintic ease-in approximation
    inline float rocket_acceleration_curve(float t) noexcept
    {
        // Precomputed polynomial: t⁵ for t < 0.5, cubic ease-out otherwise
        if (t < 0.5f)
        {
            float t2 = t * t;                                      // t²
            float t4 = t2 * t2;                                    // t⁴
            return t4 * t * AnimationState::kAccelCurveMultiplier; // t⁵ * 32
        }
        else
        {
            float u = 1.0f - t;
            float u2 = u * u;
            return 1.0f - u * u2; // 1 - (1-t)³
        }
    }

    // Fast cubic ease-out approximation
    inline float rocket_deceleration_curve(float t) noexcept
    {
        float u = 1.0f - t;
        float u2 = u * u;
        return 1.0f - u * u2; // 1 - (1-t)³
    }
}

inline Vec3f calculate_rocket_position(float t, float totalTime,
                                       const Vec3f &start, const Vec3f &end,
                                       float &outSpeed,
                                       float acceleration,
                                       float maxSpeed,
                                       const AnimationState &state) noexcept
{
    using namespace AnimationCurves;

    // Normalized time (0 to 1)
    float normalizedT = t / totalTime;
    Vec3f position;

    // PHASE 1: LAUNCH (0% - 40%)
    if (normalizedT < AnimationState::kLaunchPhaseEnd)
    {
        float phaseT = normalizedT / AnimationState::kLaunchPhaseEnd;
        float easedT = rocket_acceleration_curve(phaseT * 0.5f) * 2.0f;

        // Fast horizontal progress calculation (easedT² * 0.3)
        float horizontalProgress = easedT * easedT * 0.3f;

        // Use precomputed values
        position.x = start.x + (end.x - start.x) * horizontalProgress;
        position.y = start.y + state.adjustedLaunchHeight * easedT;
        position.z = start.z + (end.z - start.z) * horizontalProgress;

        // Fast acceleration
        outSpeed = std::min(outSpeed + acceleration * AnimationState::kLaunchAccelFactor,
                            AnimationState::kMaxLaunchSpeed);
    }
    // PHASE 2: CRUISE (40% - 70%)
    else if (normalizedT < AnimationState::kCruisePhaseEnd)
    {
        float phaseT = (normalizedT - AnimationState::kLaunchPhaseEnd) /
                       (AnimationState::kCruisePhaseEnd - AnimationState::kLaunchPhaseEnd);

        // Precomputed control points
        Vec3f p0 = start + Vec3f{0.f, state.adjustedLaunchHeight, 0.f};
        Vec3f p1 = p0 + state.targetDirection * AnimationState::kBezierControlOffset +
                   Vec3f{0.f, AnimationState::kBezierControlOffset, 0.f};
        Vec3f p2 = end + Vec3f{0.f, state.adjustedCruiseHeight, 0.f} -
                   state.targetDirection * AnimationState::kBezierControlOffset;
        Vec3f p3 = end + Vec3f{0.f, state.adjustedHoverHeight, 0.f};

        // Fast ease-in-out
        float easedT;
        if (phaseT < 0.5f)
        {
            easedT = 2.0f * phaseT * phaseT;
        }
        else
        {
            float tmp = -2.0f * phaseT + 2.0f;
            easedT = 1.0f - tmp * tmp * 0.5f;
        }

        position = calculate_bezier_position(easedT, p0, p1, p2, p3);

        // Maintain/decay speed
        if (phaseT > AnimationState::kCruiseDecelStart)
        {
            float decel = (phaseT - AnimationState::kCruiseDecelStart) /
                          (1.0f - AnimationState::kCruiseDecelStart);
            outSpeed = maxSpeed * (1.0f - decel * AnimationState::kCruiseDecelFactor);
        }
        else
        {
            outSpeed = maxSpeed;
        }
    }
    // PHASE 3: LANDING (70% - 100%)
    else
    {
        float phaseT = (normalizedT - AnimationState::kCruisePhaseEnd) /
                       (1.0f - AnimationState::kCruisePhaseEnd);

        if (phaseT < AnimationState::kLandingHoverFraction)
        {
            float hoverT = phaseT / AnimationState::kLandingHoverFraction;
            float easedT = rocket_deceleration_curve(hoverT);

            float height = state.adjustedHoverHeight * (1.0f - easedT * 0.7f);
            position = end + Vec3f{0.f, height, 0.f};
            outSpeed = maxSpeed * AnimationState::kHoverSpeedFactor * (1.0f - hoverT);
        }
        else
        {
            float descentT = (phaseT - AnimationState::kLandingHoverFraction) /
                             (1.0f - AnimationState::kLandingHoverFraction);
            float easedT = rocket_deceleration_curve(descentT);

            float startHeight = state.adjustedHoverHeight * 0.3f;
            float height = startHeight * (1.0f - easedT);
            position = end + Vec3f{0.f, height, 0.f};
            outSpeed = maxSpeed * AnimationState::kDescentSpeedFactor * (1.0f - descentT);
        }
    }

    // Safety clamp
    float heightAboveStart = position.y - start.y;
    if (heightAboveStart > AnimationState::kMaxAllowedHeight)
    {
        position.y = start.y + AnimationState::kMaxAllowedHeight;
    }

    return position;
}

inline Mat44f calculate_rocket_rotation(const Vec3f &currentPos,
                                        const Vec3f &previousPos,
                                        const Vec3f &velocity,
                                        AnimationState::Phase phase,
                                        float phaseProgress,
                                        const AnimationState &state) noexcept
{
    Mat44f rotation = kIdentity44f;

    if (length(velocity) > 0.001f)
    {
        Vec3f forwardDir = normalize(velocity);

        switch (phase)
        {
        case AnimationState::Phase::Launch:
        {
            // Use precomputed target direction
            float yaw = atan2(state.targetDirection.x, state.targetDirection.z);
            float tiltAngle = phaseProgress * AnimationState::kMaxTiltAngle;
            rotation = make_rotation_y(yaw) * make_rotation_x(-tiltAngle);
            break;
        }

        case AnimationState::Phase::Cruise:
        {
            float yaw = atan2(forwardDir.x, forwardDir.z);
            float pitch = -asin(forwardDir.y);

            // Clamp pitch
            pitch = std::clamp(pitch, -AnimationState::kMaxPitchAngle,
                               AnimationState::kMaxPitchAngle);

            // Reduce pitch toward end
            if (phaseProgress > AnimationState::kCruiseDecelStart)
            {
                float reduce = (phaseProgress - AnimationState::kCruiseDecelStart) /
                               (1.0f - AnimationState::kCruiseDecelStart);
                pitch *= (1.0f - reduce * 0.8f);
            }

            rotation = make_rotation_y(yaw) * make_rotation_x(pitch);
            break;
        }

        case AnimationState::Phase::Landing:
        {
            if (phaseProgress < AnimationState::kLandingHoverFraction)
            {
                float orientProgress = phaseProgress / AnimationState::kLandingHoverFraction;
                float yaw = atan2(forwardDir.x, forwardDir.z);
                float currentPitch = -asin(forwardDir.y);
                float pitch = currentPitch * (1.0f - orientProgress);
                rotation = make_rotation_y(yaw) * make_rotation_x(pitch);
            }
            break;
        }
        }
    }

    return rotation;
}

inline float calculate_rocket_speed(AnimationState::Phase phase,
                                    float phaseProgress,
                                    float &currentSpeed,
                                    float acceleration,
                                    float maxSpeed) noexcept
{
    switch (phase)
    {
    case AnimationState::Phase::Launch:
    {
        float targetSpeed = maxSpeed * 0.4f * phaseProgress * phaseProgress;
        currentSpeed += (targetSpeed - currentSpeed) * 0.1f;
        break;
    }

    case AnimationState::Phase::Cruise:
    {
        if (phaseProgress < AnimationState::kCruiseDecelStart)
            currentSpeed = maxSpeed;
        else
            currentSpeed = maxSpeed * (1.0f - (phaseProgress - AnimationState::kCruiseDecelStart) /
                                                  (1.0f - AnimationState::kCruiseDecelStart) *
                                                  AnimationState::kCruiseDecelFactor);
        break;
    }

    case AnimationState::Phase::Landing:
    {
        if (phaseProgress < AnimationState::kLandingHoverFraction)
            currentSpeed = maxSpeed * AnimationState::kHoverSpeedFactor *
                           (1.0f - phaseProgress * 1.5f);
        else
            currentSpeed = maxSpeed * AnimationState::kDescentSpeedFactor *
                           (1.0f - (phaseProgress - AnimationState::kLandingHoverFraction) /
                                       (1.0f - AnimationState::kLandingHoverFraction));
        break;
    }
    }

    return currentSpeed;
}

#endif // ANIMATION_STATE_HPP