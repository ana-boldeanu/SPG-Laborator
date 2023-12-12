#include "lab_m2/ar-mirror/ar-mirror.h"

#include <vector>
#include <iostream>

#include "stb/stb_image.h"

using namespace std;
using namespace m2;


AR_Mirror::AR_Mirror()
{
    framebuffer_object = 0;
    color_texture = 0;
    depth_texture = 0;
    draw_outlines = 0;

    archer_angle = 0;

    mirror_position = glm::vec3(0);
    mirror_angle_OX = 0;
    mirror_angle_OY = 0;
    mirror_angle_OZ = 0;

    fireflyEffect = new FireflyEffect(fireflyEffect_particles, fireflyEffect_radius, glm::vec3(0));
}


AR_Mirror::~AR_Mirror()
{
}


void AR_Mirror::Init()
{
    auto camera = GetSceneCamera();
    camera->SetPositionAndRotation(glm::vec3(0, -1, 4), glm::quat(glm::vec3(RADIANS(10), 0, 0)));
    camera->Update();

    LoadMeshes();
    LoadShaders();
    LoadTextures();

    // Create the framebuffer on which the scene is rendered from the perspective of the mesh (texture size must be cubic)
    CreateFramebuffer(1024, 1024);
}


void AR_Mirror::FrameStart()
{
}


