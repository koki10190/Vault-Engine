
#include <iostream>
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

class CollisionListener : public b2ContactListener {
public:
    void BeginContact(b2Contact *contact) {
        b2Fixture *fixtureA = contact->GetFixtureA();
        b2Fixture *fixtureB = contact->GetFixtureB();
        // getuser data
        b2BodyUserData &bodyUserDataA = fixtureA->GetBody()->GetUserData();
        b2BodyUserData &bodyUserDataB = fixtureB->GetBody()->GetUserData();

        Experimental::GameObject *gameObjectA = (Experimental::GameObject *)bodyUserDataA.pointer;
        Experimental::GameObject *gameObjectB = (Experimental::GameObject *)bodyUserDataB.pointer;

        if(gameObjectA->HasComponent<Experimental::NativeScriptManager>()) {
            auto &scriptManager = gameObjectA->GetComponent<Experimental::NativeScriptManager>();
            for(auto script : scriptManager.m_StaticScripts) {
                script->Collision2D(gameObjectB);
            }
        }

        if(gameObjectB->HasComponent<Experimental::NativeScriptManager>()) {
            auto &scriptManager = gameObjectB->GetComponent<Experimental::NativeScriptManager>();
            for(auto script : scriptManager.m_StaticScripts) {
                script->Collision2D(gameObjectA);
            }
        }

        if(gameObjectA->HasComponent<Experimental::m_LuaScriptComponent>()) {
            auto &scriptManager = gameObjectA->GetComponent<Experimental::m_LuaScriptComponent>();
            for(auto script : scriptManager.scripts) {
                script.Collision2D(gameObjectB);
            }
        }

        if(gameObjectB->HasComponent<Experimental::m_LuaScriptComponent>()) {
            auto &scriptManager = gameObjectB->GetComponent<Experimental::m_LuaScriptComponent>();
            for(auto script : scriptManager.scripts) {
                script.Collision2D(gameObjectA);
            }
        }
    }

