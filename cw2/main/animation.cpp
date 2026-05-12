#include "animation.hpp"
#include <print>
#include <algorithm>

// Constructors
Animation::Animation(const Vec3f &start, const Vec3f &end)
    : startPosition(start), endPosition(end), currentPosition(start), previousPosition(start)
{
    precomputeTrajectory();
}

Animation::Animation()
    : startPosition(Config::World::kLandingPad1Pos + kLandingPadOffset),
      endPosition(Config::World::kLandingPad2Pos + kLandingPadOffset),
      currentPosition(Config::World::kLandingPad1Pos + kLandingPadOffset),
      previousPosition(Config::World::kLandingPad1Pos + kLandingPadOffset)
{
    precomputeTrajectory();
}

// Implementation methods
void Animation::precomputeTrajectory()
{
    Vec3f horizontal = endPosition - startPosition;
    horizontal.y = 0.f;

    horizontalDistance = length(horizontal);
    targetDirection =
        horizontalDistance > 0.001f
            ? horizontal / horizontalDistance
            : Vec3f{0.f, 0.f, 1.f};

    // Heights
    adjustedVerticalHeight = kVerticalAscentHeight;
    adjustedCruiseHeight   = kCruiseHeight;
    adjustedHoverHeight    = kHoverHeight;

    // Arc start (top of vertical ascent)
    bezierP0 = {
        startPosition.x,
        startPosition.y + adjustedVerticalHeight,
        startPosition.z
    };

    // Arc end (above landing pad)
    bezierP3 = {
        endPosition.x,
        endPosition.y + adjustedHoverHeight,
        endPosition.z
    };

    // Control points define the arch
    float arcHeight = adjustedCruiseHeight;

    bezierP1 = bezierP0 + targetDirection * (horizontalDistance * 0.25f)
                         + Vec3f{0.f, arcHeight, 0.f};

    bezierP2 = bezierP3 - targetDirection * (horizontalDistance * 0.25f)
                         + Vec3f{0.f, arcHeight, 0.f};
}


float Animation::getNormalizedTime() const noexcept
{
    return animationTime / kTotalAnimationTime;
}

Animation::Phase Animation::getCurrentPhase(float normalizedT) const noexcept
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

float Animation::getPhaseProgress() const noexcept
{
    float normalizedT = getNormalizedTime();
    using enum Phase;
    switch (phase)
    {
    case VerticalAscent:
        return normalizedT / kVerticalAscentEnd;
    case LaunchTilt:
        return (normalizedT - kVerticalAscentEnd) / (kLaunchTiltEnd - kVerticalAscentEnd);
    case Cruise:
        return (normalizedT - kLaunchTiltEnd) / (kCruisePhaseEnd - kLaunchTiltEnd);
    case Landing:
        return (normalizedT - kCruisePhaseEnd) / (1.0f - kCruisePhaseEnd);
    default:
        return 0.0f;
    }
}

// Easing functions
float Animation::easeInQuad(float t) noexcept { return t * t; }
float Animation::easeOutQuad(float t) noexcept { return 1.0f - (1.0f - t) * (1.0f - t); }
float Animation::easeInOutQuad(float t) noexcept
{
    return t < 0.5f ? 2.0f * t * t : 1.0f - (-2.0f * t + 2.0f) * (-2.0f * t + 2.0f) * 0.5f;
}
Vec3f Animation::calculatePosition(float t) const noexcept
{
    float nt = t / kTotalAnimationTime;

    // 1) Vertical ascent
    if (nt < kVerticalAscentEnd)
    {
        float u = nt / kVerticalAscentEnd;

        // Stronger slow-start acceleration
        u = u * u * u * u; // quartic ease-in (slower start, faster end)

        return {
            startPosition.x,
            startPosition.y + adjustedVerticalHeight * u,
            startPosition.z
        };
    }


    // 2) Curved arc
    else if (nt < kArcEnd)
    {
        float u = (nt - kVerticalAscentEnd) / (kArcEnd - kVerticalAscentEnd);
        u = easeInOutQuad(u);

        return calculate_bezier_position(u, bezierP0, bezierP1, bezierP2, bezierP3);
    }

    // 3) Landing (UNCHANGED behaviour)
    else
    {
        float u = (nt - kArcEnd) / (1.f - kArcEnd);
        u = easeInOutQuad(u);

        return {
            endPosition.x,
            endPosition.y + adjustedHoverHeight * (1.f - u),
            endPosition.z
        };
    }

}

