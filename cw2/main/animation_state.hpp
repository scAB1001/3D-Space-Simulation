#ifndef ANIMATION_STATE_HPP
#define ANIMATION_STATE_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"

#include "config.hpp"
#include <print>
#include <numbers>

// /* Has a weird cruise and acceleration
struct AnimationState
{
    // Precomputed constants
    static constexpr float kTotalAnimationTime = 30.0f;          // Increased to 30s for slower ascent
    static constexpr float kVerticalAscentEnd = 0.2f;            // 20% for pure vertical ascent
    static constexpr float kLaunchTiltEnd = 0.4f;                // 40% total for launch (20% vertical + 20% tilt)
    static constexpr float kCruisePhaseEnd = 0.7f;               // 70% for cruise (30% landing)
    static constexpr float kMaxAllowedHeight = 35.0f;            // Maximum height limit
    static constexpr float kMaxVelocity = 15.0f;                    // units per second
    static constexpr float kAccelerationRate = 1.2f;             // acceleration (units/sec²)
    static constexpr Vec3f kLandingPadOffset = {0.f, 1.0f, 0.f}; // On the pad

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
    Vec3f launchEndPosition;    // Position at end of launch (tilt) phase

    // Animation Phase
    enum class Phase
    {
        VerticalAscent, // 0% - 20% - Pure vertical ascent
        LaunchTilt,     // 20% - 40% - Tilt toward target while ascending
        Cruise,         // 40% - 70% - Curved trajectory, maintain speed
        Landing         // 70% - 100% - Hover and vertical descent
    } phase = Phase::VerticalAscent;

    // Constructor - precomputes trajectory constants
    AnimationState(const Vec3f &start, const Vec3f &end)
        : startPosition(start), endPosition(end), currentPosition(start), previousPosition(start)
    {
        precomputeTrajectory();
    }

    // Default constructor (for initialization)
    AnimationState()
        : startPosition(Config::World::kLandingPad1Pos + kLandingPadOffset), endPosition(Config::World::kLandingPad2Pos + kLandingPadOffset), currentPosition(Config::World::kLandingPad1Pos + kLandingPadOffset), previousPosition(Config::World::kLandingPad1Pos + kLandingPadOffset)
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
        adjustedVerticalHeight = kVerticalAscentHeight * distanceScale;
        adjustedLaunchHeight = kLaunchHeight * distanceScale;
        adjustedCruiseHeight = std::min(kCruiseHeight * distanceScale, kMaxAllowedHeight);
        adjustedHoverHeight = kHoverHeight * distanceScale;

        // Calculate vertical ascent end position (straight up)
        verticalAscentEndPos = Vec3f{
            startPosition.x,
            startPosition.y + adjustedVerticalHeight,
            startPosition.z};

        // Calculate launch end position (after tilt phase)
        float tiltPhaseProgress = 0.8f; // 80% horizontal progress by end of tilt
        launchEndPosition = Vec3f{
            startPosition.x + (endPosition.x - startPosition.x) * tiltPhaseProgress,
            startPosition.y + adjustedLaunchHeight,
            startPosition.z + (endPosition.z - startPosition.z) * tiltPhaseProgress};

        // Precompute Bezier control points for cruise phase
        bezierP0 = launchEndPosition; // Start of cruise = end of launch
        bezierP1 = bezierP0 + targetDirection * kBezierControlOffset +
                   Vec3f{0.f, kBezierControlOffset * 0.8f, 0.f};
        bezierP2 = endPosition + Vec3f{0.f, adjustedCruiseHeight, 0.f} -
                   targetDirection * kBezierControlOffset * 0.7f;
        bezierP3 = endPosition + Vec3f{0.f, adjustedHoverHeight, 0.f};

