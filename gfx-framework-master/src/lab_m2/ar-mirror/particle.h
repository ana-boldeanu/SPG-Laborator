#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"

#include <string>

namespace m2
{
    class Particle
    {
    public:
        Particle();
        Particle(const glm::vec4& position, const glm::vec4& speed);
        ~Particle();

        void SetInitial(const glm::vec4& position, const glm::vec4& speed, float delay = 0, float lifetime = 0);

        glm::vec4 position;
        glm::vec4 initialPosition;
        glm::vec4 speed;
        glm::vec4 initialSpeed;
        float delay;
        float initialDelay;
        float lifetime;
        float initialLifetime;
    };
}
