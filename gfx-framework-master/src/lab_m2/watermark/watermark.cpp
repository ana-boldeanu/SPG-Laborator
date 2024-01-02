#include "lab_m2/watermark/watermark.h"

#include <vector>
#include <iostream>

#include "pfd/portable-file-dialogs.h"

using namespace std;
using namespace m2;


/*
 *  To find out more about `FrameStart`, `Update`, `FrameEnd`
 *  and the order in which they are called, see `world.cpp`.
 */


Watermark::Watermark()
{
    window->SetSize(600, 600);
}


Watermark::~Watermark()
{
}


void Watermark::Init()
{
    // Load default texture for imagine processing
    originalImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "star.png"), nullptr, "image", true, true);
    grayscaleImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "star.png"), nullptr, "grayscaleImage", true, true);
    sobelImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "star.png"), nullptr, "sobelImage", true, true);

    // Load watermark image
    originalWatermark = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "watermark.png"), nullptr, "watermark", true, true);
    grayscaleWatermark = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "watermark.png"), nullptr, "grayscaleWatermark", true, true);
    sobelWatermark = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "watermark.png"), nullptr, "sobelWatermark", true, true);

    {
        Mesh* mesh = new Mesh("quad");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "shaders");

    // Create a shader program for particle system
    {
        Shader *shader = new Shader("ImageProcessing");
        shader->AddShader(PATH_JOIN(shaderPath, "VertexShader.glsl"), GL_VERTEX_SHADER);
        shader->AddShader(PATH_JOIN(shaderPath, "FragmentShader.glsl"), GL_FRAGMENT_SHADER);

        shader->CreateAndLink();
        shaders[shader->GetName()] = shader;
    }

    // Process watermark image
    ApplySobelOnWatermark();
    int whiteAmount = ProcessWatermarkWhiteAmount();
    watermarkMinimumWhiteAmount = static_cast<int>(floor(whiteAmount * watermarkMinimumOverlapThreshold));
}


void Watermark::FrameStart()
{
}


void Watermark::Update(float deltaTimeSeconds)
{
    ClearScreen();

    auto shader = shaders["ImageProcessing"];
    shader->Use();

    glUniform1i(shader->GetUniformLocation("textureImage"), 0);

    Texture2D* textureImage;
    switch (showImageMode) {
    case 2:
        textureImage = grayscaleImage;
        break;
    case 3:
        textureImage = sobelImage;
        break;
    case 1:
    default:
        textureImage = originalImage;
    }

    if (showWatermark) {
        textureImage = sobelWatermark;
    }

    textureImage->BindToTextureUnit(GL_TEXTURE0);

    RenderMesh(meshes["quad"], shader, glm::mat4(1));
}


void Watermark::FrameEnd()
{
    DrawCoordinateSystem();
}


void Watermark::ApplySobelOnWatermark()
{
    GrayScale(true);
    Sobel(true);
}


void Watermark::ApplySobelOnLoadedImage()
{
    GrayScale();
    Sobel();
}

int Watermark::ProcessWatermarkWhiteAmount()
{
    unsigned char* watermarkData = sobelWatermark->GetImageData();
    glm::ivec2 watermarkSize = glm::ivec2(sobelWatermark->GetWidth(), sobelWatermark->GetHeight());
    int whiteAmount = 0;

    for (int i = 0; i < watermarkSize.y; ++i)
    {
        for (int j = 0; j < watermarkSize.x; ++j)
        {
            if (watermarkData[i * watermarkSize.x + j] == 255)
                whiteAmount++;
        }
    }

    std::cout << "whiteAmount = " << whiteAmount << "\n";

    return whiteAmount;
}


