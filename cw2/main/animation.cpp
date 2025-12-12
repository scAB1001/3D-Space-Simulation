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

    // Calculate vertical ascent end position
    verticalAscentEndPos = Vec3f{
        startPosition.x,
        startPosition.y + adjustedVerticalHeight,
        startPosition.z};

    // Calculate launch end position
    float tiltPhaseProgress = 0.8f;
    launchEndPosition = Vec3f{
        startPosition.x + (endPosition.x - startPosition.x) * tiltPhaseProgress,
        startPosition.y + adjustedLaunchHeight,
        startPosition.z + (endPosition.z - startPosition.z) * tiltPhaseProgress};

    // Precompute Bezier control points
    bezierP0 = launchEndPosition;
    bezierP1 = bezierP0 + targetDirection * kBezierControlOffset +
               Vec3f{0.f, kBezierControlOffset * 0.8f, 0.f};
    bezierP2 = endPosition + Vec3f{0.f, adjustedCruiseHeight, 0.f} -
               targetDirection * kBezierControlOffset * 0.7f;
    bezierP3 = endPosition + Vec3f{0.f, adjustedHoverHeight, 0.f};
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

// Position calculation
Vec3f Animation::calculatePosition(float t) const noexcept
{
    float normalizedT = t / kTotalAnimationTime;

    // Phase 1: VERTICAL ASCENT (0% - 20%)
    if (normalizedT < kVerticalAscentEnd)
    {
        float phaseT = normalizedT / kVerticalAscentEnd;
        float heightProgress = easeInQuad(phaseT);
        return Vec3f{
            startPosition.x,
            startPosition.y + adjustedVerticalHeight * heightProgress,
            startPosition.z};
    }

    // Phase 2: LAUNCH TILT (20% - 40%)
    else if (normalizedT < kLaunchTiltEnd)
    {
        float phaseT = (normalizedT - kVerticalAscentEnd) / (kLaunchTiltEnd - kVerticalAscentEnd);
        float targetHeight = adjustedVerticalHeight + (adjustedLaunchHeight - adjustedVerticalHeight) * phaseT;
        float horizontalEase = phaseT * phaseT * phaseT;
        float horizontalProgress = 0.8f * horizontalEase;
        return Vec3f{
            startPosition.x + (endPosition.x - startPosition.x) * horizontalProgress,
            startPosition.y + targetHeight,
            startPosition.z + (endPosition.z - startPosition.z) * horizontalProgress};
    }

    // Phase 3: CRUISE (40% - 70%)
    else if (normalizedT < kCruisePhaseEnd)
    {
        float phaseT = (normalizedT - kLaunchTiltEnd) / (kCruisePhaseEnd - kLaunchTiltEnd);
        float easedT = easeInOutQuad(phaseT);
        return calculate_bezier_position(easedT, bezierP0, bezierP1, bezierP2, bezierP3);
    }

    // Phase 4: LANDING (70% - 100%)
    else
    {
        float phaseT = (normalizedT - kCruisePhaseEnd) / (1.0f - kCruisePhaseEnd);
        if (phaseT < kLandingHoverFraction)
        {
            float hoverT = phaseT / kLandingHoverFraction;
            float height = adjustedHoverHeight * (1.0f - easeOutQuad(hoverT) * 0.7f);
            return Vec3f{endPosition.x, endPosition.y + height, endPosition.z};
        }
        else
        {
            float descentT = (phaseT - kLandingHoverFraction) / (1.0f - kLandingHoverFraction);
            float height = adjustedHoverHeight * 0.3f * (1.0f - easeInQuad(descentT));
            return Vec3f{endPosition.x, endPosition.y + height, endPosition.z};
        }
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
            // Slow hover descent
            float hoverT = phaseT / kLandingHoverFraction;
            return kMaxVelocity * 0.3f * (1.0f - easeOutQuad(hoverT)); // Start at 30%, slow to 0
        }
        else
        {
            // Very slow final descent
            float descentT = (phaseT - kLandingHoverFraction) / (1.0f - kLandingHoverFraction);
            return kMaxVelocity * 0.1f * (1.0f - easeInQuad(descentT));
        }
    }
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

// Rocket rotation function
Mat44f calculate_rocket_rotation(const Animation &state) noexcept
{
    Mat44f rotation = kIdentity44f;

    using enum Animation::Phase;
    switch (state.phase)
    {
    case VerticalAscent:
    {
        float idleRotation = state.animationTime * 0.5f;
        rotation = make_rotation_y(idleRotation);
        break;
    }

    case LaunchTilt:
    {
        float phaseProgress = state.getPhaseProgress();
        float targetYaw = atan2(state.targetDirection.x, state.targetDirection.z);
        float currentYaw = targetYaw * phaseProgress;
        float tiltAngle = Animation::kMaxTiltAngle * phaseProgress;
        rotation = make_rotation_y(currentYaw) * make_rotation_x(tiltAngle);
        break;
    }

    case Cruise:
    {
        if (length(state.velocity) > 0.001f)
        {
            Vec3f forwardDir = normalize(state.velocity);
            float yaw = atan2(forwardDir.x, forwardDir.z);
            float pitch = -asin(forwardDir.y);

            pitch = std::clamp(pitch, -Animation::kMaxPitchAngle, Animation::kMaxPitchAngle);
            rotation = make_rotation_y(yaw) * make_rotation_x(pitch);
        }
        break;
    }

    case Landing:
    {
        float phaseProgress = state.getPhaseProgress();
        if (phaseProgress < Animation::kLandingHoverFraction)
        {
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
            rotation = kIdentity44f;
        }
        break;
    }
    }

    return rotation;
}