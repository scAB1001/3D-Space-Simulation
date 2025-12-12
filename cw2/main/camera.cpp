#include "camera.hpp"

Camera::Camera()
    : position(Config::Camera::kInitialPosition), yaw(Config::Camera::kInitialYaw), pitch(Config::Camera::kInitialPitch), speed(Config::Camera::kBaseSpeed), baseSpeed(Config::Camera::kBaseSpeed), mode(Mode::Free)
{
    updateVectors();
}

Camera::Camera(Vec3f position, float yaw, float pitch)
    : position(position), yaw(yaw), pitch(pitch), speed(Config::Camera::kBaseSpeed), baseSpeed(Config::Camera::kBaseSpeed), mode(Mode::Free)
{
    updateVectors();
}

void Camera::updateVectors() noexcept
{
    // Calculate new forward vector from yaw and pitch
    forward.x = std::cos(yaw) * std::cos(pitch);
    forward.y = std::sin(pitch);
    forward.z = std::sin(yaw) * std::cos(pitch);
    forward = normalize(forward);

    // Recalculate right and up vectors
    right = normalize(cross(forward, worldUp));
    up = normalize(cross(right, forward));
}

Mat44f Camera::getViewMatrix() const noexcept
{
    return make_look_at(position, position + forward, up);
}

bool Camera::isMode(Mode mode) noexcept
{
    return mode == getMode();
}

void Camera::setPosition(Vec3f newPosition) noexcept
{
    position = newPosition;
}

void Camera::setYaw(float newYaw) noexcept
{
    yaw = newYaw;
    normalizeYaw();
    updateVectors();
}

void Camera::setPitch(float newPitch) noexcept
{
    pitch = newPitch;
    applyPitchConstraints();
    updateVectors();
}

void Camera::setMode(Mode newMode) noexcept
{
    mode = newMode;
}

void Camera::setSpeed(float newSpeed) noexcept
{
    speed = newSpeed;
}

void Camera::setBaseSpeed(float newBaseSpeed) noexcept
{
    baseSpeed = newBaseSpeed;
    speed = newBaseSpeed; // Reset current speed to base
}

void Camera::moveForward(float dt) noexcept
{
    if (mode != Mode::Free)
        return;
    position += forward * speed * dt;
}

void Camera::moveBackward(float dt) noexcept
{
    if (mode != Mode::Free)
        return;
    position -= forward * speed * dt;
}

void Camera::moveLeft(float dt) noexcept
{
    if (mode != Mode::Free)
        return;
    position -= right * speed * dt;
}

void Camera::moveRight(float dt) noexcept
{
    if (mode != Mode::Free)
        return;
    position += right * speed * dt;
}

void Camera::moveUp(float dt) noexcept
{
    if (mode != Mode::Free)
        return;
    position += up * speed * dt;
}

void Camera::moveDown(float dt) noexcept
{
    if (mode != Mode::Free)
        return;
    position -= up * speed * dt;
}

void Camera::updateSpeed(Camera &cam, const Input &input) noexcept
{
    // Calculate speed multiplier based on current modifier state
    float speedMultiplier = 1.0f;

    if (input.shiftPressed)
    {
        speedMultiplier = Config::Camera::kSpeedFastMultiplier;
    }
    else if (input.controlPressed)
    {
        speedMultiplier = Config::Camera::kSpeedSlowMultiplier;
    }

    // Update relevant camera
    cam.setSpeed(Config::Camera::kBaseSpeed * speedMultiplier);
}

void Camera::rotate(float yawOffset, float pitchOffset) noexcept
{
    if (mode != Mode::Free && mode != Mode::Follow)
        return;

    yaw += yawOffset;
    pitch += pitchOffset;

    normalizeYaw();
    applyPitchConstraints();
    updateVectors();
}

void Camera::updateForAnimation(const Vec3f &vehiclePos, const Vec3f &vehicleVelocity, float dt) noexcept
{
    switch (mode)
    {
        using enum Mode;
        case Follow:
        {
            updateFollowMode(vehiclePos, vehicleVelocity, dt);
            break;
        }

        case Fixed:
        {
            position = fixedSettings.position;
            lookAtTarget(vehiclePos);
            break;
        }

        case Free:
        default:
            break;
    }
}

void Camera::cycleMode() noexcept
{
    switch (mode)
    {
        using enum Mode;
        case Free:
            mode = Follow;
            break;
        case Follow:
            mode = Fixed;
            break;
        case Fixed:
            mode = Free;
            break;
    }
}

