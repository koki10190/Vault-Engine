#include <random>
#include <memory>
#include "lib/api.hpp"
#include <experimental/filesystem>

namespace fs = std::experimental::filesystem;

#ifndef _WIN32

#include <unistd.h>

#else
#include <direct.h>
#endif

using namespace HyperAPI;
using namespace HyperAPI::Experimental;

class CollisionListener : public b2ContactListener {
public:
    void BeginContact(b2Contact *contact) override {
        b2Fixture *fixtureA = contact->GetFixtureA();
        b2Fixture *fixtureB = contact->GetFixtureB();
        // getuser data
        b2BodyUserData &bodyUserDataA = fixtureA->GetBody()->GetUserData();
        b2BodyUserData &bodyUserDataB = fixtureB->GetBody()->GetUserData();

        auto *gameObjectA = (GameObject *) bodyUserDataA.pointer;
        auto *gameObjectB = (GameObject *) bodyUserDataB.pointer;

        if (gameObjectA->HasComponent<NativeScriptManager>()) {
            auto &scriptManager = gameObjectA->GetComponent<NativeScriptManager>();
            for (auto script : scriptManager.m_StaticScripts) {
                script->Collision2D(gameObjectB);
            }
        }

        if (gameObjectB->HasComponent<NativeScriptManager>()) {
            auto &scriptManager = gameObjectB->GetComponent<NativeScriptManager>();
            for (auto script : scriptManager.m_StaticScripts) {
                script->Collision2D(gameObjectA);
            }
        }

        if (gameObjectA->HasComponent<m_LuaScriptComponent>()) {
            auto &scriptManager = gameObjectA->GetComponent<m_LuaScriptComponent>();
            for (auto script : scriptManager.scripts) {
                script.Collision2D(gameObjectB);
            }
        }

        if (gameObjectB->HasComponent<m_LuaScriptComponent>()) {
            auto &scriptManager = gameObjectB->GetComponent<m_LuaScriptComponent>();
            for (auto script : scriptManager.scripts) {
                script.Collision2D(gameObjectA);
            }
        }
    }

    void EndContact(b2Contact *contact) override {
        b2Fixture *fixtureA = contact->GetFixtureA();
        b2Fixture *fixtureB = contact->GetFixtureB();
        // getuser data
        b2BodyUserData &bodyUserDataA = fixtureA->GetBody()->GetUserData();
        b2BodyUserData &bodyUserDataB = fixtureB->GetBody()->GetUserData();

        auto *gameObjectA = (GameObject *) bodyUserDataA.pointer;
        auto *gameObjectB = (GameObject *) bodyUserDataB.pointer;

        if (gameObjectA->HasComponent<NativeScriptManager>()) {
            auto &scriptManager = gameObjectA->GetComponent<NativeScriptManager>();
            for (auto script : scriptManager.m_StaticScripts) {
                script->CollisionExit2D(gameObjectB);
            }
        }

        if (gameObjectB->HasComponent<NativeScriptManager>()) {
            auto &scriptManager = gameObjectB->GetComponent<NativeScriptManager>();
            for (auto script : scriptManager.m_StaticScripts) {
                script->CollisionExit2D(gameObjectA);
            }
        }

        if (gameObjectA->HasComponent<m_LuaScriptComponent>()) {
            auto &scriptManager = gameObjectA->GetComponent<m_LuaScriptComponent>();
            for (auto script : scriptManager.scripts) {
                script.CollisionExit2D(gameObjectB);
            }
        }

        if (gameObjectB->HasComponent<m_LuaScriptComponent>()) {
            auto &scriptManager = gameObjectB->GetComponent<m_LuaScriptComponent>();
            for (auto script : scriptManager.scripts) {
                script.CollisionExit2D(gameObjectA);
            }
        }
    }
};

CollisionListener *listener = new CollisionListener();

struct Config {
    char name[50];
    std::string mainScene;
    float ambientLight;
    bool resizable;
    bool fullscreenOnLaunch;
    int width, height;
};

struct AddComponentList {
    int selected = 0;
    int length = 1;
    bool showed = false;
    const char *items[1] = {
            "Lua Scripts Component"
    };
};

Config config = {
        "Static Engine",
        "assets/scenes/main.static",
        0.2
};

AddComponentList AddComponentConfig;

std::string originalName = "";
bool LoadState = false;

bool ends_with(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void DirIter(const std::string &path) {
    for (const auto &entry : fs::directory_iterator(path)) {
        if (fs::is_directory(entry)) {
            if (ImGui::TreeNodeEx((std::string(ICON_FA_FOLDER) + " " + entry.path().filename().string()).c_str(),
                                  ImGuiTreeNodeFlags_SpanAvailWidth)) {
                DirIter(entry.path().string());
                ImGui::TreePop();
            }
        } else {
            // offset for the arrow thing
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 25);
            ImGui::Selectable((std::string(ICON_FA_FILE) + " " + entry.path().filename().string()).c_str());
            // make it draggable

            // disable it getting out of the window
            ImGui::SetItemAllowOverlap();
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
                dirPayloadData = entry.path().string();
                ImGui::SetDragDropPayload("file", dirPayloadData.c_str(), strlen(dirPayloadData.c_str()));
                ImGui::Text(entry.path().filename().string().c_str());
                ImGui::EndDragDropSource();
            }
        }
    }
}

void ApplyMaterial(nlohmann::json JSON, Material &material, int i) {
    material.baseColor = glm::vec4(
            JSON[i]["material"]["baseColor"]["r"],
            JSON[i]["material"]["baseColor"]["g"],
            JSON[i]["material"]["baseColor"]["b"],
            1
    );

    material.roughness = JSON[i]["material"]["roughness"];
    material.metallic = JSON[i]["material"]["metallic"];
    material.texUVs = Vector2(JSON[i]["material"]["texUV"]["x"], JSON[i]["material"]["texUV"]["y"]);

    if (JSON[i]["material"]["diffuse"] != "nullptr") {
        std::string diffusePath = JSON[i]["material"]["diffuse"];
        material.diffuse = new Texture((char *) diffusePath.c_str(), 0, "texture_diffuse");
    }

    if (JSON[i]["material"]["specular"] != "nullptr") {
        std::string specularPath = JSON[i]["material"]["specular"];
        material.specular = new Texture((char *) specularPath.c_str(), 1, "texture_specular");
    }

    if (JSON[i]["material"]["normal"] != "nullptr") {
        std::string normalPath = JSON[i]["material"]["normal"];
        material.normal = new Texture((char *) normalPath.c_str(), 2, "texture_normal");
    }
}

namespace InspecType {
    enum Type {
        None,
        Material
    };
}

struct InspectorMaterial {
    std::string diffuse = "None";
    std::string specular = "None";
    std::string normal = "None";
    float metallic = 0;
    float roughness = 0;
    Vector4 baseColor = Vector4(1, 1, 1, 1);
    Vector2 texUVs = Vector2(0, 0);
};

