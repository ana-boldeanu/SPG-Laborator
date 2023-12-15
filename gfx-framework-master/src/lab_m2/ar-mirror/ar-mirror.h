#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"
#include "firefly-effect.h"

#include <string>


namespace m2
{
    class AR_Mirror : public gfxc::SimpleScene
    {
     public:
         AR_Mirror();
        ~AR_Mirror();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        // Update
        void UpdateObjectPositions(float deltaTimeSeconds);

        // Initializers
        void LoadMeshes();
        void LoadShaders();
        void LoadTextures();

        unsigned int UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string &neg_x, const std::string &neg_y, const std::string &neg_z);
        void CreateFramebuffer(int width, int height);
        void LoadShader(const std::string& name, const std::string& VS, const std::string& FS, const std::string& GS, bool hasGeomtery);

        // Callback functions
        void OnKeyPress(int key, int mods) override;
        void OnInputUpdate(float deltaTime, int mods) override;


     private:
        // Shader variables
        int cubeMapTextureID = 0;
        unsigned int framebuffer_object;
        unsigned int color_texture;
        unsigned int depth_texture;
        unsigned int draw_outlines;

        // Particle effects
        FireflyEffect* fireflyEffect;
        float fireflyEffect_radius = 0.25f;
        unsigned int fireflyEffect_particles = 100;
        unsigned int draw_fireflyEffect = 0;

        // Mirror movement
        glm::vec3 mirror_position;
        float mirror_angle_OX;
        float mirror_angle_OY;
        float mirror_angle_OZ;
        float mirror_translate_x = 0;
        float mirror_translate_y = 0;
        float mirror_translate_z = -7;
        float mirror_rotate_OX = 0;
        float mirror_rotate_OY = 0;
        float mirror_rotate_OZ = 0;
        float translate_step = 3;
        float rotate_step = 30;

        // Other scene elements
        float archer_angle;
    };
} 