void Watermark::FindWatermarks()
{
    unsigned char* imageData = sobelImage->GetImageData();
    unsigned char* watermarkData = sobelWatermark->GetImageData();
    unsigned int imageChannels = originalImage->GetNrChannels();
    unsigned int watermarkChannels = originalWatermark->GetNrChannels();
    glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
    glm::ivec2 watermarkSize = glm::ivec2(originalWatermark->GetWidth(), originalWatermark->GetHeight());
    int minimumMatches = watermarkMinimumWhiteAmount;
    int pixelMatches = 0;
    int y, x, m, n, imageOffset, watermarkOffset;

    std::cout << "imageSize = [x = " << imageSize.x << ", y = " << imageSize.y << "]\n";
    std::cout << "watermarkSize = [x = " << watermarkSize.x << ", y = " << watermarkSize.y << "]\n";
    std::cout << "maximumMatches = " << watermarkSize.x * watermarkSize.y << "\n";
    std::cout << "minimumMatches = " << minimumMatches << "\n";
    std::cout << "Started search.\n";

    // use offset + channels, cuz maybe watermark is differently mapped
    if (imageChannels < 3 || watermarkChannels < 3)
        return;

    for (y = 0; y < imageSize.y; ++y)
    {
        for (x = 0; x < imageSize.x; ++x)
        {
            for (m = 0; m < watermarkSize.y; ++m)
            {
                for (n = 0; n < watermarkSize.x; ++n)
                {
                    imageOffset = imageChannels * (y * imageSize.x + x);
                    watermarkOffset = watermarkChannels * (m * watermarkSize.x + n);

                    if (imageData[imageOffset] == 255 && watermarkData[watermarkOffset] == 255) {
                        ++pixelMatches;
                    }
                }
            }

            // std::cout << pixelMatches << "\n"; // always 4997

            if (pixelMatches >= minimumMatches) {
                std::cout << "Found match at [x = " << x << " y = " << y << "]\n";

                // Go to the next row
                x += watermarkSize.x;

                if (x >= imageSize.x) {
                    x = 0;
                    y += watermarkSize.y;
                }

                if (y >= imageSize.y)
                {
                    std::cout << "Finished search.\n";
                    return;
                }
            }

            pixelMatches = 0;
        }
    }

    std::cout << "Finished search.\n";
}


void Watermark::GrayScale(bool onWatermark)
{
    unsigned int channels;
    unsigned char *data, *newData;
    glm::ivec2 imageSize;

    if (onWatermark) {
        channels = originalWatermark->GetNrChannels();
        data = originalWatermark->GetImageData();
        newData = grayscaleWatermark->GetImageData();
        imageSize = glm::ivec2(originalWatermark->GetWidth(), originalWatermark->GetHeight());
    }
    else {
        channels = originalImage->GetNrChannels();
        data = originalImage->GetImageData();
        newData = grayscaleImage->GetImageData();
        imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());
    }

    if (channels < 3)
        return;


    for (int i = 0; i < imageSize.y; ++i)
    {
        for (int j = 0; j < imageSize.x; ++j)
        {
            int offset = channels * (i * imageSize.x + j);

            // Reset save image data
            char value = CharToGrayScale(data[offset + 0], data[offset + 1], data[offset + 2]);
            memset(&newData[offset], value, 3);
        }
    }

    if (onWatermark) {
        grayscaleWatermark->UploadNewData(newData);
    }
    else {
        grayscaleImage->UploadNewData(newData);
    }
}