void AR_Mirror::Update(float deltaTimeSeconds)
{
    UpdateObjectPositions(deltaTimeSeconds);

    auto camera = GetSceneCamera();

    // Draw the scene in Framebuffer
    if (framebuffer_object)
    {
        glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);

        // Set the clear color for the color buffer
        glClearColor(0,0,0, 1);

        // Clears the color buffer (using the previously set color) and depth buffer
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glViewport(0, 0, 1024, 1024);

        Shader *shader = draw_outlines == 1 ? shaders["Framebuffer_Outlines"] : shaders["Framebuffer"];
        shader->Use();

        glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 100.0f);

        {
            glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            glUniform1i(glGetUniformLocation(shader->program, "texture_cubemap"), 1);

            glUniform1i(glGetUniformLocation(shader->program, "draw_cubemap"), 1);

            meshes["cube"]->Render();
        }

        auto camera_forward = camera->m_transform->GetLocalOZVector();
        glUniform3f(shader->GetUniformLocation("camera_forward"), camera_forward.x, camera_forward.y, camera_forward.z);

        glUniform1i(shader->GetUniformLocation("draw_outlines"), draw_outlines);

        for (int i = 0; i < 5; i++)
        {
            glm::mat4 modelMatrix = glm::mat4(1);
            modelMatrix *= glm::rotate(glm::mat4(1), archer_angle + i * glm::radians(360.0f) / 5, glm::vec3(0, 1, 0));
            modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(3, -1, 0));
            modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));
            modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(0.01f));

            glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));

            glm::mat4 cubeView[6] =
            {
                glm::lookAt(mirror_position, mirror_position + glm::vec3(1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)),  // +X
                glm::lookAt(mirror_position, mirror_position + glm::vec3(-1.0f, 0.0f, 0.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // -X
                glm::lookAt(mirror_position, mirror_position + glm::vec3(0.0f, 1.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f)),  // +Y
                glm::lookAt(mirror_position, mirror_position + glm::vec3(0.0f, -1.0f, 0.0f), glm::vec3(0.0f, 0.0f,-1.0f)), // -Y
                glm::lookAt(mirror_position, mirror_position + glm::vec3(0.0f, 0.0f, 1.0f), glm::vec3(0.0f,-1.0f, 0.0f)),  // +Z
                glm::lookAt(mirror_position, mirror_position + glm::vec3(0.0f, 0.0f, -1.0f), glm::vec3(0.0f,-1.0f, 0.0f)), // -Z
            };

            glUniformMatrix4fv(glGetUniformLocation(shader->GetProgramID(), "viewMatrices"), 6, GL_FALSE, glm::value_ptr(cubeView[0]));
            glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(projection));

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("Akai_E_Espiritu.fbm\\akai_diffuse.png")->GetTextureID());
            glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

            glUniform1i(glGetUniformLocation(shader->program, "draw_cubemap"), 0);

            meshes["archer"]->Render();
        }

        glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        // Reset drawing to screen
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    // Clear the screen
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glViewport(0, 0, window->GetResolution().x, window->GetResolution().y);

    // Draw the cubemap
    {
        Shader* shader = shaders["ShaderNormal"];
        shader->Use();

        glm::mat4 modelMatrix = glm::scale(glm::mat4(1), glm::vec3(30));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
        glUniform1i(shader->GetUniformLocation("texture_cubemap"), 0);

        meshes["cube"]->Render();
    }

    // Draw five archers around the mirror
    for (int i = 0; i < 5; i++)
    {
        Shader* shader = shaders["Simple"];
        shader->Use();

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= glm::rotate(glm::mat4(1), archer_angle + i * glm::radians(360.0f) / 5, glm::vec3(0, 1, 0));
        modelMatrix *= glm::translate(glm::mat4(1), glm::vec3(3, -1, 0));
        modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(-90.0f), glm::vec3(0, 1, 0));
        modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(0.01f));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, TextureManager::GetTexture("Akai_E_Espiritu.fbm\\akai_diffuse.png")->GetTextureID());
        glUniform1i(glGetUniformLocation(shader->program, "texture_1"), 0);

        meshes["archer"]->Render();
    }

    // Draw the mirror and the reflection on its surface
    {
        Shader *shader = shaders["CubeMap"];
        shader->Use();

        glm::mat4 modelMatrix = glm::mat4(1);
        modelMatrix *= glm::translate(glm::mat4(1), mirror_position);
        modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(mirror_angle_OX), glm::vec3(1, 0, 0));
        modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(mirror_angle_OY), glm::vec3(0, 1, 0));
        modelMatrix *= glm::rotate(glm::mat4(1), glm::radians(mirror_angle_OZ), glm::vec3(0, 0, 1));
        modelMatrix *= glm::scale(glm::mat4(1), glm::vec3(2.5f));

        glUniformMatrix4fv(shader->loc_model_matrix, 1, GL_FALSE, glm::value_ptr(modelMatrix));
        glUniformMatrix4fv(shader->loc_view_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetViewMatrix()));
        glUniformMatrix4fv(shader->loc_projection_matrix, 1, GL_FALSE, glm::value_ptr(camera->GetProjectionMatrix()));

        if (!color_texture) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, cubeMapTextureID);
            glUniform1i(shader->GetUniformLocation("texture_cubemap"), 0);
        }

        if (color_texture) {
            glActiveTexture(GL_TEXTURE1);
            glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);
            glUniform1i(shader->GetUniformLocation("texture_cubemap"), 1);
        }

        auto cameraPosition = camera->m_transform->GetWorldPosition();
        glUniform3f(shader->GetUniformLocation("camera_position"), cameraPosition.x, cameraPosition.y, cameraPosition.z);

        meshes["mirror"]->Render();
    }

    // Draw the particles effects (should find a way to draw it in cubemap only)
    if (draw_fireflyEffect)
    {
        glLineWidth(3);
        glEnable(GL_BLEND);
        glDisable(GL_DEPTH_TEST);
        glBlendFunc(GL_ONE, GL_ONE);
        glBlendEquation(GL_FUNC_ADD);

        {
            auto shader = shaders["Particle"];
            if (shader->GetProgramID())
            {
                shader->Use();
                TextureManager::GetTexture("particle2.png")->BindToTextureUnit(GL_TEXTURE0);
                fireflyEffect->particleEffect->Render(GetSceneCamera(), shader);

                glm::vec3& generator_position = fireflyEffect->generator_position;
                glUniform3f(glGetUniformLocation(shader->program, "generator_position"), generator_position.x, generator_position.y, generator_position.z);

                glUniform3fv(glGetUniformLocation(shader->program, "bezier_points"), 20, glm::value_ptr(fireflyEffect->bezier_points[0]));

                glUniform1f(glGetUniformLocation(shader->program, "deltaTime"), deltaTimeSeconds);

                glUniform1f(glGetUniformLocation(shader->program, "offset"), 0.2f);
            }
        }

        glEnable(GL_DEPTH_TEST);
        glDisable(GL_BLEND);
    }
}


