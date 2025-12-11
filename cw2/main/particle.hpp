#pragma once
#include "../vmlib/vec3.hpp"

struct Particle {
    Vec3f position;
    Vec3f velocity;
    float life;
    float maxLife;
    bool alive = false;
};
