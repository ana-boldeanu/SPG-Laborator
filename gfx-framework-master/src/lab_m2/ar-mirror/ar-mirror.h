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

        void OnKeyPress(int key, int mods) override;

     private:
        int cubeMapTextureID = 0;
        float archer_angle;
        unsigned int framebuffer_object;
        unsigned int color_texture;
        unsigned int depth_texture;
        unsigned int draw_outlines;

    };
}   // namespace m2