void AR_Mirror::FrameEnd()
{
    DrawCoordinateSystem();
}


void AR_Mirror::UpdateObjectPositions(float deltaTimeSeconds)
{
    archer_angle += 0.5f * deltaTimeSeconds;
    mirror_position.x = mirror_translate_x;
    mirror_position.y = mirror_translate_y;
    mirror_position.z = mirror_translate_z;
    mirror_angle_OX = mirror_rotate_OX;
    mirror_angle_OY = mirror_rotate_OY;
    mirror_angle_OZ = mirror_rotate_OZ;
}


void AR_Mirror::LoadMeshes()
{
    {
        Mesh* mesh = new Mesh("mirror");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "quad.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("cube");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "primitives"), "box.obj");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }

    {
        Mesh* mesh = new Mesh("archer");
        mesh->LoadMesh(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS, "characters", "archer"), "Archer.fbx");
        mesh->UseMaterials(false);
        meshes[mesh->GetMeshID()] = mesh;
    }
}


void AR_Mirror::LoadShaders()
{
    LoadShader("CubeMap", "CubeMap", "CubeMap", "", false);     // Rendering cubemap texture
    LoadShader("ShaderNormal", "Normal", "Normal", "", false);  // Standard rendering
    LoadShader("Framebuffer", "Framebuffer", "Framebuffer", "Framebuffer", true); // Rendering cubemap in the framebuffer
    LoadShader("Framebuffer_Outlines", "Framebuffer", "Framebuffer", "Framebuffer_Outlines", true); // Rendering cubemap with outlines only
    LoadShader("Particle", "Particle", "Particle", "Particle", true);   // Rendering firefly particles effect
}


void AR_Mirror::LoadTextures()
{
    std::string texturePath_cubemap = PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES, "cube");

    cubeMapTextureID = UploadCubeMapTexture(
        PATH_JOIN(texturePath_cubemap, "pos_x.png"),
        PATH_JOIN(texturePath_cubemap, "pos_y.png"),
        PATH_JOIN(texturePath_cubemap, "pos_z.png"),
        PATH_JOIN(texturePath_cubemap, "neg_x.png"),
        PATH_JOIN(texturePath_cubemap, "neg_y.png"),
        PATH_JOIN(texturePath_cubemap, "neg_z.png"));

    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::MODELS), "characters", "archer", "Akai_E_Espiritu.fbm", "akai_diffuse.png");
    TextureManager::LoadTexture(PATH_JOIN(window->props.selfDir, RESOURCE_PATH::TEXTURES), "particle2.png");
}


void AR_Mirror::LoadShader(const std::string& name, const std::string& VS, const std::string& FS, const std::string& GS, bool hasGeomtery)
{
    std::string shaderPath = PATH_JOIN(window->props.selfDir, SOURCE_PATH::M2, "ar-mirror", "shaders");
    Shader* shader = new Shader(name);
    shader->AddShader(PATH_JOIN(shaderPath, VS + ".VS.glsl"), GL_VERTEX_SHADER);
    shader->AddShader(PATH_JOIN(shaderPath, FS + ".FS.glsl"), GL_FRAGMENT_SHADER);
    if (hasGeomtery)
    {
        shader->AddShader(PATH_JOIN(shaderPath, GS + ".GS.glsl"), GL_GEOMETRY_SHADER);
    }

    shader->CreateAndLink();
    shaders[shader->GetName()] = shader;
}


