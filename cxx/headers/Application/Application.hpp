#pragma once
#include <dllapi.hpp>
#include <libs.hpp>
#include "../Renderer/Shader.hpp"
#include "../Renderer/Texture.hpp"
#include "../Renderer/Renderer.hpp"
#include "../Renderer/Timestep.hpp"

namespace Hyper {
    class DLL_API Application {
    public:
        static inline Application *instance;
        bool isGuzimoInUse = false;
        bool mouseClicked = false;
        int sceneMouseX, sceneMouseY;

        bool renderOnScreen = false;
        int winWidth, winHeight;
        int width;
        int height;
        int sw = 0, sh = 0;
        float exposure = 1;

        std::string vendor, srenderer, version;

        const std::string title;
        std::function<void()> m_update;

        HyperAPI::Renderer *renderer;
        HyperAPI::Experimental::GameObject *currently_hovering_over;

        Application(
            const int width, const int height, const char *gameTitle,
            bool fullscreen = false, bool resizable = true,
            bool wireframe = false, std::function<void()> ioConf = []() {})
            : width(width), height(height), title(std::string(gameTitle)) {
            instance = this;
            HYPER_LOG("Initializing Vault Engine");
            renderer =
                new HyperAPI::Renderer(width, height, title.c_str(), {0, -1}, 8,
                                       fullscreen, resizable, wireframe);
            glfwWindowHint(GLFW_DECORATED, false);
            HYPER_LOG("Initialized Vault Engine");

            // HYPER_LOG("Initializing Audio Engine");
            // HyperAPI::AudioEngine::Init();
            // HYPER_LOG("Initialized Audio Engine");

            // get vendor
            vendor = (char *)glGetString(GL_VENDOR);
            srenderer = (char *)glGetString(GL_RENDERER);
            version = (char *)glGetString(GL_VERSION);

            ImGui::CreateContext();
            ImGuiIO &io = ImGui::GetIO();

            ioConf();

            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 2.0f);

            ImGui_ImplGlfw_InitForOpenGL(renderer->window, true);
            ImGui_ImplOpenGL3_Init("#version 330");
            HYPER_LOG("Initialized ImGui");
        }

        void Run(
            std::function<void(uint32_t &, HyperAPI::Shader &)> update,
            std::function<void(uint32_t &PPT, uint32_t &PPFBO,
                               uint32_t &gui_gui)>
                gui = [](uint32_t &PPT, uint32_t &PPFBO, uint32_t &gui_gui) {},
            std::function<void(HyperAPI::Shader &)> shadowMapRender =
                [](HyperAPI::Shader &m_shadowMapShader) {});
    };

    class DLL_API MousePicker {
    public:
        Vector3 currentRay;
        glm::mat4 projectionMatrix;
        HyperAPI::Camera *camera;
        Application *appRef;

        int winX, winY;
        int mouseX, mouseY;

        MousePicker(Application *app, HyperAPI::Camera *camera,
                    glm::mat4 projection);

        Vector3 getCurrentRay();

        void update();

        Vector3 calculateMouseRay();

        Vector2 getNormalizedDeviceCoords(float mouseX, float mouseY);

        Vector4 toEyeCoords(Vector4 clipCoords);

        Vector3 toWorldCoords(Vector4 eyeCoords);
    };

    DLL_API float LerpFloat(float a, float b, float t);
} // namespace Hyper
