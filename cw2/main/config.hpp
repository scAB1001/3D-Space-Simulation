#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include <numbers>

namespace Config
{
    constexpr float kFloatPi = std::numbers::pi_v<float>;
    constexpr Vec3f kInitialCameraPos = {0.f, 0.f, 10.f};
    constexpr float kInitialCameraYaw = -90.f * (kFloatPi / 180.f); // in radians
    constexpr float kCameraBaseSpeed = 5.f;      // Units per second
    constexpr float kCameraSensitivity = 0.001f; // TODO: Adjust as needed
    constexpr float kNearPlane = 0.1f;
    constexpr float kFarPlane = 100.f;
    constexpr float kFOV = kFloatPi / 4.f;
    constexpr float kWorldBorder = 99.9f;
    constexpr Vec3f kZeroVec3 = {0.f, 0.f, 0.f};

    // Landing pad positions
    constexpr float kSeaLevel = -0.968f;
    constexpr Vec3f kLandingPad1Pos = {-72.28f, kSeaLevel, 12.27f};
    constexpr Vec3f kLandingPad2Pos = {19.0f, kSeaLevel, 14.25f};
    constexpr float kLandingPadScale = 2.0f;

    // NdotL Lighting
    constexpr Vec3f kLightDir = {0.f, 1.f, -1.f};
    constexpr Vec3f kLightDiffuse = {0.9f, 0.9f, 0.6f};
    constexpr Vec3f kSceneAmbient = {0.05f, 0.05f, 0.05f};

    /* TODO: Potential view setup for fixed camera mode
        - position: (-12.102316, 23.208414, 75.28221)
        - yaw: -1.7148036 radians, pitch: -0.23699781 radians
        - yaw: -98.25 degrees, pitch: -13.58 degrees

        TODO: Flight follows phases by seconds, not actual state of animation.
        FIX.
    */
}

#endif // CONFIG_HPP