void Camera::initFollowMode(const Vec3f &vehiclePos, const Vec3f &vehicleForward)
{
    if (mode != Mode::Follow)
        return;

    // Initialize camera position for follow mode
    Vec3f flightDir = normalize(Vec3f{1.f, 0.f, 0.02f});  // Mostly east
    Vec3f sideDir = normalize(cross(flightDir, worldUp)); // Perpendicular
    Vec3f upDir = Vec3f{0.f, 1.f, 0.f};

    // Position: 25 units to right, 8 units up, 5 units behind
    Vec3f offset =
        sideDir * 25.0f +  // Right side
        upDir * 8.0f +     // Above
        -flightDir * 5.0f; // Slightly behind

    position = vehiclePos + offset;

    lookAtTarget(vehiclePos);
}

void Camera::initFixedMode(const Vec3f &vehiclePos)
{
    if (mode != Mode::Fixed)
        return;

    // Set fixed ground position
    position = fixedSettings.position;
    lookAtTarget(vehiclePos);
}

void Camera::lookAtTarget(const Vec3f &targetPosition) noexcept
{
    // Direct forward vector calculation
    Vec3f newForward = targetPosition - position;

    if (length(newForward) > 0.001f)
    {
        newForward = normalize(newForward);

        // Calculate right vector
        Vec3f newRight = normalize(cross(newForward, worldUp));
        if (length(newRight) < 0.001f)
        {
            // Handle edge case when looking straight up/down
            newRight = Vec3f{1.0f, 0.0f, 0.0f};
        }

        // Calculate up vector
        Vec3f newUp = normalize(cross(newRight, newForward));

        // Set vectors directly
        forward = newForward;
        right = newRight;
        up = newUp;

        // Extract yaw and pitch from forward vector
        yaw = std::atan2(forward.z, forward.x);
        pitch = std::asin(forward.y);
    }
}

void Camera::updateFollowMode(const Vec3f &vehiclePos, const Vec3f &vehicleVelocity, float dt)
{
    if (mode != Mode::Follow)
        return;

    //Calculate flight path direction (horizontal)
    static Vec3f flightPathDir = normalize(Vec3f{1.f, 0.f, 0.f}); // Default east
    if (length(vehicleVelocity) > 0.1f)
    {
        Vec3f horizontalVel = vehicleVelocity;
        horizontalVel.y = 0.f;
        if (length(horizontalVel) > 0.01f)
        {
            flightPathDir = normalize(horizontalVel);
        }
    }

    // Camera position
    Vec3f perpendicularDir = normalize(cross(flightPathDir, worldUp));
    if (length(perpendicularDir) < 0.001f)
        perpendicularDir = Vec3f{0.f, 0.f, 1.f}; // Fallback

    // Determine which side (right/left)
    float sideMultiplier = followSettings.useRightSide ? 1.0f : -1.0f;

    // Calculate position
    Vec3f desiredOffset =
        perpendicularDir * (25.0f * sideMultiplier) + // Side distance
        Vec3f{0.f, 10.0f, 0.f} +                      // Height
        -flightPathDir * 5.0f;                        // Slightly behind

    Vec3f desiredPosition = vehiclePos + desiredOffset;

    // Smooth movement
    float smoothFactor = std::min(followSettings.smoothness * dt * 2.0f, 1.0f);
    position = mix(position, desiredPosition, smoothFactor);
}

void Camera::offsetPositionFromTarget(const Vec3f &targetPosition, Vec3f offset) noexcept
{
    position = targetPosition + offset;
}

void Camera::resetToInitial() noexcept
{
    position = Config::Camera::kInitialPosition;
    yaw = Config::Camera::kInitialYaw;
    pitch = Config::Camera::kInitialPitch;
    speed = baseSpeed;
    mode = Mode::Free;
    updateVectors();
}

void Camera::applyPitchConstraints() noexcept
{
    constexpr float maxPitch = Config::kFloatPi / 2.1f; // ~89 degrees
    pitch = std::clamp(pitch, -maxPitch, maxPitch);
}

void Camera::normalizeYaw() noexcept
{
    constexpr float twoPi = 2.0f * Config::kFloatPi;

    // Normalize yaw to the range [-pi, pi] for numerical stability
    if (yaw > Config::kFloatPi)
    {
        yaw -= twoPi;
    }
    else if (yaw < -Config::kFloatPi)
    {
        yaw += twoPi;
    }
}

void processMovement(Camera &cam, const Input &input, float dt) noexcept
{
    if (!cam.isMode(Camera::Mode::Free))
        return;

    if (input.moveForward)
        cam.moveForward(dt);
    if (input.moveBackward)
        cam.moveBackward(dt);
    if (input.moveLeft)
        cam.moveLeft(dt);
    if (input.moveRight)
        cam.moveRight(dt);
    if (input.moveUp)
        cam.moveUp(dt);
    if (input.moveDown)
        cam.moveDown(dt);
}