void Watermark::Sobel(bool onWatermark)
{
    unsigned int channels;
    unsigned char *data, *newData;
    unsigned char neighbours[9] = { 0 };
    unsigned char dx, dy, d;
    int i, j, m, n, idx, offset;
    glm::ivec2 imageSize;

    if (onWatermark)
    {
        channels = grayscaleWatermark->GetNrChannels();
        data = grayscaleWatermark->GetImageData();
        newData = sobelWatermark->GetImageData();
        imageSize = glm::ivec2(grayscaleWatermark->GetWidth(), grayscaleWatermark->GetHeight());
    }
    else {
        channels = grayscaleImage->GetNrChannels();
        data = grayscaleImage->GetImageData();
        newData = sobelImage->GetImageData();
        imageSize = glm::ivec2(grayscaleImage->GetWidth(), grayscaleImage->GetHeight());
    }

    if (channels < 3)
        return;

    for (i = 0; i < imageSize.y; ++i)
    {
        for (j = 0; j < imageSize.x; ++j)
        {
            offset = channels * (i * imageSize.x + j);

            // Get neighbours grayscale values
            idx = 0;
            for (m = i - 1; m <= i + 1; ++m)
            {
                for (n = j - 1; n <= j + 1; ++n)
                {
                    // Treat image bounds
                    if (m == -1 || n == -1 || m == imageSize.y || n == imageSize.x)
                        neighbours[idx] = 0;
                    else
                        neighbours[idx] = data[channels * (m * imageSize.x + n)];
                    ++idx;
                }
            }
            
            // Apply sobel filter
            dx = static_cast<unsigned char>(abs(-neighbours[0] + neighbours[2] - 2 * neighbours[3] + 2 * neighbours[5] - neighbours[6] + neighbours[8]));
            dy = static_cast<unsigned char>(abs(-neighbours[0] - 2 * neighbours[1] - neighbours[2] + neighbours[6] + 2 * neighbours[7] + neighbours[8]));
            d = static_cast<unsigned char>(floor(sqrt(pow(dx, 2) + pow(dy, 2))));

            if (d > sobelThreshold)
                d = 255;
            else
                d = 0;
            
            memset(&newData[offset], d, 3);
        }
    }

    if (onWatermark) {
        sobelWatermark->UploadNewData(newData);
    }
    else {
        sobelImage->UploadNewData(newData);
    }
}


void Watermark::OnFileSelected(const std::string& fileName)
{
    if (fileName.size())
    {
        std::cout << fileName << endl;
        originalImage = TextureManager::LoadTexture(fileName, nullptr, "image", true, true);
        grayscaleImage = TextureManager::LoadTexture(fileName, nullptr, "grayscaleImage", true, true);
        sobelImage = TextureManager::LoadTexture(fileName, nullptr, "sobelImage", true, true);

        float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
        window->SetSize(static_cast<int>(600 * aspectRatio), 600);
    }
}


void Watermark::SaveImage(const std::string &fileName)
{
    cout << "Saving image! ";
    grayscaleImage->SaveToFile((fileName + ".png").c_str());
    cout << "[Done]" << endl;
}


void Watermark::OpenDialog()
{
    std::vector<std::string> filters =
    {
        "Image Files", "*.png *.jpg *.jpeg *.bmp",
        "All Files", "*"
    };

    auto selection = pfd::open_file("Select a file", ".", filters).result();
    if (!selection.empty())
    {
        std::cout << "User selected file " << selection[0] << "\n";
        OnFileSelected(selection[0]);
    }
}


void Watermark::OnKeyPress(int key, int mods)
{
    // Add key press event
    if (key == GLFW_KEY_S)
    {
        ApplySobelOnLoadedImage();
        showImageMode = 3;
        FindWatermarks();
    }

    if (key == GLFW_KEY_W)
    {
        showWatermark = !showWatermark;

        if (showWatermark) {
            float aspectRatio = static_cast<float>(originalWatermark->GetWidth()) / originalWatermark->GetHeight();
            window->SetSize(static_cast<int>(600 * aspectRatio), 600);
        }
        else {
            float aspectRatio = static_cast<float>(originalImage->GetWidth()) / originalImage->GetHeight();
            window->SetSize(static_cast<int>(600 * aspectRatio), 600);
        }
    }

    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
    }

    if (key == GLFW_KEY_1)
    {
        showImageMode = 1;
    }

    if (key == GLFW_KEY_2)
    {
        showImageMode = 2;
    }

    if (key == GLFW_KEY_3)
    {
        showImageMode = 3;
    }

    if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
    {
        SaveImage("processCPU_" + std::to_string(0));
    }
}