// Speed calculation
float Animation::calculateSpeed(float t) const noexcept
{
    float normalizedT = t / kTotalAnimationTime;
    if (normalizedT < kVerticalAscentEnd)
    {
        float phaseT = normalizedT / kVerticalAscentEnd;
        return kMaxVelocity * 0.1f * easeInQuad(phaseT); // Max 10% speed
    }
    else if (normalizedT < kLaunchTiltEnd)
    {
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
            float hoverT = phaseT / kLandingHoverFraction;
            return kMaxVelocity * 0.3f * (1.0f - easeOutQuad(hoverT));
        }
        else
        {
            // Final descent: very slow or stopped
            return 0.0f;
        }
    }
    return 0.0f;

}

// Update animation
void Animation::update(float dt) noexcept
{
    if (!isAnimating || isPaused)
        return;

    animationTime += dt;
    previousPosition = currentPosition;

    // Calculate new position and speed
    currentPosition = calculatePosition(animationTime);
    currentSpeed = calculateSpeed(animationTime);

    if (dt > 0.001f)
    {
        velocity = (currentPosition - previousPosition) / dt;
    }

    // Update phase
    float normalizedT = getNormalizedTime();
    phase = getCurrentPhase(normalizedT);

    // Track max heigh
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

    // Clamp to min height
    if (currentHeight < startPosition.y)
    {
        currentPosition.y = startPosition.y;
    }
}

// State management
void Animation::reset()
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

void Animation::start()
{
    isAnimating = true;
    isPaused = false;
    animationTime = 0.0f;
    currentSpeed = 0.0f;
    currentPosition = startPosition;
    previousPosition = startPosition;
    phase = Phase::VerticalAscent;
    maxHeightReached = 0.0f;
}

void Animation::togglePause()
{
    isPaused = !isPaused;
}

Mat44f calculate_rocket_rotation(const Animation &state) noexcept
{
    Mat44f rotation = kIdentity44f;

    // Cache orientation for continuity across phases
    static float cachedYaw   = 0.f;
    static float cachedPitch = 0.f;

    using enum Animation::Phase;
    switch (state.phase)
    {
        case VerticalAscent:
        {
            // Mimic idle rotation during vertical ascent
            float idleRotation = state.animationTime * 0.5f;
            rotation = make_rotation_y(idleRotation);

            // Reset cached orientation for next phases
            cachedYaw = idleRotation;
            cachedPitch = 0.f;
            break;
        }

        case LaunchTilt:
        {
            float phaseProgress = state.getPhaseProgress();

            float targetYaw = atan2(state.targetDirection.x,
                                    state.targetDirection.z);

            cachedYaw   = targetYaw * phaseProgress;
            cachedPitch = Animation::kMaxTiltAngle * phaseProgress;

            rotation = make_rotation_y(cachedYaw)
                     * make_rotation_x(cachedPitch);
            break;
        }

        case Cruise:
        {
            // Start cruise with EXACT last LaunchTilt orientation
            if (length(state.velocity) > 0.001f)
            {
                Vec3f f = normalize(state.velocity);
                float velYaw   = atan2(f.x, f.z);
                float velPitch = -asin(f.y);

                // Gentle convergence toward velocity direction
                float blend = 0.05f;
                cachedYaw   = cachedYaw   * (1.f - blend) + velYaw   * blend;
                cachedPitch = cachedPitch * (1.f - blend) + velPitch * blend;
            }

            rotation = make_rotation_y(cachedYaw) * make_rotation_x(cachedPitch);
            break;
        }

        case Landing:
        {
            float phaseProgress = state.getPhaseProgress();

            float yaw   = cachedYaw;
            float pitch = cachedPitch;

            // Smoothly rotate back to vertical (pitch -> 0)
            pitch *= (1.f - Animation::easeInOutQuad(phaseProgress));

            cachedPitch = pitch;

            rotation = make_rotation_y(yaw) * make_rotation_x(pitch);
            break;
        }
    }

    return rotation;
}
