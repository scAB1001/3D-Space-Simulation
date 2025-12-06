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
    position += forward * speed * dt;
}

void Camera::moveBackward(float dt) noexcept
{
    position -= forward * speed * dt;
}

void Camera::moveLeft(float dt) noexcept
{
    position -= right * speed * dt;
}

void Camera::moveRight(float dt) noexcept
{
    position += right * speed * dt;
}

void Camera::moveUp(float dt) noexcept
{
    position += up * speed * dt;
}

void Camera::moveDown(float dt) noexcept
{
    position -= up * speed * dt;
}

void Camera::rotate(float yawOffset, float pitchOffset) noexcept
{
    yaw += yawOffset;
    pitch += pitchOffset;

    normalizeYaw();
    applyPitchConstraints();
    updateVectors();
}

void Camera::clampToWorldBounds() noexcept
{
    position.x = std::clamp(position.x, -Config::World::kBorder, Config::World::kBorder);
    position.z = std::clamp(position.z, -Config::World::kBorder, Config::World::kBorder);
}

void Camera::clampVertical(float minY, float maxY) noexcept
{
    position.y = std::clamp(position.y, minY, maxY);
}

void Camera::updateForAnimation(const Vec3f &vehiclePos, const Vec3f &vehicleVelocity, float dt) noexcept
{
    switch (mode)
    {
    case Mode::Follow:
    {
        Vec3f desiredPosition;

        if (length(vehicleVelocity) > 0.1f)
        {
            // Follow from behind based on velocity direction
            Vec3f backDir = normalize(vehicleVelocity) * -followSettings.distance;
            desiredPosition = vehiclePos + backDir + Vec3f{0.f, followSettings.distance * 0.3f, 0.f};
        }
        else
        {
            // Default offset if not moving much
            desiredPosition = vehiclePos + followSettings.offset;
        }

        // Smooth interpolation to desired position
        float followSpeed = 2.0f * dt;
        position = position * (1.0f - followSpeed) + desiredPosition * followSpeed;

        // Look at vehicle (slightly ahead during movement)
        Vec3f lookTarget = vehiclePos;
        if (length(vehicleVelocity) > 0.1f)
        {
            lookTarget = lookTarget + normalize(vehicleVelocity) * 5.0f;
        }

        forward = normalize(lookTarget - position);
        updateVectors();
        break;
    }

    case Mode::FixedGround:
    {
        position = fixedGroundSettings.position;
        yaw = fixedGroundSettings.yaw;
        pitch = fixedGroundSettings.pitch;
        forward = normalize(vehiclePos - position);
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

void Camera::followMode(const Vec3f& curPos, const Vec3f& backDir) noexcept
{
    // Position camera behind vehicle
    position = curPos + backDir * followSettings.distance + followSettings.offset;
    updateVectors();
}

void Camera::fixedGroundMode() noexcept
{
    position = fixedGroundSettings.position;
    yaw = fixedGroundSettings.yaw;
    pitch = fixedGroundSettings.pitch;
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