bool bulletPhysicsStarted = false;
void StartWorld() {
    InitScripts();

    for (auto &gameObject : Scene::m_GameObjects) {
        if (gameObject->HasComponent<m_LuaScriptComponent>()) {
            gameObject->GetComponent<m_LuaScriptComponent>().Start();
        }

        if (gameObject->HasComponent<NativeScriptManager>()) {
            gameObject->GetComponent<NativeScriptManager>().Start();
        }
    }

    Scene::world = new b2World({0.0, -5.8f});
    Scene::world->SetContactListener(listener);
    auto view = Scene::m_Registry.view<Rigidbody2D>();
    auto view3D = Scene::m_Registry.view<Rigidbody3D>();

    for (auto e : view) {
        GameObject *gameObject;
        for (auto &go : Scene::m_GameObjects) {
            if (go->entity == e) {
                gameObject = go;
                break;
            }
        }

        auto &transform = gameObject->GetComponent<Transform>();
        auto &rb2d = gameObject->GetComponent<Rigidbody2D>();

        b2BodyDef bodyDef;
        bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(gameObject);
        bodyDef.type = rb2d.type;
        bodyDef.position.Set(transform.position.x, transform.position.y);
        bodyDef.angle = glm::radians(transform.rotation.z);
        bodyDef.gravityScale = rb2d.gravityScale;

        b2Body *body = Scene::world->CreateBody(&bodyDef);
        body->SetFixedRotation(rb2d.fixedRotation);
        rb2d.body = body;

        if (gameObject->HasComponent<BoxCollider2D>()) {
            auto &boxCollider2D = gameObject->GetComponent<BoxCollider2D>();
            b2PolygonShape shape;
            shape.SetAsBox((((boxCollider2D.size.x) / 2) - 0.02) / 2, (((boxCollider2D.size.y) / 2) - 0.02) / 2);

            b2FixtureDef fixtureDef;
            fixtureDef.isSensor = boxCollider2D.trigger;
            fixtureDef.shape = &shape;
            fixtureDef.density = boxCollider2D.density;
            fixtureDef.friction = boxCollider2D.friction;
            fixtureDef.restitution = boxCollider2D.restitution;
            fixtureDef.restitutionThreshold = boxCollider2D.restitutionThreshold;
            boxCollider2D.fixture = body->CreateFixture(&fixtureDef);
        }
    }

    BulletPhysicsWorld::Init();

    for(auto e : view3D) {
        GameObject *gameObject;
        for (auto &go : Scene::m_GameObjects) {
            if (go->entity == e) {
                gameObject = go;
                break;
            }
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody3D>();
        rigidbody.transform = &gameObject->GetComponent<Transform>();

        if(gameObject->HasComponent<BoxCollider3D>()) {
            auto &collider = gameObject->GetComponent<BoxCollider3D>();
            collider.CreateShape();
            rigidbody.CreateBody(collider.shape);
        }

        if(gameObject->HasComponent<MeshCollider3D>()) {
            auto &collider = gameObject->GetComponent<MeshCollider3D>();
            collider.CreateShape(&gameObject->GetComponent<MeshRenderer>());
            rigidbody.CreateBody(collider.shape);
        }
    }

    auto jointView = Scene::m_Registry.view<FixedJoint3D>();

    for(auto e : jointView) {
        GameObject *gameObject;
        for (auto &go : Scene::m_GameObjects) {
            if (go->entity == e) {
                gameObject = go;
                break;
            }
        }

        auto &joint = gameObject->GetComponent<FixedJoint3D>();
        joint.CreateJoint();
    }

    bulletPhysicsStarted = true;
}

void DeleteWorld() {
    bulletPhysicsStarted = false;

    // halt
    Mix_HaltChannel(-1);
    Mix_HaltMusic();

    for (auto &gameObject : Scene::m_GameObjects) {
        if(gameObject->HasComponent<FixedJoint3D>()) {
            auto &fixedJoint = gameObject->GetComponent<FixedJoint3D>();
            fixedJoint.DeleteJoint();
        }
    }

    for (auto &gameObject : Scene::m_GameObjects) {
        if (gameObject->HasComponent<NativeScriptManager>()) {
            auto &script = gameObject->GetComponent<NativeScriptManager>();
            for (auto &script : script.m_StaticScripts) {
                delete script;
            }

            gameObject->RemoveComponent<NativeScriptManager>();
        }

        if(gameObject->HasComponent<Rigidbody3D>()) {
            auto &component = gameObject->GetComponent<Rigidbody3D>();
            component.DeleteBody();
            if(component.ref) {
                delete component.ref;
            }
        }


    }

    BulletPhysicsWorld::Delete();
}

time_t timestamp = time(0);

#ifndef _WIN32 || GAME_BUILD

void UpdatePresence(
        const std::string &details = "",
        const std::string &state = "",
        const std::string &largeImageKey = "logo",
        const std::string &largeImageText = "",
        const std::string &smallImageKey = "",
        const std::string &smallImageText = ""
) {
    char buffer[256];
    DiscordRichPresence discordPresence;
    memset(&discordPresence, 0, sizeof(discordPresence));
    discordPresence.largeImageKey = largeImageKey.c_str();
    discordPresence.largeImageText = largeImageText.c_str();
    discordPresence.smallImageKey = smallImageKey.c_str();
    discordPresence.smallImageText = smallImageText.c_str();
    discordPresence.state = state.c_str();
    discordPresence.details = details.c_str();
    discordPresence.startTimestamp = timestamp;
    Discord_UpdatePresence(&discordPresence);
}

#endif

int main() {
#ifndef _WIN32 || GAME_BUILD
    DiscordEventHandlers handlers;
    memset(&handlers, 0, sizeof(handlers));
    handlers.ready = [](const DiscordUser *request) {
        std::cout << "Discord: Ready" << std::endl;
    };

    handlers.errored = [](int errorCode, const char *message) {
        std::cout << "Discord: Error " << errorCode << ": " << message << std::endl;
    };

    handlers.disconnected = [](int errorCode, const char *message) {
        std::cout << "Discord: Disconnected " << errorCode << ": " << message << std::endl;
    };

    handlers.joinGame = [](const char *joinSecret) {
        std::cout << "Discord: Join Game " << joinSecret << std::endl;
    };

    handlers.spectateGame = [](const char *spectateSecret) {
        std::cout << "Discord: Spectate Game " << spectateSecret << std::endl;
    };

    Discord_Initialize("1025522890688442400", &handlers, 1, nullptr);

    UpdatePresence(
            "In Editor",
            "Making a game"
    );
#endif

    Scene::layers["Default"] = true;
    // ScriptEngine::Init();

    char CWD[1024];
#ifdef _WIN32
    _getcwd(CWD, sizeof(CWD));
#else
    getcwd(CWD, sizeof(CWD));
#endif
    cwd = std::string(CWD);

    // check if game.config exists
    std::ifstream file("assets/game.config");
    bool mainSceneFound = false;

    if (file.is_open()) {
        nlohmann::json JSON = nlohmann::json::parse(file);

        config.width = JSON["width"];
        config.height = JSON["height"];
        strcpy(config.name, ((std::string) JSON["name"]).c_str());
        config.ambientLight = JSON["ambientLight"];
        config.mainScene = JSON["mainScene"];
        config.resizable = JSON["resizable"];
        config.fullscreenOnLaunch = JSON["fullscreen_on_launch"];
        for (auto &layer : JSON["layers"]) {
            Scene::layers[(std::string) layer] = true;
        }
        mainSceneFound = true;
    } else {
        nlohmann::json j = {
                {"name",                 config.name},
                {"ambientLight",         config.ambientLight},
                {"mainScene",            config.mainScene},
                {"width",                config.width},
                {"height",               config.height},
                {"resizable",            config.resizable},
                {"fullscreen_on_launch", config.fullscreenOnLaunch},
                {"layers",               {
                                                 "Default"
                                         }}
        };

        std::ofstream o("assets/game.config");
        o << std::setw(4) << j << std::endl;
    }

#ifdef GAME_BUILD
    Hyper::Application app(1280, 720, "Static Engine", config.fullscreenOnLaunch, config.resizable);
    app.renderOnScreen = true;
#else
    Hyper::Application app(1280, 720, "Static Engine", false);
#endif

    Input::window = app.renderer->window;
    // glfw enable sticky mouse buttons
    Shader shader("shaders/default.glsl");
    Shader spriteShader("shaders/sprite.glsl");
    Shader workerShader("shaders/worker.glsl");
    Shader outlineShader("shaders/outline.glsl");
    // Shader batchShader("shaders/batch.glsl");

    spriteShader.Bind();
    spriteShader.SetUniform1f("ambient", 1);

    shader.Bind();
    shader.SetUniform1f("ambient", 0.2);

    Skybox skybox("assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg",
                  "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg");
    auto *camera = new Camera(false, app.width, app.height, Vector3(0, 3, 15));

#ifndef GAME_BUILD
    Scene::mainCamera = camera;
#endif

    Hyper::MousePicker picker(&app, camera, camera->projection);

    bool focusedOnScene = false;
    bool hoveredScene = false;

    Vector2 winSize = Vector2(0, 0);
    Vector2 winPos = Vector2(0, 0);

    ScriptEngine::window = app.renderer->window;

    bool openConfig = false;
    bool openDetails = false;
    bool openInspector = false;
    bool openLayers = false;
    char layerName[32] = "New Layer";

    int inspectorType = InspecType::None;

    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImVec4(1, 1, 1, 1.0f), ICON_FA_FILE);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImVec4(1, 1, 1, 1.0f), ICON_FA_FOLDER);

    InspectorMaterial m_InspectorMaterial;

    Scene::LoadScene(config.mainScene);

#ifdef GAME_BUILD
    if(Scene::mainCamera == nullptr) {
        Scene::mainCamera = camera;
    }
#else
    Scene::mainCamera = camera;
#endif

    Transform transform;
    transform.position = glm::vec3(2, 0, 2);
    transform.rotation = glm::vec3(15, 0, 0);
    transform.scale = glm::vec3(1, 5, 1);
    transform.Update();

    ImVec2 mousePos = ImVec2(0, 0);
    ImVec2 windowPos = ImVec2(0, 0);
    ImVec2 windowSize = ImVec2(0, 0);

    int frameCount = 0;
    float timeElapsed = 0.0f;
    int fps = 0;
    std::string fpsText;

    int m_GuizmoMode = -1;

    std::vector<Vertex_Batch> vertices =
            {
            };

    std::vector<unsigned int> indices =
            {
            };

    bool usingImGuizmo = false;

