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
    // Load default texture fore imagine processing
    originalImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "star.png"), nullptr, "image", true, true);
    grayscaleImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "star.png"), nullptr, "grayscaleImage", true, true);
    sobelImage = TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "watermark", "test_images", "star.png"), nullptr, "sobelImage", true, true);

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

    auto textureImage = showSobel ? sobelImage : grayscaleImage;
    textureImage->BindToTextureUnit(GL_TEXTURE0);

    RenderMesh(meshes["quad"], shader, glm::mat4(1));
}


void Watermark::FrameEnd()
{
    DrawCoordinateSystem();
}


void Watermark::OnFileSelected(const std::string &fileName)
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


void Watermark::GrayScale()
{
    unsigned int channels = originalImage->GetNrChannels();
    unsigned char* data = originalImage->GetImageData();
    unsigned char* newData = grayscaleImage->GetImageData();

    if (channels < 3)
        return;

    glm::ivec2 imageSize = glm::ivec2(originalImage->GetWidth(), originalImage->GetHeight());

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

    grayscaleImage->UploadNewData(newData);
}


void Watermark::Sobel()
{
    unsigned int channels = grayscaleImage->GetNrChannels();
    unsigned char* data = grayscaleImage->GetImageData();
    unsigned char* newData = sobelImage->GetImageData();
    unsigned char neighbours[9] = { 0 };
    unsigned char dx, dy, d;
    int i, j, m, n, idx, offset;

    if (channels < 3)
        return;

    glm::ivec2 imageSize = glm::ivec2(grayscaleImage->GetWidth(), grayscaleImage->GetHeight());

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

    sobelImage->UploadNewData(newData);
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
    if (key == GLFW_KEY_F || key == GLFW_KEY_ENTER || key == GLFW_KEY_SPACE)
    {
        OpenDialog();
    }

    if (key == GLFW_KEY_1)
    {
        GrayScale();
        Sobel();
        showSobel = true;
    }

    if (key == GLFW_KEY_S && mods & GLFW_MOD_CONTROL)
    {
        SaveImage("processCPU_" + std::to_string(0));
    }
}
