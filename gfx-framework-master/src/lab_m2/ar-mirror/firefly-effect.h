#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"
#include "core/gpu/particle_effect.h"
#include "particle.h"

namespace m2
{
    class FireflyEffect
    {
    public:
        FireflyEffect(unsigned int nrParticles, float radius, glm::vec3 generator_position);
        ~FireflyEffect();

        void ResetParticles(float radius);

        ParticleEffect<Particle>* particleEffect;
        unsigned int nrParticles;
        glm::vec3 generator_position;
        glm::vec3 bezier_points[5][4] = {glm::vec3(0)};  // 5 Bezier lines, each line defined by 4 control points
    };
}