#ifndef GAME_BUILD
    std::vector<Vertex> cubeVertices = {
            // front
            {Vector3(-0.5f, -0.5f, 0.5f),  Vector3(0), Vector3(0, 0, 1),  Vector2(0, 0)},
            {Vector3(0.5f, -0.5f, 0.5f),   Vector3(0), Vector3(0, 0, 1),  Vector2(1, 0)},
            {Vector3(0.5f, 0.5f, 0.5f),    Vector3(0), Vector3(0, 0, 1),  Vector2(1, 1)},
            {Vector3(-0.5f, 0.5f, 0.5f),   Vector3(0), Vector3(0, 0, 1),  Vector2(0, 1)},
            // back
            {Vector3(-0.5f, -0.5f, -0.5f), Vector3(0), Vector3(0, 0, -1), Vector2(1, 0)},
            {Vector3(0.5f, -0.5f, -0.5f),  Vector3(0), Vector3(0, 0, -1), Vector2(0, 0)},
            {Vector3(0.5f, 0.5f, -0.5f),   Vector3(0), Vector3(0, 0, -1), Vector2(0, 1)},
            {Vector3(-0.5f, 0.5f, -0.5f),  Vector3(0), Vector3(0, 0, -1), Vector2(1, 1)},
    };

    // indices
    std::vector<unsigned int> cubeIndices = {
            0, 1, 2, 2, 3, 0,
            1, 5, 6, 6, 2, 1,
            7, 6, 5, 5, 4, 7,
            4, 0, 3, 3, 7, 4,
            4, 5, 1, 1, 0, 4,
            3, 2, 6, 6, 7, 3
    };

    // flat plane vertices
    std::vector<Vertex> planeVertices = {
            {Vector3(-0.5f, 0.0f, 0.5f),  Vector3(0), Vector3(0, 1, 0), Vector2(0, 0)},
            {Vector3(0.5f, 0.0f, 0.5f),   Vector3(0), Vector3(0, 1, 0), Vector2(1, 0)},
            {Vector3(0.5f, 0.0f, -0.5f),  Vector3(0), Vector3(0, 1, 0), Vector2(1, 1)},
            {Vector3(-0.5f, 0.0f, -0.5f), Vector3(0), Vector3(0, 1, 0), Vector2(0, 1)},
    };

    // indices
    std::vector<unsigned int> planeIndices = {
            0, 1, 2, 2, 3, 0
    };

    Material material(Vector4(0, 4, 0.2, 1));
    Mesh CubeCollider(cubeVertices, cubeIndices, material);
    Mesh mesh_BoxCollider2D(planeVertices, planeIndices, material);
    bool drawBoxCollider2D = false;
    glm::vec3 bc2dPos = glm::vec3(0, 0, 0);
    glm::vec3 bc2dScale = glm::vec3(1, 1, 1);
    float bc2dRotation = 0.0f;

    std::vector<Vertex> sprite_vertices = {
            Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec3(1, 1, 1), glm::vec3(0, 1, 0),
                   glm::vec2(0.0f, 0.0f)},
            Vertex{glm::vec3(0.5f, -0.5f, 0.0f), glm::vec3(1, 1, 1), glm::vec3(0, 1, 0),
                   glm::vec2(1.0f, 0.0f)},
            Vertex{glm::vec3(0.5f, 0.5f, 0.0f), glm::vec3(1, 1, 1), glm::vec3(0, 1, 0),
                   glm::vec2(1.0f, 1.0f)},
            Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec3(1, 1, 1), glm::vec3(0, 1, 0),
                   glm::vec2(0.0f, 1.0f)}
    };

    std::vector<unsigned int> sprite_indices = {
            0, 1, 2,
            0, 2, 3
    };

    auto *dirLightIcon_texture = new Texture("assets/icons/directional_light.png", 0, "texture_diffuse");
    auto *pointLightIcon_texture = new Texture("assets/icons/point_light.png", 0, "texture_diffuse");
    auto *spotLightIcon_texture = new Texture("assets/icons/spot_light.png", 0, "texture_diffuse");
    auto *cameraIcon_texture = new Texture("assets/icons/camera.png", 0, "texture_diffuse");

    Material dirLightIconMaterial(Vector4(1,1,1,1));
    dirLightIconMaterial.diffuse = dirLightIcon_texture;
    Mesh dirLightIconMesh(sprite_vertices, sprite_indices, dirLightIconMaterial);

    Material pointLightIconMaterial(Vector4(1,1,1,1));
    pointLightIconMaterial.diffuse = pointLightIcon_texture;
    Mesh pointLightIconMesh(sprite_vertices, sprite_indices, pointLightIconMaterial);

    Material spotLightIconMaterial(Vector4(1,1,1,1));
    spotLightIconMaterial.diffuse = spotLightIcon_texture;
    Mesh spotLightIconMesh(sprite_vertices, sprite_indices, spotLightIconMaterial);

    Material cameraIconMaterial(Vector4(1,1,1,1));
    cameraIconMaterial.diffuse = cameraIcon_texture;
    Mesh cameraIconMesh(sprite_vertices, sprite_indices, cameraIconMaterial);

#endif

#ifdef GAME_BUILD
    StartWorld();
    HyperAPI::isRunning = true;
    HyperAPI::isStopped = false;
