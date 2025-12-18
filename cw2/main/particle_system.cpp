#include "particle_system.hpp"

ParticleSystem::ParticleSystem(size_t n)
    : maxCount(n)
{
    particles.resize(maxCount);
}

void ParticleSystem::setShader(GLuint shaderId)
{
    shader = shaderId;

    uProj        = glGetUniformLocation(shader, "uProj");
    uView        = glGetUniformLocation(shader, "uView");
    uCamRight    = glGetUniformLocation(shader, "uCameraRight");
    uCamUp       = glGetUniformLocation(shader, "uCameraUp");
    uParticlePos = glGetUniformLocation(shader, "uParticlePos");
    uAlpha       = glGetUniformLocation(shader, "uAlpha");
}

void ParticleSystem::init()
{
    float quad[] = {
        -1.f, -1.f,
         1.f, -1.f,
        -1.f,  1.f,
         1.f,  1.f
    };

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);

    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quad), quad, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float), (void*)0);

    glBindVertexArray(0);

    texture = load_texture_2d("assets/cw2/particle.png");
}

void ParticleSystem::cleanup()
{
    if (vbo) glDeleteBuffers(1, &vbo);
    if (vao) glDeleteVertexArrays(1, &vao);
    if (texture) glDeleteTextures(1, &texture);

    vbo = vao = texture = 0;
}

void ParticleSystem::emit(const Vec3f &origin, const Vec3f &dir)
{
    for (auto &p : particles)
    {
        if (!p.alive)
        {
            p.position = origin;
            p.velocity = dir * 4.f;

            p.velocity.x += (float(rand())/RAND_MAX - 0.5f) * 1.f;
            p.velocity.z += (float(rand())/RAND_MAX - 0.5f) * 1.f;

            p.life = p.maxLife = 0.6f;
            p.alive = true;
            return;
        }
    }
}

void ParticleSystem::update(float dt)
{
    for (auto &p : particles)
    {
        if (!p.alive) continue;

        p.position += p.velocity * dt;
        p.life -= dt;

        if (p.life <= 0.f)
            p.alive = false;
    }
}

void ParticleSystem::render(const Mat44f &proj, const Mat44f &view, const Camera &cam)
{
    /* References for code inspiration:
     * - https://wikis.khronos.org/opengl/Primitive#Point_primitives
     * - https://www.kenney.nl/assets/particle-pack
     */

    glUseProgram(shader);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);

    glBindVertexArray(vao);
    glBindTexture(GL_TEXTURE_2D, texture);

    glUniformMatrix4fv(uProj, 1, GL_TRUE, proj.v);
    glUniformMatrix4fv(uView, 1, GL_TRUE, view.v);

    Vec3f right = cam.getRight();
    Vec3f up    = cam.getUp();

    glUniform3fv(uCamRight, 1, &right.x);
    glUniform3fv(uCamUp,    1, &up.x);

    for (auto &p : particles)
    {
        if (!p.alive) continue;

        float alpha = p.life / p.maxLife;
        glUniform1f(uAlpha, alpha);

        glUniform3fv(uParticlePos, 1, &p.position.x);

        glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
    }

    glDisable(GL_BLEND);
}
