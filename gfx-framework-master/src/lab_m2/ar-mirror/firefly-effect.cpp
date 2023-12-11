#include "lab_m2/ar-mirror/firefly-effect.h"

using namespace std;
using namespace m2;


FireflyEffect::FireflyEffect(unsigned int nrParticles, float radius, glm::vec3 generator_position)
{
    this->nrParticles = nrParticles;
    this->generator_position = generator_position;
    ResetParticles(radius);

    // Define the 5 bezier curves that the particles will follow
    glm::vec3 point_1_ref = glm::vec3(0, 0, 0);
    glm::vec3 point_2_ref = glm::vec3(120, 105, 0);
    glm::vec3 point_3_ref = glm::vec3(250, 50, 0);
    glm::vec3 point_4_ref = glm::vec3(350, 180, 0);
    glm::vec3 points_ref[4] = { point_1_ref, point_2_ref, point_3_ref, point_4_ref };

    float angle = RADIANS(360 / 5);

    for (int curve = 0; curve < 5; ++curve)
    {
        glm::mat4 curveRotation = glm::mat4(1);
        curveRotation *= glm::rotate(glm::mat4(1), angle * curve, glm::vec3(0, 1, 0));

        for (int i = 0; i < 4; ++i)
        {
            glm::vec4 point = curveRotation * glm::vec4(points_ref[i], 1);
            bezier_points[curve][i] = point;
        }
    }
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