unsigned int AR_Mirror::UploadCubeMapTexture(const std::string &pos_x, const std::string &pos_y, const std::string &pos_z, const std::string& neg_x, const std::string& neg_y, const std::string& neg_z)
{
    int width, height, chn;

    unsigned char* data_pos_x = stbi_load(pos_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_y = stbi_load(pos_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_pos_z = stbi_load(pos_z.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_x = stbi_load(neg_x.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_y = stbi_load(neg_y.c_str(), &width, &height, &chn, 0);
    unsigned char* data_neg_z = stbi_load(neg_z.c_str(), &width, &height, &chn, 0);

    // Create the texture
    unsigned int textureID = 0;
    glGenTextures(1, &textureID);

    // Bind the texture
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    if (GLEW_EXT_texture_filter_anisotropic) {
        float maxAnisotropy;

        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Load texture information for each face
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_pos_z);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_x);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_y);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data_neg_z);

    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

    // Free memory
    SAFE_FREE(data_pos_x);
    SAFE_FREE(data_pos_y);
    SAFE_FREE(data_pos_z);
    SAFE_FREE(data_neg_x);
    SAFE_FREE(data_neg_y);
    SAFE_FREE(data_neg_z);

    return textureID;
}


void AR_Mirror::CreateFramebuffer(int width, int height)
{
    // Generate and bind the framebuffer
    glGenFramebuffers(1, &framebuffer_object);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_object);

    // Generate and bind the color texture
    glGenTextures(1, &color_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, color_texture);

    // Initialize the color textures
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, NULL);

    if (color_texture) {
        //Cubemap parameters
        glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        if (GLEW_EXT_texture_filter_anisotropic) {
            float maxAnisotropy;

            glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &maxAnisotropy);
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, maxAnisotropy);
        }
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // Bind the color textures to the framebuffer as a color attachments
        glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, color_texture, 0);

        glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

        std::vector<GLenum> draw_textures;
        draw_textures.push_back(GL_COLOR_ATTACHMENT0);
        glDrawBuffers((GLsizei)draw_textures.size(), &draw_textures[0]);
    }

    // Generate and bind the depth texture
    glGenTextures(1, &depth_texture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, depth_texture);

    // Initialize the depth textures
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_DEPTH_COMPONENT, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);

    if (depth_texture) {
        glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, depth_texture, 0);
    }

    glCheckFramebufferStatus(GL_FRAMEBUFFER);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void AR_Mirror::OnKeyPress(int key, int mods)
{
    // Toggle outline drawing 
    if (key == GLFW_KEY_SPACE)
        draw_outlines = draw_outlines == 0 ? 1 : 0;

    // Toggle firefly effect drawing 
    if (key == GLFW_KEY_F)
    {
        draw_fireflyEffect = draw_fireflyEffect == 0 ? 1 : 0;
        fireflyEffect->ResetParticles(fireflyEffect_radius);
    }
}


void AR_Mirror::OnInputUpdate(float deltaTime, int mods)
{
    // Don't move mirror if the camera is moving
    if (window->MouseHold(GLFW_MOUSE_BUTTON_2))
        return;
    
    // Mirror translations
    if (window->KeyHold(GLFW_KEY_S))
        mirror_translate_z += translate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_W))
        mirror_translate_z -= translate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_D))
        mirror_translate_x += translate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_A))
        mirror_translate_x -= translate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_E))
        mirror_translate_y += translate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_Q))
        mirror_translate_y -= translate_step * deltaTime;

    // Mirror rotations
    if (window->KeyHold(GLFW_KEY_O))
        mirror_rotate_OZ += rotate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_U))
        mirror_rotate_OZ -= rotate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_I))
        mirror_rotate_OX += rotate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_K))
        mirror_rotate_OX -= rotate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_L))
        mirror_rotate_OY += rotate_step * deltaTime;

    if (window->KeyHold(GLFW_KEY_J))
        mirror_rotate_OY -= rotate_step * deltaTime;
}