#endif

    Scene::SceneType sceneType = Scene::MAIN_SCENE;
    std::function<void(unsigned int &PPT, unsigned int &PPFBO)> GUI_EXP =
            [&](unsigned int &PPT, unsigned int &PPFBO) {
#ifdef GAME_BUILD
                return;
#else
                if (ImGui::BeginMainMenuBar()) {
                    if (ImGui::BeginMenu("File")) {
                        if (ImGui::MenuItem("Save Scene", "CTRL+S")) {
                            if (Scene::currentScenePath == "") {
                                ImGuiFileDialog::Instance()->OpenDialog("SaveSceneDialog",
                                                                        ICON_FA_FLOPPY_DISK " Save Scene", ".static",
                                                                        ".");
                            } else {
                                Scene::SaveScene(Scene::currentScenePath);
                            }
                        }

                        if (ImGui::MenuItem("Save Scene As", "CTRL+SHIFT+S")) {
                            ImGuiFileDialog::Instance()->OpenDialog("SaveSceneDialog",
                                                                    ICON_FA_FLOPPY_DISK " Save Scene", ".static", ".");
                        }

                        if (ImGui::MenuItem("Config", "CTRL+SHIFT+C")) {
                            openConfig = true;
                        }

                        if (ImGui::MenuItem("Build for Linux", "CTRL+B+L")) {
                            ImGuiFileDialog::Instance()->OpenDialog("BuildLinuxDialog", "Build for Linux", nullptr,
                                                                    ".");
                        }

                        if (ImGui::MenuItem("Build for Windows", "CTRL+B+W")) {
                            ImGuiFileDialog::Instance()->OpenDialog("BuildWindowsDialog", "Build for Windows", nullptr,
                                                                    ".");
                        }

                        // if(ImGui::MenuItem("Build")) {
                        // ImGuiFileDialog::Instance()->OpenDialog("BuildDialog", "Build", nullptr, cwd + "/builds");
                        // }
                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Tools")) {
                        if (ImGui::MenuItem("Inspector")) {
                            openInspector = true;
                        }

                        if (ImGui::MenuItem("Layers")) {
                            openLayers = true;
                        }

                        ImGui::EndMenu();
                    }
                    if (ImGui::BeginMenu("Info")) {
                        if (ImGui::MenuItem("Details")) {
                            openDetails = true;
                        }

                        ImGui::EndMenu();
                    }
                    ImGui::EndMainMenuBar();
                }

                ImGui::DockSpaceOverViewport(ImGui::GetMainViewport(), ImGuiDockNodeFlags_PassthruCentralNode);
//                ImGui::ShowDemoWindow();

                if (openLayers) {
                    if (ImGui::Begin(ICON_FA_LAYER_GROUP " Layers")) {
                        if (ImGui::TreeNode("New Layer")) {
                            ImGui::InputText("Name", layerName, 32);
                            if (ImGui::Button("Create")) {
                                std::ofstream file("assets/game.config");
                                Scene::layers[layerName] = true;
                                strcpy(layerName, "New Layer");
                                std::vector<std::string> layerStarters;
                                for (auto &layer : Scene::layers) {
                                    layerStarters.push_back(layer.first);
                                }


                                nlohmann::json j = {
                                        {"name",         config.name},
                                        {"ambientLight", config.ambientLight},
                                        {"mainScene",    config.mainScene},
                                        {"width",        config.width},
                                        {"height",       config.height},
                                        {"layers",       layerStarters}
                                };

                                file << j.dump(4);
                                file.close();
                            }

                            ImGui::TreePop();
                        }
                        ImGui::NewLine();

                        for (auto layer : Scene::layers) {
                            ImGui::PushID(typeid(layer).hash_code());
                            ImGui::Text(((std::string(ICON_FA_PHOTO_FILM)) + std::string(layer.first)).c_str());
                            ImGui::PopID();
                        }

                        ImGui::NewLine();
                        if (ImGui::Button("Close")) {
                            openLayers = false;
                        }

                    }
                    ImGui::End();
                }

                if (openInspector) {
                    if (ImGui::Begin(ICON_FA_MAGNIFYING_GLASS " Inspector")) {
                        if (ImGui::BeginDragDropTarget()) {
                            if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("file")) {
                                // const char* path = (const char*)payload->Data;
                                dirPayloadData.erase(0, cwd.length() + 1);

                                if (
                                        ends_with(dirPayloadData, ".material")
                                        ) {
                                    inspectorType = InspecType::Material;
                                    std::ifstream file(dirPayloadData);
                                    nlohmann::json JSON = nlohmann::json::parse(file);

                                    m_InspectorMaterial.diffuse =
                                            JSON["diffuse"] == "nullptr" ? "None" : JSON["diffuse"];
                                    m_InspectorMaterial.specular =
                                            JSON["specular"] == "nullptr" ? "None" : JSON["specular"];
                                    m_InspectorMaterial.normal = JSON["normal"] == "nullptr" ? "None" : JSON["normal"];
                                    m_InspectorMaterial.roughness = JSON["roughness"];
                                    m_InspectorMaterial.metallic = JSON["metallic"];

                                    file.close();
                                }
                            }

                            ImGui::EndDragDropTarget();
                        }

                        switch (inspectorType) {
                            case InspecType::Material: {
                                if (ImGui::TreeNode("Diffuse")) {
                                    if (ImGui::BeginDragDropTarget()) {
                                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("file")) {
                                            dirPayloadData.erase(0, cwd.length() + 1);
                                            m_InspectorMaterial.diffuse = dirPayloadData;
                                        }

                                        ImGui::EndDragDropTarget();
                                    }

                                    ImGui::Text(m_InspectorMaterial.diffuse.c_str());

                                    ImGui::TreePop();
                                }

                                if (ImGui::TreeNode("Specular")) {
                                    if (ImGui::BeginDragDropTarget()) {
                                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("file")) {
                                            dirPayloadData.erase(0, cwd.length() + 1);
                                            m_InspectorMaterial.specular = dirPayloadData;
                                        }

                                        ImGui::EndDragDropTarget();
                                    }

                                    ImGui::Text(m_InspectorMaterial.specular.c_str());

                                    ImGui::TreePop();
                                }

                                if (ImGui::TreeNode("Normal")) {
                                    if (ImGui::BeginDragDropTarget()) {
                                        if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("file")) {
                                            dirPayloadData.erase(0, cwd.length() + 1);
                                            m_InspectorMaterial.normal = dirPayloadData;
                                        }

                                        ImGui::EndDragDropTarget();
                                    }

                                    ImGui::Text(m_InspectorMaterial.normal.c_str());

                                    ImGui::TreePop();
                                }

                                ImGui::DragFloat2("UV Scale", &m_InspectorMaterial.texUVs.x, 0.01f);
                                ImGui::DragFloat("Roughness", &m_InspectorMaterial.roughness, 0.01f, 0.0f, 1.0f);
                                ImGui::DragFloat("Metallic", &m_InspectorMaterial.metallic, 0.01f, 0.0f, 1.0f);
                                ImGui::ColorEdit3("Color", &m_InspectorMaterial.baseColor.x);

                                if (ImGui::Button(ICON_FA_FLOPPY_DISK " Save Material")) {
                                    ImGuiFileDialog::Instance()->OpenDialog("SaveMaterialDialog",
                                                                            ICON_FA_FLOPPY_DISK " Save Material",
                                                                            ".material", ".");
                                }
                                ImGui::NewLine();


                                if (ImGuiFileDialog::Instance()->Display("SaveMaterialDialog")) {
                                    // action if OK
                                    if (ImGuiFileDialog::Instance()->IsOk()) {
                                        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                                        // remove cwd from filePathName
                                        filePathName.erase(0, cwd.length() + 1);
                                        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                                        std::ofstream file(filePathName);
                                        nlohmann::json j = {
                                                {"diffuse",   m_InspectorMaterial.diffuse},
                                                {"specular",  m_InspectorMaterial.specular},
                                                {"normal",    m_InspectorMaterial.normal},
                                                {"roughness", m_InspectorMaterial.roughness},
                                                {"metallic",  m_InspectorMaterial.metallic},
                                                {"baseColor", {
                                                                      {"r", m_InspectorMaterial.baseColor.x},
                                                                      {"g", m_InspectorMaterial.baseColor.y},
                                                                      {"b", m_InspectorMaterial.baseColor.z},
                                                                      {"a", m_InspectorMaterial.baseColor.w}
                                                              }
                                                },
                                                {"texUV",     {
                                                                      {"x", m_InspectorMaterial.texUVs.x},
                                                                      {"y", m_InspectorMaterial.texUVs.y}
                                                              }
                                                }
                                        };

                                        file << j.dump(4);
                                    }

                                    ImGuiFileDialog::Instance()->Close();
                                }

                                break;
                            }
                        }

                        if (inspectorType == InspecType::None) {
                            ImGui::Text("To inspect an object, drag it into the inspector window's title bar.");
                        }
                        if (ImGui::Button("Close")) {
                            inspectorType = InspecType::None;
                            openInspector = false;
                        }

                    }
                    ImGui::End();
                }

                if (ImGuiFileDialog::Instance()->Display("SaveSceneDialog")) {
                    // action if OK
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                        // remove cwd from filePathName
                        filePathName.erase(0, cwd.length() + 1);
                        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                        Scene::SaveScene(filePathName);
                    }

                    ImGuiFileDialog::Instance()->Close();
                }

                if (openConfig) {
                    ImGui::OpenPopup("Edit Config");
                    openConfig = false;
                }

                if (openDetails) {
                    ImGui::OpenPopup("Details");
                    openDetails = false;
                }

                ImGui::SetNextWindowSize(ImVec2(500, 0));
                if (ImGui::BeginPopup("Edit Config")) {
#ifndef _WIN32 || GAME_BUILD
                    UpdatePresence(
                            "In Editor",
                            "Editing Configurations"
                    );
#endif
                    ImGui::InputText("Game Name", config.name, 500);
                    ImGui::DragFloat("Ambient Lightning", &config.ambientLight, 0.01f, 0, 1);
                    ImGui::Checkbox("Fullscreen On Launch", &config.fullscreenOnLaunch);
                    ImGui::Checkbox("Resizable", &config.resizable);
                    ImGui::DragInt("Width", &config.width, 1, 0, 1920);
                    ImGui::DragInt("Height", &config.height, 1, 0, 1080);
                    if (ImGui::Button("Main Scene", ImVec2(500, 0))) {
                        ImGuiFileDialog::Instance()->OpenDialog("ChooseMainScene", "Choose Main Scene", ".static", ".");
#ifndef _WIN32 || GAME_BUILD
                        UpdatePresence(
                                "In Editor",
                                "Making a game"
                        );
#endif
                        // ImGui::CloseCurrentPopup();
                    }
                    ImGui::Separator();
                    if (ImGui::Button("Save", ImVec2(500, 0))) {
                        nlohmann::json j = {
                                {"name",                 config.name},
                                {"ambientLight",         config.ambientLight},
                                {"mainScene",            config.mainScene},
                                {"resizable",            config.resizable},
                                {"fullscreen_on_launch", config.fullscreenOnLaunch},
                                {"width",                config.width},
                                {"height",               config.height}
                        };

                        std::ofstream o("assets/game.config");
                        o << std::setw(4) << j << std::endl;

#ifndef _WIN32 || GAME_BUILD
                        UpdatePresence(
                                "In Editor",
                                "Making a game"
                        );
#endif

                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if (ImGuiFileDialog::Instance()->Display("ChooseMainScene")) {
                    // action if OK
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                        // remove cwd from filePathName
                        filePathName.erase(0, cwd.length() + 1);
                        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                        config.mainScene = filePathName;
                    }


                    // close
                    ImGuiFileDialog::Instance()->Close();
                }

                // docking with mutli-viewport
                if (ImGui::BeginPopup("Details")) {
                    ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
                    ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
                    ImGui::Text("Version: %s", glGetString(GL_VERSION));
                    ImGui::Text("Shading Language Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

#ifndef _WIN32 || GAME_BUILD
                    UpdatePresence(
                            "In Editor",
                            "Checking Details"
                    );
#endif

                    if (ImGui::Button("Close")) {
                        ImGui::CloseCurrentPopup();
                    }
                    ImGui::EndPopup();
                }

                if(ImGui::Begin(ICON_FA_GAMEPAD " Scene")) {
                    sceneType = Scene::MAIN_SCENE;

                    bool complete = Scene::DropTargetMat(Scene::DRAG_SCENE, nullptr, nullptr);
                    if(Scene::LoadingScene) {
                        Scene::m_Object = nullptr;
                    }

                    if(complete) {
                        Scene::mainCamera = nullptr;
                    }

                    ImVec2 w_s = ImGui::GetWindowSize();
                    winSize = Vector2(w_s.x, w_s.y);
                    // ImGui::SetWindowSize(ImVec2(w_s.x, w_s.y - 50));
                    if (camera->mode2D) {
                        if (ImGui::Button(ICON_FA_CAMERA " 3D View")) {
                            camera->mode2D = false;
                        }
                    } else {
                        if (ImGui::Button(ICON_FA_CAMERA " 2D View")) {
                            camera->mode2D = true;
                        }
                    }
                    // make them look like they are linked
                    ImGui::SameLine();

                    if(ImGui::Button(ICON_FA_ARROWS_UP_DOWN_LEFT_RIGHT)) {
                        m_GuizmoMode = ImGuizmo::OPERATION::TRANSLATE;
                    }
                    ImGui::SameLine();

                    if(ImGui::Button(ICON_FA_ARROWS_ROTATE)) {
                        m_GuizmoMode = ImGuizmo::OPERATION::ROTATE;
                    }

                    ImGui::SameLine();
                    if(ImGui::Button(ICON_FA_EXPAND)) {
                        m_GuizmoMode = ImGuizmo::OPERATION::SCALE;
                    }

                    ImGui::SameLine();

                    if (!HyperAPI::isRunning) {
                        if (HyperAPI::isStopped) {
                            ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - 25 / 2);
                        } else {
                            ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - 25 / 2 - (25 / 2));
                        }

                        if (ImGui::Button(ICON_FA_PLAY, ImVec2(25, 0))) {
                            StartWorld();

                            HyperAPI::isRunning = true;
                            HyperAPI::isStopped = false;

                            for (auto &camera : Scene::cameras) {
                                if (camera->mainCamera) {
                                    Scene::mainCamera = camera;
                                    break;
                                }
                            }
                        }

                        if (!HyperAPI::isStopped) {
                            // ImGui::SameLine();
                            // if(ImGui::Button(ICON_FA_STOP, ImVec2(25, 0))) {
                            //     HyperAPI::isRunning = false;
                            //     HyperAPI::isStopped = true;
                            //     Scene::mainCamera = camera;
                            // }
                        }
                    } else {
                        // move it more left
                        ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - 25 / 2 - (25 / 2));
                        if (ImGui::Button(ICON_FA_PAUSE, ImVec2(25, 0))) {
                            HyperAPI::isRunning = false;
                            DeleteWorld();
                            Scene::mainCamera = camera;
                        }
                        // ImGui::SameLine();
                        // if(ImGui::Button(ICON_FA_STOP, ImVec2(25, 0))) {
                        //     HyperAPI::isRunning = false;
                        //     HyperAPI::isStopped = true;
                        //     Scene::mainCamera = camera;
                        // }
                    }

                    ImGui::BeginChild("View");
                    if(sceneType == Scene::MAIN_SCENE) {
                        mousePos = ImGui::GetMousePos();
                        windowPos = ImGui::GetWindowPos();
                        windowSize = ImGui::GetWindowSize();
                        mousePos.x -= windowPos.x;
                        mousePos.y -= windowPos.y;
                        mousePos.y = windowSize.y - mousePos.y;
                        // std::cout << mx << " " << mousePos.y << std::endl;
                        app.sceneMouseX = mousePos.x;
                        app.sceneMouseY = mousePos.y;

                        app.width = windowSize.x;
                        app.height = windowSize.y;

                        ImVec2 w_p = ImGui::GetWindowPos();
                        Input::winPos = Vector3(w_p.x, w_p.y, 0);

                        glActiveTexture(GL_TEXTURE15);
                        glBindTexture(GL_TEXTURE_2D, PPT);

                        // check window hovered
                        if (ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0)) {
                            focusedOnScene = true;
                        } else if (!ImGui::IsMouseDragging(0)) {
                            focusedOnScene = false;
                        }

                        if (ImGui::IsWindowHovered()) {
                            hoveredScene = true;
                        } else if (!ImGui::IsMouseDragging(0)) {
                            hoveredScene = false;
                        }

                        ImGui::Image((void *) PPT, ImVec2(w_s.x, w_s.y - 40), ImVec2(0, 1), ImVec2(1, 0));
                    }

                    auto[viewWidth, viewHeight] = ImGui::GetWindowSize();
                    auto[viewX, viewY] = ImGui::GetWindowPos();

                    auto *selectedObject = Scene::m_Object;

