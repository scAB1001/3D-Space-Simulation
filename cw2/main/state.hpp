#ifndef STATE_HPP
#define STATE_HPP

#include "../vmlib/vec3.hpp"
#include "camera.hpp"
#include "input_state.hpp"
#include "animation_state.hpp"
#include "../support/program.hpp"
#include "particle_system.hpp"


struct PointLight
{
    Vec3f position;
    Vec3f color;
    bool enabled = true;

    void toggle() noexcept
    {
        enabled = !enabled;
    }
};



struct State_
{
    ShaderProgram *prog = nullptr;
    ParticleSystem particles { 1000000 };

    Camera camera;
    InputState input;
    AnimationState animation;

    // === Split Screen ===
    bool splitScreen = false;      // Press V to toggle

    Camera leftCamera;
    Camera rightCamera;

    bool globalDirLightEnabled = true;
    PointLight pointLights[3] = {
        {Vec3f{0, 0, 0}, Vec3f{1.f, 0.2f, 0.2f}, true},
        {Vec3f{0, 0, 0}, Vec3f{0.2f, 1.f, 0.2f}, true},
        {Vec3f{0, 0, 0}, Vec3f{0.2f, 0.2f, 1.f}, true}};

    void toggleGlobalDirLight() noexcept
    {
        globalDirLightEnabled = !globalDirLightEnabled;
    }
};

#endif