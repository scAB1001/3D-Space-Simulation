#ifndef CONFIG_HPP
#define CONFIG_HPP

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include <numbers>

namespace Config
{
    constexpr float kFloatPi = std::numbers::pi_v<float>;
    constexpr Vec3f kInitialCameraPos = {0.f, 0.f, 10.f};
    constexpr float kInitialCameraYaw = -90.f * (kFloatPi / 180.f);
    constexpr float kCameraBaseSpeed = 5.f;      // Units per second
    constexpr float kCameraSensitivity = 0.001f; // TODO: Adjust as needed
    constexpr float kNearPlane = 0.1f;
    constexpr float kFarPlane = 100.f;
    constexpr float kFOV = kFloatPi / 4.f;
    constexpr float kWorldBorder = 99.9f;

    // Landing pad positions
    constexpr float kSeaLevel = -0.968f;
    constexpr Vec3f kLandingPad1Pos = {-72.28f, kSeaLevel, 12.27f};
    constexpr Vec3f kLandingPad2Pos = {67.92f, kSeaLevel, -6.35f};
    constexpr float kLandingPadScale = 2.0f;

    // NdotL Lighting
    constexpr Vec3f kLightDir = {0.f, 1.f, -1.f};
    constexpr Vec3f kLightDiffuse = {0.9f, 0.9f, 0.6f};
    constexpr Vec3f kSceneAmbient = {0.05f, 0.05f, 0.05f};
}

#endif // CONFIG_HPP