//                    m_GuizmoMode = ImGuizmo::OPERATION::ROTATE
                    if(Scene::mainCamera != camera) m_GuizmoMode = -1;
                    if (selectedObject && m_GuizmoMode != -1) {
                        ImGuizmo::SetOrthographic(camera->mode2D);
                        ImGuizmo::SetDrawlist();
                        ImGuizmo::SetRect(viewX, viewY, viewWidth, viewHeight);

                        // check if ImGuizmo is hovered

                        glm::mat4 view = camera->view;
                        glm::mat4 projection = camera->projection;

                        auto &transform = selectedObject->GetComponent<Transform>();
                        transform.Update();
                        glm::mat4 transformMat = transform.transform;
                        glm::vec3 originalRot = transform.rotation;

                        ImGuizmo::Manipulate(glm::value_ptr(view), glm::value_ptr(projection),
                                             (ImGuizmo::OPERATION) m_GuizmoMode, ImGuizmo::WORLD,
                                             glm::value_ptr(transformMat));

                        if (ImGuizmo::IsOver()) {
                            usingImGuizmo = true;
                        } else {
                            usingImGuizmo = false;
                        }

                        if (ImGuizmo::IsUsing()) {

                            glm::vec3 translation, rotation, scale;
                            DecomposeTransform(transformMat, translation, rotation, scale);

//                            transform.rotation = rotation;
//                            transform.scale = scale;
                            switch(m_GuizmoMode) {
                                case ImGuizmo::OPERATION::TRANSLATE: {
                                    transform.position = translation;

                                    if(HyperAPI::isRunning) {
                                        if(selectedObject->HasComponent<Rigidbody2D>()) {
                                            auto &rigidbody = selectedObject->GetComponent<Rigidbody2D>();

                                            b2Body *body = (b2Body *) rigidbody.body;
                                            body->SetTransform(b2Vec2(translation.x, translation.y), body->GetAngle());
                                        }

                                        if(selectedObject->HasComponent<Rigidbody3D>()) {
                                            auto &rigidbody = selectedObject->GetComponent<Rigidbody3D>();

                                            btRigidBody *body = (btRigidBody *) rigidbody.body;
                                            body->getWorldTransform().setOrigin(btVector3(translation.x, translation.y, translation.z));
                                        }
                                    }
                                    break;
                                }
                                case ImGuizmo::OPERATION::ROTATE: {
                                    glm::vec3 deltaRot = rotation - originalRot;
                                    transform.rotation += deltaRot;

                                    if(HyperAPI::isRunning) {
                                        if(selectedObject->HasComponent<Rigidbody2D>()) {
                                            auto &rigidbody = selectedObject->GetComponent<Rigidbody2D>();

                                            b2Body *body = (b2Body *) rigidbody.body;
                                            body->SetTransform(body->GetPosition(), body->GetAngle() + glm::radians(deltaRot.z));
                                        }

                                        if(selectedObject->HasComponent<Rigidbody3D>()) {
                                            auto &rigidbody = selectedObject->GetComponent<Rigidbody3D>();

                                            btRigidBody *body = (btRigidBody *) rigidbody.body;
                                            btQuaternion rot = body->getWorldTransform().getRotation();
                                            rot.setEulerZYX(glm::radians(deltaRot.z), glm::radians(deltaRot.y), glm::radians(deltaRot.x));
                                            body->getWorldTransform().setRotation(rot);
                                        }
                                    }
                                    break;

                                }
                                case ImGuizmo::OPERATION::SCALE: {
                                    transform.scale = Vector3(scale.x * 2, scale.y * 2, scale.z * 2);
                                    break;
                                }
                            }
                        }
                    }
                    ImGui::EndChild();

                    ImGui::SetCursorPos(ImVec2(0, 0));
                    ImGui::Text("FPS: %d", fps);
                }
                ImGui::End();

                if (ImGui::Begin(ICON_FA_CUBES " Hierarchy")) {
                    if (ImGui::IsWindowHovered() && ImGui::IsMouseDoubleClicked(0)) {
                        Scene::m_Object = nullptr;
                    }

                    Scene::DropTargetMat(Scene::DRAG_MODEL, nullptr, nullptr);
                    ImVec2 win_size = ImGui::GetWindowSize();

                    if (ImGui::Button(ICON_FA_PLUS " Add GameObject", ImVec2(win_size.x, 0))) {
                        GameObject *go = new GameObject();
                        go->AddComponent<Transform>();
                        Scene::m_GameObjects.push_back(go);
                    }

                    for (int i = 0; i < Scene::m_GameObjects.size(); i++) {
                        if (Scene::m_GameObjects[i]->parentID != "NO_PARENT") continue;
                        Scene::m_GameObjects[i]->GUI();
                    }

                }
                ImGui::End();

                if (ImGui::Begin(ICON_FA_SHARE_NODES " Components")) {
                    if (Scene::m_Object != nullptr && !Scene::LoadingScene) {
                        if (ImGui::IsWindowFocused()) {
#ifndef _WIN32 || GAME_BUILD
                            UpdatePresence(
                                    "In Editor",
                                    "Editing " + Scene::m_Object->name
                            );
#endif
                        }
                        ImGui::InputText("Name", Scene::name, 500);
                        ImGui::InputText("Tag", Scene::tag, 500);
                        // layer items
                        std::vector<const char *> layerItems;
                        for (auto &layer : Scene::layers) {
                            layerItems.push_back(layer.first.c_str());
                        }

                        if (ImGui::BeginCombo("Layer", Scene::m_Object->layer.c_str())) {
                            for (int i = 0; i < layerItems.size(); i++) {
                                bool isSelected = (Scene::layer == layerItems[i]);
                                if (ImGui::Selectable(layerItems[i], isSelected)) {
                                    Scene::m_Object->layer = layerItems[i];
                                    strcpy(Scene::layer, layerItems[i]);
                                }
                                if (isSelected) {
                                    ImGui::SetItemDefaultFocus();
                                }
                            }
                            ImGui::EndCombo();
                        }

                        Scene::m_Object->tag = Scene::tag;
                        Scene::m_Object->name = Scene::name;
                        Scene::m_Object->layer = Scene::layer;

                        if (Scene::m_Object->HasComponent<Transform>()) {
                            auto &comp = Scene::m_Object->GetComponent<Transform>();
                            if (comp.hasGUI) comp.GUI();

                            if(Scene::m_Object->HasComponent<Rigidbody2D>() && HyperAPI::isRunning) {
                                auto &rigidbody = Scene::m_Object->GetComponent<Rigidbody2D>();
                                b2Body *body = (b2Body *) rigidbody.body;
                                body->SetTransform(b2Vec2(comp.position.x, comp.position.y), body->GetAngle());
                            }

                            if(Scene::m_Object->HasComponent<Rigidbody3D>() && HyperAPI::isRunning) {
                                auto &rigidbody = Scene::m_Object->GetComponent<Rigidbody3D>();
                                btRigidBody *body = (btRigidBody *) rigidbody.body;
                                body->getWorldTransform().setOrigin(btVector3(comp.position.x, comp.position.y, comp.position.z));
                            }
                        }

                        if (Scene::m_Object->HasComponent<CameraComponent>()) {
                            auto &comp = Scene::m_Object->GetComponent<CameraComponent>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<MeshRenderer>()) {
                            auto &comp = Scene::m_Object->GetComponent<MeshRenderer>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<m_LuaScriptComponent>()) {
                            auto &comp = Scene::m_Object->GetComponent<m_LuaScriptComponent>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<c_PointLight>()) {
                            auto &comp = Scene::m_Object->GetComponent<c_PointLight>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<c_Light2D>()) {
                            auto &comp = Scene::m_Object->GetComponent<c_Light2D>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<c_SpotLight>()) {
                            auto &comp = Scene::m_Object->GetComponent<c_SpotLight>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<c_DirectionalLight>()) {
                            auto &comp = Scene::m_Object->GetComponent<c_DirectionalLight>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<SpriteRenderer>()) {
                            auto &comp = Scene::m_Object->GetComponent<SpriteRenderer>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<SpriteAnimation>()) {
                            auto &comp = Scene::m_Object->GetComponent<SpriteAnimation>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<c_SpritesheetAnimation>()) {
                            auto &comp = Scene::m_Object->GetComponent<c_SpritesheetAnimation>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<SpritesheetRenderer>()) {
                            auto &comp = Scene::m_Object->GetComponent<SpritesheetRenderer>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<BoxCollider2D>()) {
                            auto &comp = Scene::m_Object->GetComponent<BoxCollider2D>();
                            if (comp.hasGUI) {
                                comp.GUI();
                                if (ImGui::IsWindowFocused()) {
                                    drawBoxCollider2D = true;
                                }
                                auto &transform = Scene::m_Object->GetComponent<Transform>();
                                bc2dPos = transform.position;
                                bc2dRotation = transform.rotation.z;
                                bc2dScale = Vector3(comp.size.x, comp.size.y, 1);
                            }
                        } else {
                            drawBoxCollider2D = false;
                        }

                        if (Scene::m_Object->HasComponent<Rigidbody2D>()) {
                            auto &comp = Scene::m_Object->GetComponent<Rigidbody2D>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<Rigidbody3D>()) {
                            auto &comp = Scene::m_Object->GetComponent<Rigidbody3D>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<FixedJoint3D>()) {
                            auto &comp = Scene::m_Object->GetComponent<FixedJoint3D>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if (Scene::m_Object->HasComponent<BoxCollider3D>()) {
                            auto &comp = Scene::m_Object->GetComponent<BoxCollider3D>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        if(Scene::m_Object->HasComponent<MeshCollider3D>()) {
                            auto &comp = Scene::m_Object->GetComponent<MeshCollider3D>();
                            if (comp.hasGUI) comp.GUI();
                        }

                        ImGui::Separator();

                        ImVec2 win_size = ImGui::GetWindowSize();
                        if (!HyperAPI::isRunning) {
                            if (ImGui::Button(ICON_FA_PLUS " Add Component", ImVec2(win_size.x, 0))) {
                                ImGui::OpenPopup("Add Component");
                            }
                        }
                    }

                    if (ImGui::BeginPopup("Add Component")) {
                        if (ImGui::Button("Transform", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<Transform>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Mesh Renderer", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<MeshRenderer>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Lua Scripts", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<m_LuaScriptComponent>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Camera", ImVec2(200, 0))) {
                            // CameraComponent has one argument of type entt::entity
                            Scene::m_Object->AddComponent<CameraComponent>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Point Light", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<c_PointLight>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Spot Light", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<c_SpotLight>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Directional Light", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<c_DirectionalLight>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("2D Light", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<c_Light2D>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Sprite Renderer", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<SpriteRenderer>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Sprite Animation", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<SpriteAnimation>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Spritesheet Renderer", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<SpritesheetRenderer>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Spritesheet Animation", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<c_SpritesheetAnimation>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Rigidbody 2D", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<Rigidbody2D>();
                            ImGui::CloseCurrentPopup();
                        }


                        if (ImGui::Button("Box Collider 2D", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<BoxCollider2D>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Rigidbody 3D", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<Rigidbody3D>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Fixed Joint 3D", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<FixedJoint3D>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Box Collider 3D", ImVec2(200, 0))) {
                            Scene::m_Object->AddComponent<BoxCollider3D>();
                            ImGui::CloseCurrentPopup();
                        }

                        if (ImGui::Button("Mesh Collider 3D", ImVec2(200, 0))) {
                            if(Scene::m_Object->HasComponent<MeshRenderer>()) {
                                Scene::m_Object->AddComponent<MeshCollider3D>();
                                ImGui::CloseCurrentPopup();
                            }
                        }

                        ImGui::EndPopup();
                    }

                }
                ImGui::End();

                if (ImGui::Begin(ICON_FA_FOLDER " Assets", nullptr)) {
                    Scene::DropTargetMat(Scene::DRAG_GAMEOBJECT, nullptr, nullptr);
                    DirIter(cwd + std::string("/assets"));
                }
                ImGui::End();

                if (ImGui::Begin(ICON_FA_TERMINAL " Console")) {
                    if (ImGui::Button(ICON_FA_TRASH " Clear")) {
                        Scene::logs.clear();
                    }

                    ImGui::Separator();

                    for (auto &log : Scene::logs) {
                        log.GUI();
                    }

                }
                ImGui::End();

                if (ImGuiFileDialog::Instance()->Display("BuildWindowsDialog")) {
                    // action if OK
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
#ifdef _WIN32
                        system(("rmdir /s /q " + filePathName).c_str());
                        system(("mkdir " + filePathName).c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\assets\" \"" + filePathName + "\\assets\\\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\build\" \"" + filePathName + "\\build\\\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\shaders\" \"" + filePathName + "\\shaders\\\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\dlls\\*.dll\" \"" + filePathName + "\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\game.exe\" \"" + filePathName + "\"").c_str());
                        // rename game.exe to game name
                        system(("rename " + filePathName + "\\game.exe " + config.name + ".exe").c_str());
#else
                        system(std::string("rm -r \"" + filePathName + "\"").c_str());
                        system(std::string("mkdir \"" + filePathName + "\"").c_str());
                        system(std::string("cp assets \"" + filePathName + "/assets\" -r").c_str());
                        system(std::string("cp build \"" + filePathName + "/build\" -r").c_str());
                        system(std::string("cp shaders \"" + filePathName + "/shaders\" -r").c_str());
                        system(std::string("cp dlls/*.dll \"" + filePathName + "\"").c_str());
                        system(std::string("cp game.exe \"" + filePathName + "/" + config.name + ".exe\"").c_str());
#endif

                        // fs::copy(cwd + "/assets", filePathName + "/assets", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        // fs::copy(cwd + "/build", filePathName + "/build", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        // fs::copy(cwd + "/shaders", filePathName + "/shaders", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        // copy all files that start with *.dll AND THEY ARE NOT IN LIB FOLDER
                        // fs::copy(cwd + "/dlls", filePathName, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        // fs::copy(cwd + "/platforms/dist/windows/Game.exe", filePathName + "/" + config.name + ".exe", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                    }


                    // close
                    ImGuiFileDialog::Instance()->Close();
                }

                if (ImGuiFileDialog::Instance()->Display("BuildLinuxDialog")) {
                    // action if OK
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                        std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

#ifdef _WIN32
                        system(("rmdir /s /q " + filePathName).c_str());
                        system(("mkdir " + filePathName).c_str());
                        // mark xcopy as directory
                        system(("xcopy /s /e /y  \"" + cwd + "\\assets\" \"" + filePathName + "\\assets\\\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\build\" \"" + filePathName + "\\build\\\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\shaders\" \"" + filePathName + "\\shaders\\\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\lib\" \"" + filePathName + "\\lib\\\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\game.out\" \"" + filePathName + "\"").c_str());
                        system(("xcopy /s /e /y \"" + cwd + "\\LaunchGame.sh\" \"" + filePathName + "\"").c_str());
                        system(("rename \"" + filePathName + "\\LaunchGame.sh\" \"" + config.name + ".sh\"").c_str());
#else
                        system(std::string("rm -r \"" + filePathName + "\"").c_str());
                        system(std::string("mkdir \"" + filePathName + "\"").c_str());
                        system(std::string("cp assets \"" + filePathName + "/assets\" -r").c_str());
                        system(std::string("cp build \"" + filePathName + "/build\" -r").c_str());
                        system(std::string("cp shaders \"" + filePathName + "/shaders\" -r").c_str());
                        system(std::string("cp lib \"" + filePathName + "/lib\" -r").c_str());
                        system(std::string("cp game.out \"" + filePathName + "/game.out\"").c_str());
                        system(std::string("cp LaunchGame.sh \"" + filePathName + "/" + config.name + ".sh\"").c_str());
#endif

                        // fs::copy(cwd + "/assets", filePathName + "/assets", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        // fs::copy(cwd + "/build", filePathName + "/build", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        // fs::copy(cwd + "/shaders", filePathName + "/shaders", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        // copy all files that start with *.dll AND THEY ARE NOT IN LIB FOLDER
                        // fs::copy(cwd + "/dlls", filePathName, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                        // fs::copy(cwd + "/platforms/dist/windows/Game.exe", filePathName + "/" + config.name + ".exe", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                    }


                    // close
                    ImGuiFileDialog::Instance()->Close();
                }
#endif
            };

    bool calledOnce = false;

    // batchShader.Bind();
    // batchShader.SetUniform1i("textures[0]", 0);
    // batchShader.SetUniform1i("textures[1]", 1);
    // batchShader.SetUniform1i("cubeMap", 10);

    app.Run([&] {
        if(Scene::LoadingScene) return;

        if (HyperAPI::isRunning) {
            BulletPhysicsWorld::UpdatePhysics();
            BulletPhysicsWorld::CollisionCallback([&](const std::string &idA, const std::string &idB) {
                auto *gameObjectA = f_GameObject::FindGameObjectByID(idA);
                auto *gameObjectB = f_GameObject::FindGameObjectByID(idB);

                if (gameObjectA->HasComponent<NativeScriptManager>()) {
                    auto &scriptManager = gameObjectA->GetComponent<NativeScriptManager>();
                    for (auto script : scriptManager.m_StaticScripts) {
                        script->Collision3D(gameObjectB);
                    }
                }

                if (gameObjectB->HasComponent<NativeScriptManager>()) {
                    auto &scriptManager = gameObjectB->GetComponent<NativeScriptManager>();
                    for (auto script : scriptManager.m_StaticScripts) {
                        script->Collision3D(gameObjectA);
                    }
                }

                if (gameObjectA->HasComponent<m_LuaScriptComponent>()) {
                    auto &scriptManager = gameObjectA->GetComponent<m_LuaScriptComponent>();
                    for (auto script : scriptManager.scripts) {
                        script.Collision3D(gameObjectB);
                    }
                }

                if (gameObjectB->HasComponent<m_LuaScriptComponent>()) {
                    auto &scriptManager = gameObjectB->GetComponent<m_LuaScriptComponent>();
                    for (auto script : scriptManager.scripts) {
                        script.Collision3D(gameObjectA);
                    }
                }
            });

#ifndef _WIN32 || GAME_BUILD
            UpdatePresence(
                    "In Editor",
                    "Playtesting " + std::string(config.name)
            );
#endif
        }

        picker.projectionMatrix = camera->projection;
        picker.mouseX = mousePos.x;
        picker.mouseY = mousePos.y;
        picker.winX = app.height;
        picker.winY = app.width;

        // calculate FPS, deltaTime is in Timestep::deltaTime
        frameCount++;
        timeElapsed += Timestep::deltaTime;
        if (timeElapsed >= 1.0f) {
            fps = frameCount;
            frameCount = 0;
            timeElapsed -= 1.0f;

            fpsText = "FPS: " + std::to_string(fps);
        }

        // glfwGetWindowSize(app.renderer->window, &picker.winX, &picker.winY);

        // quadBatch2.Update();
        // quadBatch.Update();
        // texture.Bind(0);
        // texture2.Bind(1);

        // batchShader.Bind();
        // batchShader.SetUniform1f("ambient", config.ambientLight);
        // TransformComponent cameraTransform = camera->GetComponent<TransformComponent>();
        // batchShader.SetUniform3f("cameraPosition", cameraTransform.position.x, cameraTransform.position.y, cameraTransform.position.z);
        // batchLayer.Draw(batchShader, *camera);

        // texture.Unbind();
        // texture2.Unbind();

        if (Scene::mainCamera == nullptr) {
            Scene::mainCamera = camera;
        }

        if (!calledOnce) {
            auto &colors = ImGui::GetStyle().Colors;
            colors[ImGuiCol_WindowBg] = ImVec4(0.1f, 0.105f, 0.11f, 1.0f);

            colors[ImGuiCol_Header] = ImVec4(0.2f, 0.205f, 0.2f, 1.0f);
            colors[ImGuiCol_HeaderHovered] = ImVec4(0.3f, 0.305f, 0.3f, 1.0f);
            colors[ImGuiCol_HeaderActive] = ImVec4(0.15f, 0.1505f, 0.15f, 1.0f);

            colors[ImGuiCol_Button] = ImVec4(0.6f, 0.2f, 0.2f, 1.0f);
            colors[ImGuiCol_ButtonHovered] = ImVec4(1, 0.205f, 0.2f, 1.0f);
            colors[ImGuiCol_ButtonActive] = ImVec4(1, 0.305f, 0.3f, 1.0f);

            colors[ImGuiCol_FrameBg] = ImVec4(0.2f, 0.205f, 0.2f, 1.0f);
            colors[ImGuiCol_FrameBgHovered] = ImVec4(0.3, 0.305f, 0.3f, 1.0f);
            colors[ImGuiCol_FrameBgActive] = ImVec4(0.15f, 0.1505f, 0.15f, 1.0f);

            colors[ImGuiCol_Tab] = ImVec4(0.2f, 0.205f, 0.2f, 1.0f);
            colors[ImGuiCol_TabHovered] = ImVec4(0.3f, 0.305f, 0.3f, 1.0f);
            colors[ImGuiCol_TabActive] = ImVec4(0.15f, 0.1505f, 0.15f, 1.0f);
            colors[ImGuiCol_TabUnfocused] = ImVec4(0.2f, 0.205f, 0.2f, 1.0f);
            colors[ImGuiCol_TabUnfocusedActive] = ImVec4(0.15f, 0.1505f, 0.15f, 1.0f);

            colors[ImGuiCol_TitleBg] = ImVec4(0.2f, 0.205f, 0.2f, 1.0f);
            colors[ImGuiCol_TitleBgActive] = ImVec4(0.25f, 0.255f, 0.25f, 1.0f);
            colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.15f, 0.1505f, 0.15f, 1.0f);

            colors[ImGuiCol_ResizeGrip] = ImVec4(1, 0.15, 0.15, 1);
            colors[ImGuiCol_ResizeGripActive] = ImVec4(1, 0.30, 0.30, 1);
            colors[ImGuiCol_ResizeGripHovered] = ImVec4(1, 0.20, 0.20, 1);
            colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1, 0.15, 0.15, 1);

            colors[ImGuiCol_DockingPreview] = ImVec4(1, 0.15, 0.15, 1);

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.5, 2.5));
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        }

        calledOnce = true;

        if (focusedOnScene && !usingImGuizmo && !camera->mode2D && Scene::mainCamera == camera) {
            camera->Inputs(app.renderer->window, winPos);
        }
        // Input::winPos = Vector3(winPos.x, winPos.y, 0);
        Input::winSize = Vector3(app.width, app.height, 0);

        if (hoveredScene && !usingImGuizmo && camera->mode2D && Scene::mainCamera == camera) {
            auto transform = camera->GetComponent<TransformComponent>();
            transform.rotation = glm::vec3(0.0f, 0.0f, -1.0f);
            camera->Inputs(app.renderer->window, winPos);
            camera->UpdateComponent(transform);
        }
        if (hoveredScene && !usingImGuizmo && Scene::mainCamera == camera) {
            camera->Inputs(app.renderer->window, winPos);
        }
        winSize = Vector2(app.width, app.height);

        for (auto &camera : Scene::cameras) {
            camera->cursorWinW = app.winWidth;
            camera->cursorWinH = app.winHeight;

            camera->updateMatrix(camera->cam_fov, camera->cam_near, camera->cam_far,
                                 Vector2(app.winWidth, app.winHeight), winSize);

            if(camera->m_MouseMovement && HyperAPI::isRunning) {
#ifdef GAME_BUILD
                camera->MouseMovement(winPos);
#else
                if(hoveredScene) {
                    camera->MouseMovement(winPos);
                }
#endif
            }
        }
        camera->cursorWinW = winSize.x;
        camera->cursorWinH = winSize.y;
        camera->updateMatrix(camera->cam_fov, camera->cam_near, camera->cam_far, Vector2(app.winWidth, app.winHeight), winSize);
        skybox.Draw(*Scene::mainCamera, winSize.x, winSize.y);
        // floor.Draw(shader, *camera);
        shader.Bind();
        shader.SetUniform1f("ambient", config.ambientLight);
        shader.SetUniform1i("shadowMap", 17);
        shader.SetUniformMat4("lightProjection", Scene::projection);

        spriteShader.Bind();
        spriteShader.SetUniform1f("ambient", config.ambientLight);

        // Physics

        if (HyperAPI::isRunning && Scene::world != nullptr) {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            Scene::world->Step(1.0f / 60.0f, velocityIterations, positionIterations);

            auto view = Scene::m_Registry.view<Rigidbody2D>();
            for (auto e : view) {
                GameObject *m_GameObject;
                for (auto &gameObject : Scene::m_GameObjects) {
                    if (gameObject->entity == e) {
                        m_GameObject = gameObject;
                    }
                }

                auto &transform = m_GameObject->GetComponent<Transform>();
                auto &rigidbody = m_GameObject->GetComponent<Rigidbody2D>();

                b2Body *body = (b2Body *) rigidbody.body;
                const auto &position = body->GetPosition();
                transform.position.x = position.x;
                transform.position.y = position.y;
                transform.rotation.z = glm::degrees(body->GetAngle());
            }
        }

        if(bulletPhysicsStarted) {
            auto view = Scene::m_Registry.view<Rigidbody3D>();

            for(auto e : view) {
                GameObject *m_GameObject;

                for(auto &obj : Scene::m_GameObjects) {
                    if(obj->entity == e) {
                        m_GameObject = obj;
                    }
                }

                auto &rigidbody = m_GameObject->GetComponent<Rigidbody3D>();
                rigidbody.transform = &m_GameObject->GetComponent<Transform>();
                rigidbody.Update();
            }
        }

        for (auto &gameObject : Scene::m_GameObjects) {
            if (!gameObject) continue;
            gameObject->Update();

            if (gameObject->HasComponent<Transform>()) {
                gameObject->GetComponent<Transform>().Update();
            }

            if (gameObject->HasComponent<m_LuaScriptComponent>()) {
                gameObject->GetComponent<m_LuaScriptComponent>().Update();
            }

            if (gameObject->HasComponent<c_PointLight>()) {
                gameObject->GetComponent<c_PointLight>().Update();
            }

            if (gameObject->HasComponent<c_Light2D>()) {
                gameObject->GetComponent<c_Light2D>().Update();
            }

            if (gameObject->HasComponent<c_SpotLight>()) {
                gameObject->GetComponent<c_SpotLight>().Update();
            }

            if (gameObject->HasComponent<c_DirectionalLight>()) {
                gameObject->GetComponent<c_DirectionalLight>().Update();
            }

            if (gameObject->HasComponent<m_LuaScriptComponent>()) {
                auto &script = gameObject->GetComponent<m_LuaScriptComponent>();
                if (HyperAPI::isRunning) {
                    script.Update();
                }
            }

            if (gameObject->HasComponent<NativeScriptManager>()) {
                auto &script = gameObject->GetComponent<NativeScriptManager>();
                if (HyperAPI::isRunning) {
                    script.Update();
                }
            }
        }

        for (auto &layer : Scene::layers) {
            for (auto &gameObject : Scene::m_GameObjects) {
                if (gameObject->layer != layer.first) continue;

                if (gameObject->HasComponent<MeshRenderer>()) {
                    // if(gameObject->GetComponcent<MeshRenderer>().m_Model) continue;

                    auto meshRenderer = gameObject->GetComponent<MeshRenderer>();
                    auto transform = gameObject->GetComponent<Transform>();
                    transform.Update();

                    glm::mat4 extra = meshRenderer.extraMatrix;

                    if (meshRenderer.m_Mesh != nullptr) {
                        if (transform.parentTransform != nullptr) {
                            transform.parentTransform->Update();
                            if(Scene::m_Object == gameObject) {
                                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                                glStencilMask(0xFF);
                            }
                            meshRenderer.m_Mesh->Draw(shader, *Scene::mainCamera,
                                                      transform.transform * transform.parentTransform->transform *
                                                      extra);
                        } else {
                            if(Scene::m_Object == gameObject) {
                                glStencilFunc(GL_ALWAYS, 1, 0xFF);
                                glStencilMask(0xFF);
                            }
                            meshRenderer.m_Mesh->Draw(shader, *Scene::mainCamera, transform.transform * extra);
                        }
                    }
                }

                if (gameObject->HasComponent<SpriteRenderer>()) {
                    auto spriteRenderer = gameObject->GetComponent<SpriteRenderer>();
                    auto transform = gameObject->GetComponent<Transform>();
                    transform.Update();

                    spriteRenderer.mesh->Draw(shader, *Scene::mainCamera, transform.transform);
                }

                if (gameObject->HasComponent<SpritesheetRenderer>()) {
                    auto spritesheetRenderer = gameObject->GetComponent<SpritesheetRenderer>();
                    auto transform = gameObject->GetComponent<Transform>();
                    transform.Update();

                    if (spritesheetRenderer.mesh != nullptr) {

                        spritesheetRenderer.mesh->Draw(shader, *Scene::mainCamera, transform.transform);
                    }
                }

                if (gameObject->HasComponent<SpriteAnimation>()) {
                    auto spriteAnimation = gameObject->GetComponent<SpriteAnimation>();
                    auto transform = gameObject->GetComponent<Transform>();
                    transform.Update();

                    spriteAnimation.Play();
                    if (spriteAnimation.currMesh != nullptr) {
                        spriteAnimation.currMesh->Draw(shader, *Scene::mainCamera, transform.transform);
                    }
                }

                if (gameObject->HasComponent<c_SpritesheetAnimation>()) {
                    auto spritesheetAnimation = gameObject->GetComponent<c_SpritesheetAnimation>();
                    auto transform = gameObject->GetComponent<Transform>();
                    transform.Update();

                    spritesheetAnimation.Play();
                    if (spritesheetAnimation.currMesh != nullptr) {
                        spritesheetAnimation.currMesh->Draw(shader, *Scene::mainCamera, transform.transform);
                    }
                }
            }
        }
