#include "lab_m2/ar-mirror/firefly-effect.h"

using namespace std;
using namespace m2;


FireflyEffect::FireflyEffect(unsigned int nrParticles, float radius, glm::vec3 generator_position)
{
    this->nrParticles = nrParticles;
    this->generator_position = generator_position;
    ResetParticles(radius);
}


FireflyEffect::~FireflyEffect()
{
}

void FireflyEffect::ResetParticles(float radius)
{
    particleEffect = new ParticleEffect<Particle>();
    particleEffect->Generate(nrParticles, true);

    auto particleSSBO = particleEffect->GetParticleBuffer();
    Particle* data = const_cast<Particle*>(particleSSBO->GetBuffer());

    for (unsigned int i = 0; i < nrParticles; i++)
    {
        glm::vec3 pos(1);
        pos.x = (rand() % 100 - 50) / 100.0f;
        pos.y = (rand() % 100 - 50) / 100.0f;
        pos.z = (rand() % 100 - 50) / 100.0f;
        pos = glm::normalize(pos) * radius;

        glm::vec3 speed(0);
        speed = glm::normalize(glm::vec3(0, 5, 0) - glm::vec3(pos));
        speed *= (rand() % 100 / 100.0f);
        speed += glm::vec3(rand() % 5 / 5.0f, rand() % 5 / 5.0f, rand() % 5 / 5.0f) * 0.2f;

        float lifetime = 1 + (rand() % 100 / 100.0f);

        data[i].SetInitial(glm::vec4(pos, 1), glm::vec4(speed, 0), 0, lifetime);
    }

    particleSSBO->SetBufferData(data);
}