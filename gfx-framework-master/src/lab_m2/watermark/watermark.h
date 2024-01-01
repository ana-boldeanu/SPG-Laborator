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
        void GrayScale();
        void SaveImage(const std::string &fileName);

     private:
        Texture2D *originalImage;
        Texture2D *processedImage;
    };
}   // namespace m2
