#pragma once

#include <string>

#include "components/simple_scene.h"
#include "core/gpu/frame_buffer.h"


namespace m2
{
    class Watermark : public gfxc::SimpleScene
    {
     public:
         Watermark();
        ~Watermark();

        void Init() override;

     private:
        void FrameStart() override;
        void Update(float deltaTimeSeconds) override;
        void FrameEnd() override;

        void OnKeyPress(int key, int mods) override;

        void OpenDialog();
        void OnFileSelected(const std::string &fileName);

        // Processing effects
        inline char CharToGrayScale(char red, char green, char blue) { return static_cast<char>(red * 0.2f + green * 0.71f + blue * 0.07); }
        void GrayScale();
        void Sobel();

        void SaveImage(const std::string &fileName);

     private:
        unsigned char sobelThreshold = 20;
        bool showSobel = false;

        Texture2D* originalImage;
        Texture2D* grayscaleImage;
        Texture2D* sobelImage;

    };
}   // namespace m2
