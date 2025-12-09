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
                // If vehicle isn't moving, forward should be from camera to vehicle
                vehicleForward = normalize(vehiclePos - position);
            }

            // Calculate vehicle right and up vectors
            Vec3f vehicleRight = normalize(cross(vehicleForward, worldUp));
            if (length(vehicleRight) < 0.001f)
            {
                vehicleRight = Vec3f{1.0f, 0.0f, 0.0f};
            }

            Vec3f vehicleUp = normalize(cross(vehicleRight, vehicleForward));

            // Determine which side to be on
            float sideMultiplier = followSettings.useRightSide ? 1.0f : -1.0f;

            // Calculate camera position relative to vehicle
            Vec3f offset =
                vehicleRight * (followSettings.sideOffset.x * sideMultiplier) +
                vehicleUp * followSettings.sideOffset.y -
                vehicleForward * followSettings.sideOffset.z; // Negative = behind

            // Apply fixed distance
            if (length(offset) > 0.001f)
            {
                offset = normalize(offset) * followSettings.distance;
            }
            else
            {
                offset = Vec3f{0.f, followSettings.distance * 0.5f, -followSettings.distance};
            }

            Vec3f desiredPosition = vehiclePos + offset;

            // Smooth interpolation
            float followSpeed = 3.0f * dt;
            followSpeed = std::clamp(followSpeed, 0.0f, 1.0f);
            position = position * (1.0f - followSpeed) + desiredPosition * followSpeed;

            lookAtTarget(vehiclePos);
            break;
        }

        case Mode::FixedGround:
        {
            position = fixedGroundSettings.position;
            lookAtTarget(vehiclePos);
            break;
        }

        case Mode::Free:
        default:
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

    yaw = fixedGroundSettings.yaw;
    pitch = fixedGroundSettings.pitch;
    // Look at vehicle
    // forward = normalize(vehiclePos - position);

    // Calculate yaw and pitch from forward vector
    // yaw = atan2(forward.x, forward.z);
    // pitch = asin(forward.y);

    applyPitchConstraints();
    normalizeYaw();
    updateVectors();
}

void Camera::debugOrientation(const Vec3f &targetPosition) const noexcept
{
    Vec3f toTarget = targetPosition - position;
    if (length(toTarget) > 0.001f)
    {
        toTarget = normalize(toTarget);

        std::print("=== Camera Debug ===\n");
        std::print("Camera position: ({:.2f}, {:.2f}, {:.2f})\n",
                   position.x, position.y, position.z);
        std::print("Target position: ({:.2f}, {:.2f}, {:.2f})\n",
                   targetPosition.x, targetPosition.y, targetPosition.z);
        std::print("Direction to target: ({:.3f}, {:.3f}, {:.3f})\n",
                   toTarget.x, toTarget.y, toTarget.z);
        std::print("Camera forward: ({:.3f}, {:.3f}, {:.3f})\n",
                   forward.x, forward.y, forward.z);
        std::print("Camera yaw: {:.3f} rad ({:.1f}°)\n",
                   yaw, yaw * (180.0f / Config::kFloatPi));
        std::print("Camera pitch: {:.3f} rad ({:.1f}°)\n",
                   pitch, pitch * (180.0f / Config::kFloatPi));

        // Calculate what yaw/pitch SHOULD be
        float desiredYaw = std::atan2(toTarget.x, toTarget.z);
        float desiredPitch = std::asin(toTarget.y);

        std::print("Desired yaw: {:.3f} rad ({:.1f}°)\n",
                   desiredYaw, desiredYaw * (180.0f / Config::kFloatPi));
        std::print("Desired pitch: {:.3f} rad ({:.1f}°)\n",
                   desiredPitch, desiredPitch * (180.0f / Config::kFloatPi));

        float dotp = dot(forward, toTarget);
        std::print("Alignment: {:.3f} (1.0 = perfect, 0.0 = 90°, -1.0 = opposite)\n", dotp);

        if (dotp < 0.99f)
        {
            std::print("WARNING: Camera not facing target properly!\n");
        }
    }
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

        // TODO: Remove later. For debugging.
        std::print("=== lookAtTarget Debug ===\n");
        std::print("Direct vector method:\n");
        std::print("  Forward: ({:.3f}, {:.3f}, {:.3f})\n",
                    forward.x, forward.y, forward.z);
        std::print("  Yaw: {:.3f} rad\n", yaw);
        std::print("  Pitch: {:.3f} rad\n", pitch);
    }
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