        std::print("Precomputed trajectory:\n");
        std::print("  Horizontal distance: {:.2f}\n", horizontalDistance);
        std::print("  Vertical ascent height: {:.2f}\n", adjustedVerticalHeight);
        std::print("  Vertical ascent end: ({:.2f}, {:.2f}, {:.2f})\n",
                   verticalAscentEndPos.x, verticalAscentEndPos.y, verticalAscentEndPos.z);
        std::print("  Launch end position: ({:.2f}, {:.2f}, {:.2f})\n",
                   launchEndPosition.x, launchEndPosition.y, launchEndPosition.z);
    }

    // Helper methods
    float getNormalizedTime() const noexcept
    {
        return animationTime / kTotalAnimationTime;
    }

    Phase getCurrentPhase(float normalizedT) const noexcept
    {
        if (normalizedT < kVerticalAscentEnd)
            return Phase::VerticalAscent;
        else if (normalizedT < kLaunchTiltEnd)
            return Phase::LaunchTilt;
        else if (normalizedT < kCruisePhaseEnd)
            return Phase::Cruise;
        else
            return Phase::Landing;
    }

    const char *getPhaseName() const noexcept
    {
        switch (phase)
        {
        case Phase::VerticalAscent:
            return "VERTICAL ASCENT";
        case Phase::LaunchTilt:
            return "LAUNCH TILT";
        case Phase::Cruise:
            return "CRUISE";
        case Phase::Landing:
            return "LANDING";
        default:
            return "UNKNOWN";
        }
    }

    const char *getPhaseNameSpecific(Phase phase) const noexcept
    {
        switch (phase)
        {
            using enum Phase;
        case VerticalAscent:
            return "VERTICAL ASCENT";
        case LaunchTilt:
            return "LAUNCH TILT";
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
        case Phase::VerticalAscent:
            return normalizedT / kVerticalAscentEnd;
        case Phase::LaunchTilt:
            return (normalizedT - kVerticalAscentEnd) / (kLaunchTiltEnd - kVerticalAscentEnd);
        case Phase::Cruise:
            return (normalizedT - kLaunchTiltEnd) / (kCruisePhaseEnd - kLaunchTiltEnd);
        case Phase::Landing:
            return (normalizedT - kCruisePhaseEnd) / (1.0f - kCruisePhaseEnd);
        default:
            return 0.0f;
        }
    }

    // Easing functions for smooth acceleration
    static float easeInQuad(float t) noexcept { return t * t; }
    static float easeOutQuad(float t) noexcept { return 1.0f - (1.0f - t) * (1.0f - t); }
    static float easeInOutQuad(float t) noexcept
    {
        return t < 0.5f ? 2.0f * t * t : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * 0.5f;
    }

    // Unified position calculation
    Vec3f calculatePosition(float t) const noexcept
    {
        float normalizedT = t / kTotalAnimationTime;

        // PHASE 1: VERTICAL ASCENT (0% - 20%)
        if (normalizedT < kVerticalAscentEnd)
        {
            float phaseT = normalizedT / kVerticalAscentEnd;

            // Slow start with quadratic ease-in
            float heightProgress = easeInQuad(phaseT);

            // Pure vertical - no horizontal movement
            return Vec3f{
                startPosition.x,
                startPosition.y + adjustedVerticalHeight * heightProgress,
                startPosition.z};
        }
        // PHASE 2: LAUNCH TILT (20% - 40%)
        else if (normalizedT < kLaunchTiltEnd)
        {
            float phaseT = (normalizedT - kVerticalAscentEnd) / (kLaunchTiltEnd - kVerticalAscentEnd);

            // Continue ascending while gradually moving horizontally
            float heightProgress = phaseT; // Linear continuation
            // float totalHeightProgress = kVerticalAscentEnd + phaseT * (kLaunchTiltEnd - kVerticalAscentEnd);
            float targetHeight = adjustedVerticalHeight + (adjustedLaunchHeight - adjustedVerticalHeight) * heightProgress;

            // Gradually increase horizontal movement (cubic ease-in)
            float horizontalEase = phaseT * phaseT * phaseT;
            float horizontalProgress = 0.8f * horizontalEase; // Reach 80% by end

            return Vec3f{
                startPosition.x + (endPosition.x - startPosition.x) * horizontalProgress,
                startPosition.y + targetHeight,
                startPosition.z + (endPosition.z - startPosition.z) * horizontalProgress};
        }
        // PHASE 3: CRUISE (40% - 70%)
        else if (normalizedT < kCruisePhaseEnd)
        {
            float phaseT = (normalizedT - kLaunchTiltEnd) / (kCruisePhaseEnd - kLaunchTiltEnd);

            // Smooth ease-in-out for cruise
            float easedT = easeInOutQuad(phaseT);

            // Use precomputed Bezier curve
            return calculate_bezier_position(easedT, bezierP0, bezierP1, bezierP2, bezierP3);
        }
        // PHASE 4: LANDING (70% - 100%)
        else
        {
            float phaseT = (normalizedT - kCruisePhaseEnd) / (1.0f - kCruisePhaseEnd);

            if (phaseT < kLandingHoverFraction)
            {
                float hoverT = phaseT / kLandingHoverFraction;
                // Smooth descent from hover height (ease-out)
                float height = adjustedHoverHeight * (1.0f - easeOutQuad(hoverT) * 0.7f);
                return Vec3f{
                    endPosition.x,
                    endPosition.y + height,
                    endPosition.z};
            }
            else
            {
                float descentT = (phaseT - kLandingHoverFraction) / (1.0f - kLandingHoverFraction);
                // Final slow descent (ease-in)
                float height = adjustedHoverHeight * 0.3f * (1.0f - easeInQuad(descentT));
                return Vec3f{
                    endPosition.x,
                    endPosition.y + height,
                    endPosition.z};
            }
        }
    }

    // Unified speed calculation with slow acceleration
    float calculateSpeed(float t) const noexcept
    {
        float normalizedT = t / kTotalAnimationTime;

        if (normalizedT < kVerticalAscentEnd)
        {
            // Very slow acceleration during vertical ascent
            float phaseT = normalizedT / kVerticalAscentEnd;
            return kMaxVelocity * 0.1f * easeInQuad(phaseT); // Max 10% speed
        }
        else if (normalizedT < kLaunchTiltEnd)
        {
            // Moderate acceleration during tilt phase
            float phaseT = (normalizedT - kVerticalAscentEnd) / (kLaunchTiltEnd - kVerticalAscentEnd);
            return kMaxVelocity * (0.1f + 0.3f * easeInQuad(phaseT)); // Reach 40% speed
        }
        else if (normalizedT < kCruisePhaseEnd)
        {
            float phaseT = (normalizedT - kLaunchTiltEnd) / (kCruisePhaseEnd - kLaunchTiltEnd);

            if (phaseT < 0.3f)
            {
                // Accelerate to full speed in first 30% of cruise
                float accelT = phaseT / 0.3f;
                return kMaxVelocity * (0.4f + 0.6f * easeInQuad(accelT)); // Reach 100% speed
            }
            else if (phaseT < kCruiseDecelStart)
            {
                // Maintain max speed
                return kMaxVelocity;
            }
            else
            {
                // Decelerate toward end of cruise
                float decel = (phaseT - kCruiseDecelStart) / (1.0f - kCruiseDecelStart);
                return kMaxVelocity * (1.0f - decel * 0.4f); // Gentle deceleration
            }
        }
        else
        {
            float phaseT = (normalizedT - kCruisePhaseEnd) / (1.0f - kCruisePhaseEnd);

            if (phaseT < kLandingHoverFraction)
            {
                // Slow hover descent
                float hoverT = phaseT / kLandingHoverFraction;
                return kMaxVelocity * 0.3f * (1.0f - easeOutQuad(hoverT)); // Start at 30%, slow to 0
            }
            else
            {
                // Very slow final descent
                float descentT = (phaseT - kLandingHoverFraction) / (1.0f - kLandingHoverFraction);
                return kMaxVelocity * 0.1f * (1.0f - easeInQuad(descentT)); // Very slow
            }
        }
    }

    // Update animation state
    void update(float dt) noexcept
    {
        if (!isAnimating || isPaused)
            return;

        animationTime += dt;

        // Store previous position for velocity calculation
        previousPosition = currentPosition;

        // Calculate new position and speed
        currentPosition = calculatePosition(animationTime);
        currentSpeed = calculateSpeed(animationTime);

        // Calculate velocity
        if (dt > 0.001f)
        {
            velocity = (currentPosition - previousPosition) / dt;
        }

        // Update phase
        float normalizedT = getNormalizedTime();
        phase = getCurrentPhase(normalizedT);

        // Track max height
        float currentHeight = currentPosition.y - startPosition.y;
        if (currentHeight > maxHeightReached)
        {
            maxHeightReached = currentHeight;
        }

        // Clamp to max height
        if (currentHeight > kMaxAllowedHeight)
        {
            currentPosition.y = startPosition.y + kMaxAllowedHeight;
        }
    }

    // TODO: Remove later. For debugging.
    void printCoordinates(Vec3f position) const
    {
        std::print("Position: ({:.2f}, {:.2f}, {:.2f})\n",
                   position.x,
                   position.y,
                   position.z);
    }

    void printDebugOutput(float &lastDebugTime) const
    {
        if (animationTime - lastDebugTime > 2.0f)
        {
            std::print("[Flight] {} | Time: {:.2f}s | Altitude: {:.2f} | Speed: {:.2f} \n",
                    getPhaseName(),
                    animationTime,
                    currentPosition.y - startPosition.y,
                    currentSpeed
            );
            printCoordinates(currentPosition);
            lastDebugTime = animationTime;
        }
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
        phase = Phase::VerticalAscent;
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
        phase = Phase::VerticalAscent;
        maxHeightReached = 0.0f;

        std::print("\nAnimation STARTED\n");
        std::print("-----------------------------\n");
        std::print("  Phase: {}\n", getPhaseName());
        std::print("  Start position: ({:.2f}, {:.2f}, {:.2f})\n",
                   startPosition.x, startPosition.y, startPosition.z);
        std::print("  End position: ({:.2f}, {:.2f}, {:.2f})\n",
                   endPosition.x, endPosition.y, endPosition.z);
        std::print("  Duration: {:.2f} seconds\n", kTotalAnimationTime);
        std::print("  Max speed: {:.2f} units/s\n", kMaxVelocity);
        std::print("  Max height allowed: {:.2f} units\n", kMaxAllowedHeight);
    }

    // Toggle pause
    void togglePause()
    {
        isPaused = !isPaused;
        if (isPaused)
        {
            std::print("\nAnimation PAUSED\n");
            std::print("-----------------------------\n");
            std::print("  Phase: {}\n", getPhaseName());
            std::print("  Current position: ({:.2f}, {:.2f}, {:.2f})\n",
                       currentPosition.x, currentPosition.y, currentPosition.z);
            std::print("  Speed: {:.2f} units/sec\n", currentSpeed);
            std::print("  Max height so far: {:.2f} units\n", maxHeightReached);
        }
        else
        {
            std::print("\nAnimation UNPAUSED\n");
            std::print("-----------------------------\n");
            std::print("  Phase: {}\n", getPhaseName());
        }
    }
};