    void EndContact(b2Contact *contact) {
        b2Fixture *fixtureA = contact->GetFixtureA();
        b2Fixture *fixtureB = contact->GetFixtureB();
        // getuser data
        b2BodyUserData &bodyUserDataA = fixtureA->GetBody()->GetUserData();
        b2BodyUserData &bodyUserDataB = fixtureB->GetBody()->GetUserData();

        Experimental::GameObject *gameObjectA = (Experimental::GameObject *)bodyUserDataA.pointer;
        Experimental::GameObject *gameObjectB = (Experimental::GameObject *)bodyUserDataB.pointer;

        if(gameObjectA->HasComponent<Experimental::NativeScriptManager>()) {
            auto &scriptManager = gameObjectA->GetComponent<Experimental::NativeScriptManager>();
            for(auto script : scriptManager.m_StaticScripts) {
                script->CollisionExit2D(gameObjectB);
            }
        }

        if(gameObjectB->HasComponent<Experimental::NativeScriptManager>()) {
            auto &scriptManager = gameObjectB->GetComponent<Experimental::NativeScriptManager>();
            for(auto script : scriptManager.m_StaticScripts) {
                script->CollisionExit2D(gameObjectA);
            }
        }

        if(gameObjectA->HasComponent<Experimental::m_LuaScriptComponent>()) {
            auto &scriptManager = gameObjectA->GetComponent<Experimental::m_LuaScriptComponent>();
            for(auto script : scriptManager.scripts) {
                script.CollisionExit2D(gameObjectB);
            }
        }

        if(gameObjectB->HasComponent<Experimental::m_LuaScriptComponent>()) {
            auto &scriptManager = gameObjectB->GetComponent<Experimental::m_LuaScriptComponent>();
            for(auto script : scriptManager.scripts) {
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
    int width, height;
};

struct AddComponentList {
    int selected = 0;
    int length = 1;
    bool showed;
    const char* items[1] = {
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

bool ends_with(std::string const & value, std::string const & ending)
{
    if (ending.size() > value.size()) return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void DirIter(std::string path) {
    for (const auto & entry : fs::directory_iterator(path)) {
        if(fs::is_directory(entry)) {
            if(ImGui::TreeNodeEx((std::string(ICON_FA_FOLDER) + " " + entry.path().filename().string()).c_str(), ImGuiTreeNodeFlags_SpanAvailWidth)) {
                DirIter(entry.path().string());
                ImGui::TreePop();
            }
        } else {
            // offset for the arrow thing
            ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 25);
            ImGui::Selectable((std::string(ICON_FA_FILE) + " " + entry.path().filename().string()).c_str());
            // make it draggable
            if(ImGui::BeginDragDropSource(ImGuiDragDropFlags_None)) {
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

    if(JSON[i]["material"]["diffuse"] != "nullptr") {
        std::string diffusePath = JSON[i]["material"]["diffuse"];
        material.diffuse = new Texture((char*)diffusePath.c_str(), 0, "texture_diffuse");
    }

    if(JSON[i]["material"]["specular"] != "nullptr") {
        std::string specularPath = JSON[i]["material"]["specular"];
        material.specular = new Texture((char*)specularPath.c_str(), 1, "texture_specular");
    }

    if(JSON[i]["material"]["normal"] != "nullptr") {
        std::string normalPath = JSON[i]["material"]["normal"];
        material.normal = new Texture((char*)normalPath.c_str(), 2, "texture_normal");
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

int main() {
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

    if(file.is_open()) {
        nlohmann::json JSON = nlohmann::json::parse(file);

        config.width = JSON["width"];
        config.height = JSON["height"];
        strcpy(config.name, ((std::string)JSON["name"]).c_str());
        config.ambientLight = JSON["ambientLight"];
        config.mainScene = JSON["mainScene"];
        mainSceneFound = true;
    } else {
        nlohmann::json j = {
            {"name", config.name},
            {"ambientLight", config.ambientLight},
            {"mainScene", config.mainScene},
            {"width", config.width},
            {"height", config.height}
        };

        std::ofstream o("assets/game.config");
        o << std::setw(4) << j << std::endl;
    }

    Hyper::Application app(1280, 720, "Static Engine");
    Input::window = app.renderer->window;
    // glfw enable sticky mouse buttons
    Shader shader("shaders/default.glsl");
    Shader spriteShader("shaders/sprite.glsl");
    Shader batchShader("shaders/batch.glsl");

    spriteShader.Bind();
    spriteShader.SetUniform1f("ambient", 1);

    shader.Bind();
    shader.SetUniform1f("ambient", 0.2);

    Skybox skybox("assets/skybox/right.jpg", "assets/skybox/left.jpg", "assets/skybox/top.jpg", "assets/skybox/bottom.jpg", "assets/skybox/front.jpg", "assets/skybox/back.jpg");
    Camera *camera = new Camera(false, app.width, app.height, Vector3(0,3,15));
    Scene::mainCamera = camera;

    bool focusedOnScene = false;
    bool hoveredScene = false;

    Vector2 winSize = Vector2(0,0);
    Vector2 winPos = Vector2(0,0);

    ScriptEngine::window = app.renderer->window;

    Mesh *currEntity = nullptr;
    PointLight *currPointLight = nullptr;
    SpotLight *currSpotLight = nullptr;
    Camera *selectedCamera = nullptr;
    DirectionalLight *currDirectionalLight = nullptr;
    int currModel = -1;

    bool openConfig = false;
    bool openDetails = false;
    bool openInspector = false;

    int inspectorType = InspecType::None;

    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeFile, "", ImVec4(1,1,1, 1.0f), ICON_FA_FILE);
    ImGuiFileDialog::Instance()->SetFileStyle(IGFD_FileStyleByTypeDir, "", ImVec4(1,1,1, 1.0f), ICON_FA_FOLDER);

    InspectorMaterial m_InspectorMaterial;

    Scene::LoadScene(config.mainScene);
    Scene::mainCamera = camera;

    std::function<void(unsigned int &PPT, unsigned int &PPFBO)> GUI_EXP = 
    [&](unsigned int &PPT, unsigned int &PPFBO) {
        if(ImGui::BeginMainMenuBar()) {
            if(ImGui::BeginMenu("File")) {
                if(ImGui::MenuItem("Save Scene", "CTRL+S")) {
                    if(Scene::currentScenePath == "") {
                        ImGuiFileDialog::Instance()->OpenDialog("SaveSceneDialog", ICON_FA_FLOPPY_DISK " Save Scene", ".static", ".");
                    } else {
                        Scene::SaveScene(Scene::currentScenePath);
                    }
                }

                if(ImGui::MenuItem("Save Scene As", "CTRL+SHIFT+S")) {
                    ImGuiFileDialog::Instance()->OpenDialog("SaveSceneDialog", ICON_FA_FLOPPY_DISK " Save Scene", ".static", ".");
                }

                if(ImGui::MenuItem("Config", "CTRL+SHIFT+C")) {
                    openConfig = true;
                }

                if(ImGui::MenuItem("Build", "CTRL+B")) {
                    ImGuiFileDialog::Instance()->OpenDialog("BuildDialog", "Build", nullptr, ".");
                }
                // if(ImGui::MenuItem("Build")) {
                    // ImGuiFileDialog::Instance()->OpenDialog("BuildDialog", "Build", nullptr, cwd + "/builds");
                // }
                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Tools")) {
                if(ImGui::MenuItem("Inspector")) {
                    openInspector = true;
                }

                ImGui::EndMenu();
            }
            if(ImGui::BeginMenu("Info")) {
                if(ImGui::MenuItem("Details")) {
                    openDetails = true;
                }

                ImGui::EndMenu();
            }
            ImGui::EndMainMenuBar();
        }

        if(openInspector) {
            if(ImGui::Begin(ICON_FA_MAGNIFYING_GLASS " Inspector")) {
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
                    {
                        // const char* path = (const char*)payload->Data;
                        dirPayloadData.erase(0, cwd.length() + 1);

                        if(
                            ends_with(dirPayloadData, ".material")
                        ) {
                            inspectorType = InspecType::Material;
                            std::ifstream file(dirPayloadData);
                            nlohmann::json JSON = nlohmann::json::parse(file);

                            m_InspectorMaterial.diffuse = JSON["diffuse"] == "nullptr" ? "None" : JSON["diffuse"];
                            m_InspectorMaterial.specular = JSON["specular"] == "nullptr" ? "None" : JSON["specular"];
                            m_InspectorMaterial.normal = JSON["normal"] == "nullptr" ? "None" : JSON["normal"];
                            m_InspectorMaterial.roughness = JSON["roughness"];
                            m_InspectorMaterial.metallic = JSON["metallic"];

                            file.close();
                        }
                    }

                    ImGui::EndDragDropTarget();
                }

                switch (inspectorType)
                {
                    case InspecType::Material: {
                        if(ImGui::TreeNode("Diffuse")) {
                            if(ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
                                {
                                    dirPayloadData.erase(0, cwd.length() + 1);
                                    m_InspectorMaterial.diffuse = dirPayloadData;
                                }

                                ImGui::EndDragDropTarget();
                            }

                            ImGui::Text(m_InspectorMaterial.diffuse.c_str());

                            ImGui::TreePop();
                        }

                        if(ImGui::TreeNode("Specular")) {
                            if(ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
                                {
                                    dirPayloadData.erase(0, cwd.length() + 1);
                                    m_InspectorMaterial.specular = dirPayloadData;
                                }

                                ImGui::EndDragDropTarget();
                            }

                            ImGui::Text(m_InspectorMaterial.specular.c_str());

                            ImGui::TreePop();
                        }

                        if(ImGui::TreeNode("Normal")) {
                            if(ImGui::BeginDragDropTarget()) {
                                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("file"))
                                {
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

                        if(ImGui::Button(ICON_FA_FLOPPY_DISK " Save Material")) {
                            ImGuiFileDialog::Instance()->OpenDialog("SaveMaterialDialog", ICON_FA_FLOPPY_DISK " Save Material", ".material", ".");
                        }
                        ImGui::NewLine();


                        if (ImGuiFileDialog::Instance()->Display("SaveMaterialDialog")) 
                        {
                            // action if OK
                            if (ImGuiFileDialog::Instance()->IsOk())
                            {
                                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                                // remove cwd from filePathName
                                filePathName.erase(0, cwd.length() + 1);
                                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                                
                                std::ofstream file(filePathName);
                                nlohmann::json j = {
                                    {"diffuse", m_InspectorMaterial.diffuse},
                                    {"specular", m_InspectorMaterial.specular},
                                    {"normal", m_InspectorMaterial.normal},
                                    {"roughness", m_InspectorMaterial.roughness},
                                    {"metallic", m_InspectorMaterial.metallic},
                                    {"baseColor", {
                                            {"r", m_InspectorMaterial.baseColor.x},
                                            {"g", m_InspectorMaterial.baseColor.y},
                                            {"b", m_InspectorMaterial.baseColor.z},
                                            {"a", m_InspectorMaterial.baseColor.w}
                                        }
                                    },
                                    {"texUV", {
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

                if(inspectorType == InspecType::None) {
                    ImGui::Text("To inspect an object, drag it into the inspector window's title bar.");
                }
                if(ImGui::Button("Close")) {
                    inspectorType = InspecType::None;
                    openInspector = false;
                }

                ImGui::End();
            }
        }

        if (ImGuiFileDialog::Instance()->Display("SaveSceneDialog")) 
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                // remove cwd from filePathName
                filePathName.erase(0, cwd.length() + 1);
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();
                
                Scene::SaveScene(filePathName);
            }

            ImGuiFileDialog::Instance()->Close();
        }

        if(openConfig) {
            ImGui::OpenPopup("Edit Config");
            openConfig = false;
        }

        if(openDetails) {
            ImGui::OpenPopup("Details");
            openDetails = false;
        }

        ImGui::SetNextWindowSize(ImVec2(500, 0));
        if(ImGui::BeginPopup("Edit Config")) {
            ImGui::InputText("Game Name", config.name, 500);
            ImGui::DragFloat("Ambient Lightning", &config.ambientLight, 0.01f, 0, 1);
            ImGui::DragInt("Width", &config.width, 1, 0, 1920);
            ImGui::DragInt("Height", &config.height, 1, 0, 1080);
            if(ImGui::Button("Main Scene", ImVec2(500, 0))) {
                ImGuiFileDialog::Instance()->OpenDialog("ChooseMainScene", "Choose Main Scene", ".static", ".");
                // ImGui::CloseCurrentPopup();
            }
            ImGui::Separator();
            if(ImGui::Button("Save", ImVec2(500, 0))) {
                nlohmann::json j = {
                    {"name", config.name},
                    {"ambientLight", config.ambientLight},
                    {"mainScene", config.mainScene},
                    {"width", config.width},
                    {"height", config.height}
                };

                std::ofstream o("assets/game.config");
                o << std::setw(4) << j << std::endl;

                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        if (ImGuiFileDialog::Instance()->Display("ChooseMainScene")) 
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                // remove cwd from filePathName
                filePathName.erase(0, cwd.length() + 1);
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                config.mainScene = filePathName;
            }

            
            // close
            ImGuiFileDialog::Instance()->Close();
        }

        ImGui::DockSpaceOverViewport(ImGui::GetMainViewport());
        // make window closeable

        if(ImGui::BeginPopup("Details")) {
            ImGui::Text("Vendor: %s", glGetString(GL_VENDOR));
            ImGui::Text("Renderer: %s", glGetString(GL_RENDERER));
            ImGui::Text("Version: %s", glGetString(GL_VERSION));
            ImGui::Text("Shading Language Version: %s", glGetString(GL_SHADING_LANGUAGE_VERSION));

            if(ImGui::Button("Close")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::End();
        }

        if(ImGui::Begin(ICON_FA_GAMEPAD" Scene")) {
            Scene::DropTargetMat(Scene::DRAG_SCENE, nullptr);

            ImVec2 w_s = ImGui::GetWindowSize();
            winSize = Vector2(w_s.x, w_s.y);
            // ImGui::SetWindowSize(ImVec2(w_s.x, w_s.y - 50));
            if(camera->mode2D) {
                if(ImGui::Button(ICON_FA_CAMERA " 3D View")) {
                    camera->mode2D = false;
                }
            } else {
                if(ImGui::Button(ICON_FA_CAMERA " 2D View")) {
                    camera->mode2D = true;
                }
            }
            ImGui::SameLine();
            if(!HyperAPI::isRunning) {
            if(HyperAPI::isStopped) {
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - 25 / 2);
            } else {
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - 25 / 2 - (25/ 2));
            }
                
            if(ImGui::Button(ICON_FA_PLAY, ImVec2(25, 0))) {
                InitScripts();

                for(auto &gameObject : Scene::m_GameObjects) {
                    if(gameObject->HasComponent<Experimental::m_LuaScriptComponent>()) {
                        gameObject->GetComponent<Experimental::m_LuaScriptComponent>().Start();
                    }

                    if(gameObject->HasComponent<Experimental::NativeScriptManager>()) {
                        gameObject->GetComponent<Experimental::NativeScriptManager>().Start();
                    }
                }

                Scene::world = new b2World({ 0.0, -5.8f });
                Scene::world->SetContactListener(listener);
                auto view = Scene::m_Registry.view<Experimental::Rigidbody2D>();
                
                for(auto e : view) {
                    Experimental::GameObject *gameObject;
                    for(auto &go : Scene::m_GameObjects) {
                        if(go->entity == e) {
                            gameObject = go;
                            break;
                        }
                    }

                    auto &transform = gameObject->GetComponent<Experimental::Transform>();
                    auto &rb2d = gameObject->GetComponent<Experimental::Rigidbody2D>();

                    b2BodyDef bodyDef;
                    bodyDef.userData.pointer = reinterpret_cast<uintptr_t>(gameObject);
                    bodyDef.type = rb2d.type;
                    bodyDef.position.Set(transform.position.x, transform.position.y);
                    bodyDef.angle = glm::radians(transform.rotation.z);
                    bodyDef.gravityScale = rb2d.gravityScale;

                    b2Body *body = Scene::world->CreateBody(&bodyDef);
                    body->SetFixedRotation(rb2d.fixedRotation);
                    rb2d.body = body;

                    if(gameObject->HasComponent<Experimental::BoxCollider2D>()) {
                        auto &boxCollider2D = gameObject->GetComponent<Experimental::BoxCollider2D>();
                        b2PolygonShape shape;
                        shape.SetAsBox((((boxCollider2D.size.x) / 2) - 0.02) / 2, (((boxCollider2D.size.y) / 2) - 0.02) / 2);
                        
                        b2FixtureDef fixtureDef;
                        fixtureDef.shape = &shape;
                        fixtureDef.density = boxCollider2D.density;
                        fixtureDef.friction = boxCollider2D.friction;
                        fixtureDef.restitution = boxCollider2D.restitution;
                        fixtureDef.restitutionThreshold = boxCollider2D.restitutionThreshold;
                        boxCollider2D.fixture = body->CreateFixture(&fixtureDef);
                    }
                }

                HyperAPI::isRunning = true;
                HyperAPI::isStopped = false;

                for(auto &camera : Scene::cameras) {
                    if(camera->mainCamera) {
                        Scene::mainCamera = camera;
                        break;
                    }
                }
            }

            if(!HyperAPI::isStopped) {
                // ImGui::SameLine();
                // if(ImGui::Button(ICON_FA_STOP, ImVec2(25, 0))) {
                //     HyperAPI::isRunning = false;
                //     HyperAPI::isStopped = true;
                //     Scene::mainCamera = camera;
                // }
            }
            } else {
                // move it more left
                ImGui::SetCursorPosX((ImGui::GetWindowSize().x / 2) - 25 / 2 - (25/ 2));
                if(ImGui::Button(ICON_FA_PAUSE, ImVec2(25, 0))) {
                    HyperAPI::isRunning = false;
                    
                    for(auto &gameObject : Scene::m_GameObjects) {
                        if(gameObject->HasComponent<Experimental::NativeScriptManager>()) {
                            auto &script = gameObject->GetComponent<Experimental::NativeScriptManager>();
                            for(auto &script : script.m_StaticScripts) {
                                delete script;
                            }

                            gameObject->RemoveComponent<Experimental::NativeScriptManager>();
                        }
                    }
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
                auto[mx, my] = ImGui::GetMousePos();
                auto[wx, wy] = ImGui::GetWindowPos();
                auto[sizex, sizey] = ImGui::GetWindowSize();
                mx -= wx;
                my -= wy;
                my = sizey - my;
                // std::cout << mx << " " << my << std::endl;
                app.sceneMouseX = mx;
                app.sceneMouseY = my;

                app.width = sizex;
                app.height = sizey;

                ImVec2 w_p = ImGui::GetWindowPos();
                winPos = Vector2(w_p.x, w_p.y);

                glActiveTexture(GL_TEXTURE15);
                glBindTexture(GL_TEXTURE_2D, PPT);

                // check window hovered
                if(ImGui::IsWindowHovered() && ImGui::IsMouseDragging(0)) {
                    focusedOnScene = true;
                } else {
                    focusedOnScene = false;
                }

                if(ImGui::IsWindowHovered()) {
                    hoveredScene = true;
                } else {
                    hoveredScene = false;
                }

                ImGui::Image((void*)PPT, ImVec2(w_s.x, w_s.y - 40), ImVec2(0, 1), ImVec2(1, 0));

            ImGui::EndChild();
            ImGui::End();
        }
    
        if(ImGui::Begin(ICON_FA_CUBES " Hierarchy")) {
            Scene::DropTargetMat(Scene::DRAG_MODEL, nullptr);
            ImVec2 win_size = ImGui::GetWindowSize();

            if(ImGui::Button(ICON_FA_PLUS " Add GameObject", ImVec2(win_size.x, 0))) {
                Experimental::GameObject *go = new Experimental::GameObject();
                go->AddComponent<Experimental::Transform>();
            }

            for(int i = 0; i < Scene::m_GameObjects.size(); i++) {
                if(Scene::m_GameObjects[i]->parentID != "NO_PARENT") continue;
                Scene::m_GameObjects[i]->GUI();
            }

            ImGui::End();
        }

        if(ImGui::Begin(ICON_FA_SHARE_NODES " Components")) {
            if(Scene::m_Object != nullptr) {
                ImGui::InputText("Name", Scene::name, 500);
                ImGui::InputText("Tag", Scene::tag, 500);
                Scene::m_Object->tag = Scene::tag;
                Scene::m_Object->name = Scene::name;

                if(Scene::m_Object->HasComponent<Experimental::Transform>()) {
                    Scene::m_Object->GetComponent<Experimental::Transform>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::CameraComponent>()) {
                    Scene::m_Object->GetComponent<Experimental::CameraComponent>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::MeshRenderer>()) {
                    Scene::m_Object->GetComponent<Experimental::MeshRenderer>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::m_LuaScriptComponent>()) {
                    Scene::m_Object->GetComponent<Experimental::m_LuaScriptComponent>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::c_PointLight>()) {
                    Scene::m_Object->GetComponent<Experimental::c_PointLight>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::c_SpotLight>()) {
                    Scene::m_Object->GetComponent<Experimental::c_SpotLight>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::c_DirectionalLight>()) {
                    Scene::m_Object->GetComponent<Experimental::c_DirectionalLight>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::SpriteRenderer>()) {
                    Scene::m_Object->GetComponent<Experimental::SpriteRenderer>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::SpriteAnimation>()) {
                    Scene::m_Object->GetComponent<Experimental::SpriteAnimation>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::SpritesheetRenderer>()) {
                    Scene::m_Object->GetComponent<Experimental::SpritesheetRenderer>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::BoxCollider2D>()) {
                    Scene::m_Object->GetComponent<Experimental::BoxCollider2D>().GUI();
                }

                if(Scene::m_Object->HasComponent<Experimental::Rigidbody2D>()) {
                    Scene::m_Object->GetComponent<Experimental::Rigidbody2D>().GUI();
                }

                ImGui::Separator();

                ImVec2 win_size = ImGui::GetWindowSize();
                if(!HyperAPI::isRunning) {
                    if(ImGui::Button(ICON_FA_PLUS " Add Component", ImVec2(win_size.x, 0))) {
                        ImGui::OpenPopup("Add Component");
                    }
                }
            }

            if(ImGui::BeginPopup("Add Component")) {
                if(ImGui::Button("Transform", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::Transform>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Mesh Renderer", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::MeshRenderer>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Lua Scripts", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::m_LuaScriptComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Camera", ImVec2(200, 0))) {
                    // CameraComponent has one argument of type entt::entity
                    Scene::m_Object->AddComponent<Experimental::CameraComponent>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Point Light", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::c_PointLight>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Spot Light", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::c_SpotLight>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Directional Light", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::c_DirectionalLight>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Sprite Renderer", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::SpriteRenderer>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Sprite Animation", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::SpriteAnimation>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Spritesheet Renderer", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::SpritesheetRenderer>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Rigidbody 2D", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::Rigidbody2D>();
                    ImGui::CloseCurrentPopup();
                }

                if(ImGui::Button("Box Collider 2D", ImVec2(200, 0))) {
                    Scene::m_Object->AddComponent<Experimental::BoxCollider2D>();
                    ImGui::CloseCurrentPopup();
                }

                ImGui::EndPopup();
            }

            ImGui::End();
        }
        
        if(ImGui::Begin(ICON_FA_FOLDER " Assets")) {
            Scene::DropTargetMat(Scene::DRAG_GAMEOBJECT, nullptr);
            DirIter(cwd + std::string("/assets"));
            ImGui::End();
        }

        if(ImGui::Begin(ICON_FA_TERMINAL " Console")) {
            if(ImGui::Button(ICON_FA_TRASH " Clear")) {
                Scene::logs.clear();
            }

            ImGui::Separator();

            for(auto &log : Scene::logs) {
                log.GUI();
            }

            ImGui::End();
        }

        if (ImGuiFileDialog::Instance()->Display("BuildDialog")) 
        {
            // action if OK
            if (ImGuiFileDialog::Instance()->IsOk())
            {
                std::string filePathName = ImGuiFileDialog::Instance()->GetFilePathName();
                std::string filePath = ImGuiFileDialog::Instance()->GetCurrentPath();

                std::string toCd = "cd \"" + filePathName + "\" && ";

                fs::copy(cwd + "/assets", filePathName + "/assets", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                fs::copy(cwd + "/build", filePathName + "/build", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                fs::copy(cwd + "/shaders", filePathName + "/shaders", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                // copy all files that start with *.dll AND THEY ARE NOT IN LIB FOLDER
                fs::copy(cwd + "/dlls", filePathName, fs::copy_options::recursive | fs::copy_options::overwrite_existing);
                fs::copy(cwd + "/platforms/dist/windows/Game.exe", filePathName + "/" + config.name + ".exe", fs::copy_options::recursive | fs::copy_options::overwrite_existing);
            }

            
            // close
            ImGuiFileDialog::Instance()->Close();
        }
    };

    bool calledOnce = false;

    app.Run([&] {
        if(Scene::mainCamera == nullptr) {
            Scene::mainCamera = camera;
        }

        if(!calledOnce) {
            auto& colors = ImGui::GetStyle().Colors;
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

        if(focusedOnScene && !camera->mode2D && Scene::mainCamera == camera) {
            camera->Inputs(app.renderer->window, winPos);
        }
        Input::winPos = Vector3(winPos.x, winPos.y, 0);
        Input::winSize = Vector3(winSize.x, winSize.y, 0);

        if(hoveredScene && camera->mode2D) {
            auto transform = camera->GetComponent<TransformComponent>();
            transform.rotation = glm::vec3(0.0f, 0.0f, -1.0f);
            camera->Inputs(app.renderer->window, winPos);
            camera->UpdateComponent(transform);
        }
        winSize = Vector2(app.width, app.height);

        for(auto &camera : Scene::cameras) {
            camera->updateMatrix(camera->cam_fov, camera->cam_near, camera->cam_far, winSize);
        }
        camera->updateMatrix(camera->cam_fov, camera->cam_near, camera->cam_far, winSize);
        skybox.Draw(*Scene::mainCamera, winSize.x, winSize.y);

        // floor.Draw(shader, *camera);
        shader.Bind();
        shader.SetUniform1f("ambient", config.ambientLight);
        shader.SetUniform1i("shadowMap", 17);
        shader.SetUniformMat4("lightProjection", Scene::projection);

        spriteShader.Bind();
        spriteShader.SetUniform1f("ambient", config.ambientLight);

        // Physics

        if(HyperAPI::isRunning && Scene::world != nullptr) {
            const int32_t velocityIterations = 6;
            const int32_t positionIterations = 2;
            Scene::world->Step(1.0f / 60.0f, velocityIterations, positionIterations);

            auto view = Scene::m_Registry.view<Experimental::Rigidbody2D>();
            for(auto e : view) {
                Experimental::GameObject *m_GameObject;
                for(auto &gameObject : Scene::m_GameObjects) {
                    if(gameObject->entity == e) {
                        m_GameObject = gameObject;
                    }
                }

                auto &transform = m_GameObject->GetComponent<Experimental::Transform>();
                auto &rigidbody = m_GameObject->GetComponent<Experimental::Rigidbody2D>();

                b2Body *body = (b2Body*)rigidbody.body;
                const auto &position = body->GetPosition();
                transform.position.x = position.x;
                transform.position.y = position.y;
                transform.rotation.z = glm::degrees(body->GetAngle());
            }
        }

        // Phyiscs

        for(auto &gameObject : Scene::m_GameObjects) {
            gameObject->Update();

            if(gameObject->HasComponent<Experimental::Transform>()) {
                gameObject->GetComponent<Experimental::Transform>().Update();
            }

            if(gameObject->HasComponent<Experimental::m_LuaScriptComponent>()) {
                gameObject->GetComponent<Experimental::m_LuaScriptComponent>().Update();
            }

            if(gameObject->HasComponent<Experimental::c_PointLight>()) {
                gameObject->GetComponent<Experimental::c_PointLight>().Update();
            }

            if(gameObject->HasComponent<Experimental::c_SpotLight>()) {
                gameObject->GetComponent<Experimental::c_SpotLight>().Update();
            }

            if(gameObject->HasComponent<Experimental::c_DirectionalLight>()) {
                gameObject->GetComponent<Experimental::c_DirectionalLight>().Update();
            }

            if(gameObject->HasComponent<Experimental::m_LuaScriptComponent>()) {
                auto &script = gameObject->GetComponent<Experimental::m_LuaScriptComponent>();
                if(HyperAPI::isRunning) {
                    script.Update();
                }
            }

            if(gameObject->HasComponent<Experimental::NativeScriptManager>()) {
                auto &script = gameObject->GetComponent<Experimental::NativeScriptManager>();
                if(HyperAPI::isRunning) {
                    script.Update();
                }
            }
            
            if(gameObject->HasComponent<Experimental::MeshRenderer>()) {
                // if(gameObject->GetComponcent<Experimental::MeshRenderer>().m_Model) continue;

                auto meshRenderer = gameObject->GetComponent<Experimental::MeshRenderer>();
                auto transform = gameObject->GetComponent<Experimental::Transform>();
                transform.Update();

                glm::mat4 extra = meshRenderer.extraMatrix;

                if(meshRenderer.m_Mesh != nullptr) {
                    if(transform.parentTransform != nullptr) {
                        transform.parentTransform->Update();
                        meshRenderer.m_Mesh->Draw(shader, *Scene::mainCamera, transform.transform * transform.parentTransform->transform * extra);
                    } else {
                        meshRenderer.m_Mesh->Draw(shader, *Scene::mainCamera, transform.transform * extra);
                    }
                }
            }

            if(gameObject->HasComponent<Experimental::SpriteRenderer>()) {
                auto spriteRenderer = gameObject->GetComponent<Experimental::SpriteRenderer>();
                auto transform = gameObject->GetComponent<Experimental::Transform>();
                transform.Update();

                spriteRenderer.mesh->Draw(shader, *Scene::mainCamera, transform.transform);
            }

            if(gameObject->HasComponent<Experimental::SpritesheetRenderer>()) {
                auto spritesheetRenderer = gameObject->GetComponent<Experimental::SpritesheetRenderer>();
                auto transform = gameObject->GetComponent<Experimental::Transform>();
                transform.Update();

                if(spritesheetRenderer.mesh != nullptr) {
                    spritesheetRenderer.mesh->Draw(shader, *Scene::mainCamera, transform.transform);
                }
            }

            if(gameObject->HasComponent<Experimental::SpriteAnimation>()) {
                auto spriteAnimation = gameObject->GetComponent<Experimental::SpriteAnimation>();
                auto transform = gameObject->GetComponent<Experimental::Transform>();
                transform.Update();

                spriteAnimation.Play();

                if(spriteAnimation.currMesh != nullptr) {
                    spriteAnimation.currMesh->Draw(shader, *Scene::mainCamera, transform.transform);
                }
            }
        }
    }, GUI_EXP);

    return 0;
}