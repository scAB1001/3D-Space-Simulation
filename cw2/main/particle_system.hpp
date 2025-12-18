#pragma once
#include <vector>
#include <glad/glad.h>

#include "../vmlib/vec3.hpp"
#include "../vmlib/mat44.hpp"
#include "camera.hpp"

struct Particle {
    Vec3f position;
    Vec3f velocity;
    float life = 0.f;
    float maxLife = 0.f;
    bool alive = false;
};

class ParticleSystem {
public:
    ParticleSystem(size_t maxCount = 1000000);

    void init();
    void cleanup();
    void setShader(GLuint shaderId);

    void emit(const Vec3f &origin, const Vec3f &dir);
    void update(float dt);

    void render(const Mat44f &proj, const Mat44f &view, const Camera &cam);

private:
    std::vector<Particle> particles;
    size_t maxCount;

    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint texture = 0;

    GLuint shader = 0;
    GLint uProj = -1;
    GLint uView = -1;
    GLint uCamRight = -1;
    GLint uCamUp = -1;
    GLint uParticlePos = -1;
    GLint uAlpha = -1;
};
