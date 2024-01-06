#pragma once

#include <string>

#include "components/simple_scene.h"
#include "core/gpu/frame_buffer.h"


namespace m2
{
    class Watermark_OpenMP : public gfxc::SimpleScene
    {
     public:
         Watermark_OpenMP();
        ~Watermark_OpenMP();

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
        void GrayScale(bool onWatermark = false);
        void Sobel(bool onWatermark = false);

        void ApplySobelOnWatermark();
        void ApplySobelOnLoadedImage();

        int ProcessWatermarkWhiteAmount();
        void FindWatermarks();
        void RemoveWatermarks();

        void SaveImage(const std::string &fileName);

     private:
        double watermarkMinimumOverlapThreshold = 0.5;
        double minimumThresholdOtherwiseSkipImageArea = 0.095;
        int watermarkMinimumWhiteAmount = 0;
        int minimumMatchesOtherwiseSkipImageArea = 0;
        int showImageMode = 1; // 1 = original, 2 = grayscale, 3 = sobel
        unsigned char sobelThreshold = 25;
        bool showWatermark = false;

        std::vector<glm::vec2> matches;

        Texture2D* originalImage;
        Texture2D* grayscaleImage;
        Texture2D* sobelImage;
        Texture2D* finalImage;

        Texture2D* originalWatermark;
        Texture2D* grayscaleWatermark;
        Texture2D* filteredColorWatermark;
        Texture2D* sobelWatermark;
    };
}   // namespace m2
