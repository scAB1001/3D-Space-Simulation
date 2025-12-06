#include "camera.hpp"
#include <print>
#include <numbers>

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

void Camera::rotate(float yawOffset, float pitchOffset) noexcept
{
    if (mode != Mode::Free)
        return;

    yaw += yawOffset;
    pitch += pitchOffset;

    normalizeYaw();
    applyPitchConstraints();
    updateVectors();
}

void Camera::clampToWorldBounds() noexcept
{
    if (mode != Mode::Free)
        return;

    position.x = std::clamp(position.x, -Config::World::kBorder, Config::World::kBorder);
    position.z = std::clamp(position.z, -Config::World::kBorder, Config::World::kBorder);
}

void Camera::clampVertical(float minY, float maxY) noexcept
{
    if (mode != Mode::Free)
        return;

    position.y = std::clamp(position.y, minY, maxY);
}

void Camera::updateForAnimation(const Vec3f &vehiclePos, const Vec3f &vehicleVelocity, float dt) noexcept
{
    switch (mode)
    {
        case Mode::Follow:
        {
            // Get vehicle forward direction (use velocity if moving, otherwise default)
            Vec3f vehicleForward;
            if (length(vehicleVelocity) > 0.1f)
            {
                vehicleForward = normalize(vehicleVelocity);
            }
            else
            {
                // Default forward (towards target or world -Z)
                vehicleForward = Vec3f{0.f, 0.f, -1.f};
            }

            // Calculate vehicle right and up vectors
            Vec3f vehicleRight = normalize(cross(vehicleForward, worldUp));
            Vec3f vehicleUp = normalize(cross(vehicleRight, vehicleForward));

            // Determine which side to be on
            float sideMultiplier = followSettings.useRightSide ? 1.0f : -1.0f;

            // Calculate camera position relative to vehicle
            Vec3f offset =
                vehicleRight * (followSettings.sideOffset.x * sideMultiplier) +
                vehicleUp * followSettings.sideOffset.y +
                vehicleForward * followSettings.sideOffset.z;

            // Apply fixed distance
            offset = normalize(offset) * followSettings.distance;

            Vec3f desiredPosition = vehiclePos + offset;

            // Smooth interpolation
            float followSpeed = 3.0f * dt;
            position = position * (1.0f - followSpeed) + desiredPosition * followSpeed;

            // Always look at vehicle
            forward = normalize(vehiclePos - position);
            updateVectors();

            break;
        }

        case Mode::FixedGround:
        {
            // Fixed position on ground
            position = fixedGroundSettings.position;

            // Always look at vehicle
            forward = normalize(vehiclePos - position);

            // Calculate yaw and pitch from forward vector
            yaw = atan2(forward.x, forward.z);
            pitch = asin(forward.y);

            applyPitchConstraints();
            normalizeYaw();
            updateVectors();

            break;
        }

        case Mode::Free:
        default:
            // User controls camera - nothing to do here
            break;
    }
}

void Camera::cycleMode() noexcept
{
    switch (mode)
    {
        case Mode::Free:
            mode = Mode::Follow;
            std::print("Camera mode: FOLLOW (tracking vehicle)\n");
            break;
        case Mode::Follow:
            mode = Mode::FixedGround;
            std::print("Camera mode: FIXED GROUND\n");
            break;
        case Mode::FixedGround:
            mode = Mode::Free;
            std::print("Camera mode: FREE (user control)\n");
            break;
    }
}

void Camera::setupFollowMode(const Vec3f &vehiclePos, const Vec3f &vehicleForward)
{
    if (mode != Mode::Follow)
        return;

    // Initialize camera position for follow mode
    Vec3f vehicleRight = normalize(cross(vehicleForward, worldUp));
    Vec3f vehicleUp = normalize(cross(vehicleRight, vehicleForward));

    float sideMultiplier = followSettings.useRightSide ? 1.0f : -1.0f;

    Vec3f offset =
        vehicleRight * (followSettings.sideOffset.x * sideMultiplier) +
        vehicleUp * followSettings.sideOffset.y +
        vehicleForward * followSettings.sideOffset.z;

    offset = normalize(offset) * followSettings.distance;

    position = vehiclePos + offset;
    forward = normalize(vehiclePos - position);
    updateVectors();
}

void Camera::setupFixedGroundMode(const Vec3f &vehiclePos)
{
    if (mode != Mode::FixedGround)
        return;

    // Set fixed ground position
    position = fixedGroundSettings.position;

    // Look at vehicle
    forward = normalize(vehiclePos - position);

    // Calculate yaw and pitch from forward vector
    yaw = atan2(forward.x, forward.z);
    pitch = asin(forward.y);

    applyPitchConstraints();
    normalizeYaw();
    updateVectors();
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