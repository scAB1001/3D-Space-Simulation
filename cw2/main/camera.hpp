#ifndef CAMERA_HPP
#define CAMERA_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "config.hpp"

class Camera
{
public:
    // Camera modes
    enum class Mode
    {
        Free,       // User-controlled
        Follow,     // Fixed distance following vehicle
        FixedGround // Fixed position on ground
    };

    // Constructors
    Camera();
    Camera(Vec3f position, float yaw, float pitch);

    // Getters
    Vec3f getPosition() const noexcept { return position; }
    Vec3f getForward() const noexcept { return forward; }
    Vec3f getRight() const noexcept { return right; }
    Vec3f getUp() const noexcept { return up; }
    float getYaw() const noexcept { return yaw; }
    float getPitch() const noexcept { return pitch; }
    Mode getMode() const noexcept { return mode; }
    float getSpeed() const noexcept { return speed; }

    // Setters
    void setPosition(Vec3f newPosition) noexcept;
    void setYaw(float newYaw) noexcept;
    void setPitch(float newPitch) noexcept;
    void setMode(Mode newMode) noexcept;
    void setSpeed(float newSpeed) noexcept;
    void setBaseSpeed(float newBaseSpeed) noexcept;

    bool isMode(Mode mode) noexcept;

    // Camera control
    void updateVectors() noexcept;
    Mat44f getViewMatrix() const noexcept;
    void updateForAnimation(const Vec3f &vehiclePos, const Vec3f &vehicleVelocity, float dt) noexcept;

    void cycleMode() noexcept;

    // Special mode initializations
    void initFollowMode(const Vec3f &vehiclePos, const Vec3f &vehicleForward);
    void initFixedGroundMode(const Vec3f &vehiclePos);

    // Movement
    void moveForward(float dt) noexcept;
    void moveBackward(float dt) noexcept;
    void moveLeft(float dt) noexcept;
    void moveRight(float dt) noexcept;
    void moveUp(float dt) noexcept;
    void moveDown(float dt) noexcept;

    // Rotation
    void rotate(float yawOffset, float pitchOffset) noexcept;

    // Position clamping
    void clampToWorldBounds() noexcept;
    void clampVertical(float minY, float maxY) noexcept;

    // State management
    void debugOrientation(const Vec3f &targetPosition) const noexcept;
    void lookAtTarget(const Vec3f &targetPosition) noexcept;
    void offsetPositionFromTarget(const Vec3f &targetPosition, Vec3f offset) noexcept;
    void resetToInitial() noexcept;

private:
    // Camera state
    Vec3f position;
    Vec3f forward;
    Vec3f right;
    Vec3f up;
    Vec3f worldUp = {0.f, 1.f, 0.f};

    // Orientation
    float yaw;
    float pitch;

    // Movement
    float speed;
    float baseSpeed;

    // Mode
    Mode mode;

    // Mode-specific settings
    struct FollowSettings
    {
        float distance = 30.0f;
        Vec3f sideOffset = {20.f, 5.f, 0.f}; // Parallel to side: (right, up, forward)
        bool useRightSide = true;            // Switch between left/right side

    } followSettings;

    struct FixedGroundSettings
    {
        // yaw and pitch are calculated dynamically
        Vec3f position = {-33.05f, 15.90f, -32.0f};
    } fixedGroundSettings;

    // Helper methods
    void applyPitchConstraints() noexcept;
    void normalizeYaw() noexcept;
};

#endif // CAMERA_HPP -72.28,12.27 19.0,14.25
