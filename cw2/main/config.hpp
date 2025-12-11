#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include <numbers>

namespace Config
{
    // Math constants
    constexpr float kFloatPi = std::numbers::pi_v<float>;
    constexpr Vec3f kZeroVec3 = {0.f, 0.f, 0.f};
    constexpr Vec3f kWorldUp = {0.f, 1.f, 0.f};
    constexpr Vec3f kWorldForward = {0.f, 0.f, -1.f};
    constexpr Vec3f kWorldRight = {1.f, 0.f, 0.f};

    namespace Camera
    {
        // Initial camera state
        constexpr Vec3f kInitialPosition = {0.f, 0.f, 10.f};
        constexpr float kInitialYaw = -90.f * (kFloatPi / 180.f); // in radians, looking along -Z
        constexpr float kInitialPitch = 0.f;

        // Movement
        constexpr float kBaseSpeed = 5.f;            // Units per second
        constexpr float kSpeedFastMultiplier = 5.f;  // Shift modifier
        constexpr float kSpeedSlowMultiplier = 0.2f; // Ctrl modifier

        // Rotation sensitivity
        constexpr float kSensitivity = 0.001f;
        constexpr float kDeadZone = 0.0005f;

        // Constraints
        constexpr float kMinPitch = -kFloatPi / 2.1f; // ~-89 degrees
        constexpr float kMaxPitch = kFloatPi / 2.1f;  // ~89 degrees
    }

    namespace World
    {
        // Boundaries
        constexpr float kBorder = 99.9f;
        constexpr float kSeaLevel = -0.968f;
        constexpr float kMinCameraHeight = kSeaLevel + 0.5f;
        constexpr float kMaxCameraHeight = 50.f;

        // Landing pad positions
        constexpr Vec3f kLandingPad1Pos = {-72.28f, kSeaLevel, 12.27f};
        constexpr Vec3f kLandingPad2Pos = {19.0f, kSeaLevel, 14.25f};
        constexpr float kLandingPadScale = 2.0f;
        constexpr Vec3f kLandingPadOffset = {0.f, 1.0f, 0.f}; // Vehicle sits on pad
    }

    namespace Rendering
    {
        // Projection
        constexpr float kNearPlane = 0.1f;
        constexpr float kFarPlane = 100.f;
        constexpr float kFOV = kFloatPi / 4.f; // 45 degrees

        // Lighting
        constexpr Vec3f kLightDir = {0.f, 1.f, -1.f};
        constexpr Vec3f kLightDiffuse = {0.9f, 0.9f, 0.6f};
        constexpr Vec3f kSceneAmbient = {0.05f, 0.05f, 0.05f};
        constexpr Vec3f kClearColor = {0.2f, 0.2f, 0.2f};
    }
}

#endif // CONFIG_HPP