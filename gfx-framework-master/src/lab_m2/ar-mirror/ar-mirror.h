#pragma once

#include "components/simple_scene.h"
#include "components/transform.h"

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
        void CreateFramebuffer(int width, int height);
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        unsigned int UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string &neg_x, const std::string &neg_y, const std::string &neg_z);

        void UpdateObjectPositions(float deltaTimeSeconds);

        void OnKeyPress(int key, int mods) override;
        void OnInputUpdate(float deltaTime, int mods) override;

     private:
        int cubeMapTextureID = 0;
        float archer_angle;
        unsigned int framebuffer_object;
        unsigned int color_texture;
        unsigned int depth_texture;
        unsigned int draw_outlines;

        glm::vec3 mirror_position;
        float mirror_angle_OX;
        float mirror_angle_OY;
        float mirror_angle_OZ;
        float mirror_translate_x = 0;
        float mirror_translate_y = 0;
        float mirror_translate_z = 0;
        float mirror_rotate_OX = 0;
        float mirror_rotate_OY = 0;
        float mirror_rotate_OZ = 0;
        float translate_step = 5;
        float rotate_step = 30;
    };
} 
