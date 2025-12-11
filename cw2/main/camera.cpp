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

void Camera::updateSpeed(Camera &cam, const InputState &input) noexcept
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

    // Update all relevant cameras
    // float baseSpeed = ;
    cam.setSpeed(Config::Camera::kBaseSpeed * speedMultiplier);
    // state->leftCamera.setSpeed(baseSpeed * speedMultiplier);
    // state->rightCamera.setSpeed(baseSpeed * speedMultiplier);
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
            updateFollowMode(vehiclePos, vehicleVelocity, dt);
            break;
        }

        case Mode::Fixed:
        {
            position = fixedSettings.position;
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
            mode = Mode::Fixed;
            std::print("Camera mode: FIXED GROUND\n");
            break;
        case Mode::Fixed:
            mode = Mode::Free;
            std::print("Camera mode: FREE (user control)\n");
            break;
    }
}

void Camera::initFollowMode(const Vec3f &vehiclePos, const Vec3f &vehicleForward)
{
    if (mode != Mode::Follow)
        return;

    // Initialize camera position for follow mode
    std::print("Initializing FOLLOW camera mode.\n");
    // Hardcoded for your specific flight path
    Vec3f flightDir = normalize(Vec3f{1.f, 0.f, 0.02f});  // Mostly east
    Vec3f sideDir = normalize(cross(flightDir, worldUp)); // Perpendicular
    Vec3f upDir = Vec3f{0.f, 1.f, 0.f};

    // Position: 25 units to right, 8 units up, 5 units behind
    Vec3f offset =
        sideDir * 25.0f +  // Right side
        upDir * 8.0f +     // Above
        -flightDir * 5.0f; // Slightly behind

    position = vehiclePos + offset;

    // Look at vehicle
    lookAtTarget(vehiclePos);

    std::print("Follow camera: Side-on view along flight path\n");
}

void Camera::initFixedMode(const Vec3f &vehiclePos)
{
    if (mode != Mode::Fixed)
        return;

    // Set fixed ground position
    position = fixedSettings.position;
    lookAtTarget(vehiclePos);
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
        // std::print("=== lookAtTarget Debug ===\n");
        // std::print("Direct vector method:\n");
        // std::print("  Forward: ({:.3f}, {:.3f}, {:.3f})\n",
        //             forward.x, forward.y, forward.z);
        // std::print("  Yaw: {:.3f} rad\n", yaw);
        // std::print("  Pitch: {:.3f} rad\n", pitch);
    }
}

void Camera::debugOut(const Vec3f &vehiclePos, const Vec3f &vehicleVelocity)
{
    std::print("=== Camera State ===\n");
    std::print("Camera position = ({:.2f}, {:.2f}, {:.2f})\n",
               position.x, position.y, position.z);
    std::print("  Vehicle position: ({:.2f}, {:.2f}, {:.2f})\n",
               vehiclePos.x, vehiclePos.y, vehiclePos.z);
    std::print("VehicleVelocity=({:.2f},{:.2f},{:.2f}), length={:.2f}\n",
               vehicleVelocity.x, vehicleVelocity.y, vehicleVelocity.z,
               length(vehicleVelocity));
    std::print("Forward: ({:.3f}, {:.3f}, {:.3f})\n",
               forward.x, forward.y, forward.z);
    std::print("Up:      ({:.3f}, {:.3f}, {:.3f})\n",
               up.x, up.y, up.z);
    std::print("Right:   ({:.3f}, {:.3f}, {:.3f})\n",
               right.x, right.y, right.z);
    std::print("Yaw:   {:.3f} rad ({:.1f}°)\n",
               yaw, yaw * (180.0f / Config::kFloatPi));
    std::print("Pitch: {:.3f} rad ({:.1f}°)\n",
               pitch, pitch * (180.0f / Config::kFloatPi));
    std::print("Mode: {}\n",
               (mode == Mode::Free ? "Free" :
                mode == Mode::Follow ? "Follow" :
                mode == Mode::Fixed ? "Fixed" : "Unknown"));
}

void Camera::updateFollowMode(const Vec3f &vehiclePos, const Vec3f &vehicleVelocity, float dt)
{
    if (mode != Mode::Follow)
        return;

    // 1. Calculate flight path direction (horizontal)
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

    // 2. Camera position: PERPENDICULAR to flight path (true side-on)
    Vec3f perpendicularDir = normalize(cross(flightPathDir, worldUp));
    if (length(perpendicularDir) < 0.001f)
        perpendicularDir = Vec3f{0.f, 0.f, 1.f}; // Fallback

    // Determine which side (right/left)
    float sideMultiplier = followSettings.useRightSide ? 1.0f : -1.0f;

    // 3. Calculate position:
    // - Perpendicular offset for side view
    // - Slightly behind along flight path
    // - Above vehicle
    Vec3f desiredOffset =
        perpendicularDir * (25.0f * sideMultiplier) + // Side distance
        Vec3f{0.f, 10.0f, 0.f} +                      // Height
        -flightPathDir * 5.0f;                        // Slightly behind

    Vec3f desiredPosition = vehiclePos + desiredOffset;

    // 4. Smooth movement
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

void processMovement(Camera &cam, const InputState &input, float dt) noexcept
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