// Calculate rocket rotation with proper vertical ascent
inline Mat44f calculate_rocket_rotation(const AnimationState &state) noexcept
{
    Mat44f rotation = kIdentity44f;

    switch (state.phase)
    {
        case AnimationState::Phase::VerticalAscent:
        {
            // Perfectly vertical during ascent
            // Small idle rotation for visual interest
            float idleRotation = state.animationTime * 0.5f; // Slow rotation
            rotation = make_rotation_y(idleRotation);
            break;
        }

        case AnimationState::Phase::LaunchTilt:
        {
            // Gradually tilt toward target direction
            float phaseProgress = state.getPhaseProgress();

            // Calculate target yaw (direction to landing pad)
            float targetYaw = atan2(state.targetDirection.x, state.targetDirection.z);

            // Interpolate from vertical (0) to target yaw
            float currentYaw = targetYaw * phaseProgress;

            // Tilt angle increases with phase progress
            float tiltAngle = AnimationState::kMaxTiltAngle * phaseProgress;

            rotation = make_rotation_y(currentYaw) * make_rotation_x(-tiltAngle);
            break;
        }

        case AnimationState::Phase::Cruise:
        {
            if (length(state.velocity) > 0.001f)
            {
                // Follow velocity direction
                Vec3f forwardDir = normalize(state.velocity);
                float yaw = atan2(forwardDir.x, forwardDir.z);
                float pitch = -asin(forwardDir.y);

                // Clamp pitch
                pitch = std::clamp(pitch, -AnimationState::kMaxPitchAngle,
                                AnimationState::kMaxPitchAngle);

                rotation = make_rotation_y(yaw) * make_rotation_x(pitch);
            }
            break;
        }

        case AnimationState::Phase::Landing:
        {
            // Gradually level out for landing
            float phaseProgress = state.getPhaseProgress();

            if (phaseProgress < AnimationState::kLandingHoverFraction)
            {
                // Still tilted during hover
                if (length(state.velocity) > 0.001f)
                {
                    Vec3f forwardDir = normalize(state.velocity);
                    float yaw = atan2(forwardDir.x, forwardDir.z);
                    float currentPitch = -asin(forwardDir.y);
                    float pitch = currentPitch * (1.0f - phaseProgress * 2.0f);
                    rotation = make_rotation_y(yaw) * make_rotation_x(pitch);
                }
            }
            else
            {
                // Upright for final descent
                rotation = kIdentity44f;
            }
            break;
        }
    }

    return rotation;
}
// */


#endif // ANIMATION_STATE_HPP