#ifndef GAME_BUILD
        if(Scene::mainCamera != camera) return;
        glClear(GL_DEPTH_BUFFER_BIT);
        if (drawBoxCollider2D) {
            glDepthFunc(GL_LEQUAL);
            glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

            glm::mat4 model = glm::mat4(1.0f);
            model = glm::translate(model, bc2dPos) *
                    glm::scale(model, glm::vec3(bc2dScale.x / 2, bc2dScale.y / 2, 1.0f)) *
                    glm::rotate(model, glm::radians(90.0f), glm::vec3(1.0f, 0.0f, 0.0f));
            mesh_BoxCollider2D.Draw(workerShader, *camera, model);

            glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
        }

        for(auto &gameObject : Scene::m_GameObjects) {
            if(gameObject->HasComponent<c_DirectionalLight>()) {
                for(auto &game_object : Scene::m_GameObjects) {
                    if(game_object->HasComponent<MeshRenderer>()) {
                        auto &meshRenderer = game_object->GetComponent<MeshRenderer>();
                        if(meshRenderer.m_Mesh)
                            meshRenderer.m_Mesh->material.Unbind(shader);
                    }
                }

                auto &transform = gameObject->GetComponent<Transform>();
                Transform t = transform;
                t.scale = glm::vec3(-1.5f, 1.5f, 1.5f);

                auto camTransform = camera->GetComponent<TransformComponent>();
                t.LookAt(camTransform.position);
                t.Update();

                glDepthFunc(GL_LEQUAL);
                dirLightIconMesh.Draw(workerShader, *camera, t.transform);
            }

            if(gameObject->HasComponent<c_DirectionalLight>()) {
                for(auto &game_object : Scene::m_GameObjects) {
                    if(game_object->HasComponent<MeshRenderer>()) {
                        auto &meshRenderer = game_object->GetComponent<MeshRenderer>();
                        if(meshRenderer.m_Mesh)
                            meshRenderer.m_Mesh->material.Unbind(shader);
                    }
                }

                auto &transform = gameObject->GetComponent<Transform>();
                Transform t = transform;
                t.scale = glm::vec3(-1.5f, 1.5f, 1.5f);

                auto camTransform = camera->GetComponent<TransformComponent>();
                t.LookAt(camTransform.position);
                t.Update();

                glDepthFunc(GL_LEQUAL);
                dirLightIconMesh.Draw(workerShader, *camera, t.transform);
            }

            if(gameObject->HasComponent<c_PointLight>()) {
                for(auto &game_object : Scene::m_GameObjects) {
                    if(game_object->HasComponent<MeshRenderer>()) {
                        auto &meshRenderer = game_object->GetComponent<MeshRenderer>();
                        if(meshRenderer.m_Mesh)
                            meshRenderer.m_Mesh->material.Unbind(shader);
                    }
                }

                auto &transform = gameObject->GetComponent<Transform>();
                Transform t = transform;
                t.scale = glm::vec3(-1.5f, 1.5f, 1.5f);

                auto camTransform = camera->GetComponent<TransformComponent>();
                t.LookAt(camTransform.position);
                t.Update();

                glDepthFunc(GL_LEQUAL);
                pointLightIconMesh.Draw(workerShader, *camera, t.transform);
            }

            if(gameObject->HasComponent<c_SpotLight>()) {
                for(auto &game_object : Scene::m_GameObjects) {
                    if(game_object->HasComponent<MeshRenderer>()) {
                        auto &meshRenderer = game_object->GetComponent<MeshRenderer>();
                        if(meshRenderer.m_Mesh)
                            meshRenderer.m_Mesh->material.Unbind(shader);
                    }
                }

                auto &transform = gameObject->GetComponent<Transform>();
                Transform t = transform;
                t.scale = glm::vec3(-1.5f, 1.5f, 1.5f);

                auto camTransform = camera->GetComponent<TransformComponent>();
                t.LookAt(camTransform.position);
                t.Update();

                glDepthFunc(GL_LEQUAL);
                spotLightIconMesh.Draw(workerShader, *camera, t.transform);
            }

            if(gameObject->HasComponent<CameraComponent>()) {
                for(auto &game_object : Scene::m_GameObjects) {
                    if(game_object->HasComponent<MeshRenderer>()) {
                        auto &meshRenderer = game_object->GetComponent<MeshRenderer>();
                        if(meshRenderer.m_Mesh)
                            meshRenderer.m_Mesh->material.Unbind(shader);
                    }
                }

                auto &transform = gameObject->GetComponent<Transform>();
                Transform t = transform;
                t.scale = glm::vec3(-1.5f, 1.5f, 1.5f);

                auto camTransform = camera->GetComponent<TransformComponent>();
                t.LookAt(camTransform.position);
                t.Update();

                glDepthFunc(GL_LEQUAL);
                cameraIconMesh.Draw(workerShader, *camera, t.transform);
            }
        }
#endif
    }, GUI_EXP);

    // shutdown discord-rpc
#ifndef _WIN32 || GAME_BUILD
    Discord_Shutdown();
#endif

    // kill program
    exit(0);

    return 0;
}