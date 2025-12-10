#ifndef STATE_HPP
#define STATE_HPP

#include "../vmlib/vec3.hpp"
#include "camera.hpp"
#include "input_state.hpp"
#include "animation_state.hpp"
#include "../support/program.hpp"


struct PointLight {
    Vec3f position;
    Vec3f color;
    bool enabled;
};

struct State_ {
    ShaderProgram* prog = nullptr;    // <-- REQUIRED

    Camera camera;
    InputState input;
    AnimationState animation;

    // Task 1.6 Lighting
    bool dirLightEnabled = true;
    PointLight pointLights[3] = {
        { Vec3f{0,0,0}, Vec3f{1.f,0.2f,0.2f}, true },
        { Vec3f{0,0,0}, Vec3f{0.2f,1.f,0.2f}, true },
        { Vec3f{0,0,0}, Vec3f{0.2f,0.2f,1.f}, true }
    };
};


#endif
