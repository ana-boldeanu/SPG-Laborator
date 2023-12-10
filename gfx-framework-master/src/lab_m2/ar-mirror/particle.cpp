#include "lab_m2/ar-mirror/particle.h"

using namespace std;
using namespace m2;


Particle::Particle()
{
    SetInitial(glm::vec4(0), glm::vec4(0));
}


Particle::Particle(const glm::vec4& position, const glm::vec4& speed)
{
    SetInitial(position, speed);
}


Particle::~Particle()
{
}


void Particle::SetInitial(const glm::vec4& position, const glm::vec4& speed, float delay, float lifetime)
{
    this->position = position;
    initialPosition = position;

    this->speed = speed;
    initialSpeed = speed;

    this->delay = delay;
    initialDelay = delay;

    this->lifetime = lifetime;
    initialLifetime = lifetime;
}