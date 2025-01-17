#include "lib/scene.hpp"
#include "Components/3DText.hpp"
#include "Components/AudioListener.hpp"
#include "Components/CppScriptManager.hpp"
#include "Components/Exp_Base.hpp"
#include "Renderer/Mesh.hpp"
#include "Renderer/Structures.hpp"
#include "box2d/b2_world_callbacks.h"
#include "f_GameObject/f_GameObject.hpp"
#include "lib/api.hpp"
#include "libs.hpp"
#include "json/json.hpp"
#include <experimental/bits/fs_ops.h>
#include <memory>
#include <cmath>

bool G_END_WITH(std::string const &value, std::string const &ending) {
    if (ending.size() > value.size())
        return false;
    return std::equal(ending.rbegin(), ending.rend(), value.rbegin());
}

void ApplyTransform(HyperAPI::Experimental::Transform &transform, nlohmann::json JSON) {
    transform.position = Vector3(
        JSON["position"]["x"],
        JSON["position"]["y"],
        JSON["position"]["z"]);

    transform.rotation = Vector3(
        JSON["rotation"]["x"],
        JSON["rotation"]["y"],
        JSON["rotation"]["z"]);

    transform.scale = Vector3(
        JSON["scale"]["x"],
        JSON["scale"]["y"],
        JSON["scale"]["z"]);
}

namespace HyperAPI {
    namespace Scene {
        DLL_API std::map<std::string, Texture *> Textures;

        void SavePrefab(const std::string &path, Experimental::GameObject *gameObject) {
            std::ofstream file(path);
            nlohmann::json JSON;

            int offset = 0;

            std::string name = gameObject->name;
            std::string ID = gameObject->ID;
            std::string parentID = gameObject->parentID;
            std::string tag = gameObject->tag;
            std::string layer = gameObject->layer;

            JSON[0]["name"] = name;
            JSON[0]["ID"] = ID;
            JSON[0]["tag"] = tag;
            JSON[0]["parentID"] = parentID;
            JSON[0]["layer"] = layer;
            JSON[0]["components"] = nlohmann::json::array();

            int componentOffset = 0;
            SaveComponents(JSON, gameObject, 0, componentOffset);

            for (int i = 0; i < m_GameObjects->size(); i++) {
                auto &gameObject = (*m_GameObjects)[i];
                if (gameObject->parentID != ID)
                    continue;

                std::string name = gameObject->name;
                std::string ID = gameObject->ID;
                std::string parentID = gameObject->parentID;
                std::string tag = gameObject->tag;
                std::string layer = gameObject->layer;

                JSON[i]["name"] = name;
                JSON[i]["ID"] = ID;
                JSON[i]["tag"] = tag;
                JSON[i]["parentID"] = parentID;
                JSON[i]["layer"] = layer;
                JSON[i]["components"] = nlohmann::json::array();

                int componentOffset = 0;
                SaveComponents(JSON, gameObject, i, componentOffset);
            }

            file << JSON;
            file.close();
        }
        void SaveJSONPrefab(nlohmann::json &JSON, Experimental::GameObject *gameObject) {
            int offset = 0;

            std::string name = gameObject->name;
            std::string ID = gameObject->ID;
            std::string parentID = gameObject->parentID;
            std::string tag = gameObject->tag;
            std::string layer = gameObject->layer;

            JSON[0]["name"] = name;
            JSON[0]["ID"] = ID;
            JSON[0]["tag"] = tag;
            JSON[0]["parentID"] = "NO_PARENT";
            JSON[0]["layer"] = layer;
            JSON[0]["components"] = nlohmann::json::array();

            int componentOffset = 0;
            SaveComponents(JSON, gameObject, 0, componentOffset);

            for (int i = 0; i < m_GameObjects->size(); i++) {
                auto &gameObject = (*m_GameObjects)[i];
                if (gameObject->parentID != ID)
                    continue;

                std::string name = gameObject->name;
                std::string ID = gameObject->ID;
                std::string parentID = "NO_PARENT";
                std::string tag = gameObject->tag;
                std::string layer = gameObject->layer;

                JSON[i]["name"] = name;
                JSON[i]["ID"] = ID;
                JSON[i]["tag"] = tag;
                JSON[i]["parentID"] = parentID;
                JSON[i]["layer"] = layer;
                JSON[i]["components"] = nlohmann::json::array();

                int componentOffset = 0;
                SaveComponents(JSON, gameObject, i, componentOffset);
            }
        }

        void LoadComponents(nlohmann::json &component, Experimental::GameObject *gameObject, std::string type, std::string &meshType, Experimental::Transform &transform) {
            if (type == "Transform") {
                gameObject->AddComponent<Experimental::Transform>();
                ApplyTransform(gameObject->GetComponent<Experimental::Transform>(), component);
                transform.position = gameObject->GetComponent<Experimental::Transform>().position;
                transform.rotation = gameObject->GetComponent<Experimental::Transform>().rotation;
                transform.scale = gameObject->GetComponent<Experimental::Transform>().scale;
            }

            if (type == "AudioListener") {
                gameObject->AddComponent<Experimental::AudioListener>();
                auto &comp = gameObject->GetComponent<Experimental::AudioListener>();
            }

            if (type == "Audio3D") {
                gameObject->AddComponent<Experimental::Audio3D>();

                auto &comp = gameObject->GetComponent<Experimental::Audio3D>();
                comp.src.pitch = component["pitch"];
                comp.src.volume = component["volume"];
                comp.src.max_distance = component["max_distance"];
                comp.src.loop = component["loop"];
                comp.on_start = component["on_start"];
                comp.file = component["file"];
                comp.src.velocity = Vector3(
                    component["velocity"]["x"],
                    component["velocity"]["y"],
                    component["velocity"]["z"]);
            }

            if (type == "MeshRenderer") {
                gameObject->AddComponent<Experimental::MeshRenderer>();
                auto &meshRenderer = gameObject->GetComponent<Experimental::MeshRenderer>();
                nlohmann::json meshConfig = component["mesh"];

                if (!meshConfig["custom"]) {
                    std::string meshType = meshConfig["mesh"];
                    meshRenderer.meshType = meshType;

                    if (meshType == "Plane") {
                        meshRenderer.m_Mesh = Plane(Vector4(1, 1, 1, 1)).m_Mesh;
                    }
                    if (meshType == "Cube") {
                        meshRenderer.m_Mesh = Cube(Vector4(1, 1, 1, 1)).meshes[0];
                    }
                    if (meshType == "Sphere") {
                        meshRenderer.m_Mesh = Sphere(Vector4(1, 1, 1, 1)).meshes[0];
                    }
                    if (meshType == "Cone") {
                        meshRenderer.m_Mesh = Cone(Vector4(1, 1, 1, 1)).meshes[0];
                    }
                    if (meshType == "Capsule") {
                        meshRenderer.m_Mesh = Capsule(Vector4(1, 1, 1, 1)).meshes[0];
                    }
                    if (meshType == "Torus") {
                        meshRenderer.m_Mesh = Torus(Vector4(1, 1, 1, 1)).meshes[0];
                    }
                    if (meshType == "Cylinder") {
                        meshRenderer.m_Mesh = Cylinder(Vector4(1, 1, 1, 1)).meshes[0];
                    }

                    if (component["material"] != "") {
                        std::ifstream file(component["material"]);
                        nlohmann::json JSON = nlohmann::json::parse(file);

                        const std::string diffuseTexture = JSON["diffuse"];
                        const std::string specularTexture = JSON["specular"];
                        const std::string normalTexture = JSON["normal"];

                        if (diffuseTexture != "nullptr") {
                            if (meshRenderer.m_Mesh->material.diffuse != nullptr) {
                                delete meshRenderer.m_Mesh->material.diffuse;
                            }

                            meshRenderer.m_Mesh->material.diffuse = new Texture(diffuseTexture.c_str(), 0, "texture_diffuse");
                        }

                        if (specularTexture != "nullptr") {
                            if (meshRenderer.m_Mesh->material.specular != nullptr) {
                                delete meshRenderer.m_Mesh->material.specular;
                            }

                            meshRenderer.m_Mesh->material.specular = new Texture(specularTexture.c_str(), 1, "texture_specular");
                        }

                        if (normalTexture != "nullptr") {
                            if (meshRenderer.m_Mesh->material.normal != nullptr) {
                                delete meshRenderer.m_Mesh->material.normal;
                            }

                            meshRenderer.m_Mesh->material.normal = new Texture(normalTexture.c_str(), 2, "texture_normal");
                        }

                        meshRenderer.m_Mesh->material.baseColor = Vector4(
                            JSON["baseColor"]["r"],
                            JSON["baseColor"]["g"],
                            JSON["baseColor"]["b"],
                            JSON["baseColor"]["a"]);

                        meshRenderer.m_Mesh->material.roughness = JSON["roughness"];
                        meshRenderer.m_Mesh->material.metallic = JSON["metallic"];
                        meshRenderer.m_Mesh->material.texUVs = Vector2(JSON["texUV"]["x"], JSON["texUV"]["y"]);

                        meshRenderer.matPath = component["material"];
                    }
                } else {
                    Experimental::Model model((char *)((std::string)meshConfig["mesh"]).c_str(), true, Vector4(1, 1, 1, 1), true);
                    std::cout << model.mesh_mats.size() << std::endl;
                    meshRenderer.m_Mesh = model.mesh_mats[(int)meshConfig["mesh_index"]]->mesh;
                    meshRenderer.meshType = meshConfig["mesh"];

                    if (component["material"] != "") {
                        std::ifstream file(component["material"]);
                        nlohmann::json JSON = nlohmann::json::parse(file);

                        const std::string diffuseTexture = JSON["diffuse"];
                        const std::string specularTexture = JSON["specular"];
                        const std::string normalTexture = JSON["normal"];

                        if (diffuseTexture != "nullptr") {
                            if (meshRenderer.m_Mesh->material.diffuse != nullptr) {
                                delete meshRenderer.m_Mesh->material.diffuse;
                            }

                            meshRenderer.m_Mesh->material.diffuse = new Texture(diffuseTexture.c_str(), 0, "texture_diffuse");
                        }

                        if (specularTexture != "nullptr") {
                            if (meshRenderer.m_Mesh->material.specular != nullptr) {
                                delete meshRenderer.m_Mesh->material.specular;
                            }

                            meshRenderer.m_Mesh->material.specular = new Texture(specularTexture.c_str(), 1, "texture_specular");
                        }

                        if (normalTexture != "nullptr") {
                            if (meshRenderer.m_Mesh->material.normal != nullptr) {
                                delete meshRenderer.m_Mesh->material.normal;
                            }

                            meshRenderer.m_Mesh->material.normal = new Texture(normalTexture.c_str(), 2, "texture_normal");
                        }

                        meshRenderer.m_Mesh->material.baseColor = Vector4(
                            JSON["baseColor"]["r"],
                            JSON["baseColor"]["g"],
                            JSON["baseColor"]["b"],
                            JSON["baseColor"]["a"]);

                        meshRenderer.m_Mesh->material.roughness = JSON["roughness"];
                        meshRenderer.m_Mesh->material.metallic = JSON["metallic"];
                        meshRenderer.m_Mesh->material.texUVs = Vector2(JSON["texUV"]["x"], JSON["texUV"]["y"]);

                        meshRenderer.matPath = component["material"];
                    }

                    meshRenderer.m_Model = true;
                    meshRenderer.mesh_index = meshConfig["mesh_index"];
                }
            }

            if (type == "Text3D") {
                gameObject->AddComponent<Experimental::Text3D>();
                auto &text = gameObject->GetComponent<Experimental::Text3D>();
                strcpy(text.text, ((std::string)component["text"]).c_str());
                text.color = Vector3(component["color"]["r"], component["color"]["g"], component["color"]["b"]);
                text.scale = component["scale"];
                text.y_offset = component["y_offset"];
                delete text.font;
                text.font = new Font(((std::string)component["font"]).c_str(), 48);
            }

            if (type == "DirectionalLight") {
                gameObject->AddComponent<Experimental::c_DirectionalLight>();
                auto &light = gameObject->GetComponent<Experimental::c_DirectionalLight>();

                light.color = Vector3(
                    component["color"]["r"],
                    component["color"]["g"],
                    component["color"]["b"]);

                light.intensity = component["intensity"];
            }

            if (type == "PointLight") {
                gameObject->AddComponent<Experimental::c_PointLight>();
                auto &light = gameObject->GetComponent<Experimental::c_PointLight>();

                light.color = Vector3(
                    component["color"]["r"],
                    component["color"]["g"],
                    component["color"]["b"]);

                light.intensity = component["intensity"];

                if (component.contains("renderShadows")) {
                    light.light->renderShadows = component["renderShadows"];
                }

                if (component.contains("farPlane")) {
                    light.light->farPlane = component["farPlane"];
                }
            }

            if (type == "SpotLight") {
                gameObject->AddComponent<Experimental::c_SpotLight>();
                auto &light = gameObject->GetComponent<Experimental::c_SpotLight>();

                light.color = Vector3(
                    component["color"]["r"],
                    component["color"]["g"],
                    component["color"]["b"]);
            }

            if (type == "Light2D") {
                gameObject->AddComponent<Experimental::c_Light2D>();
                auto &light = gameObject->GetComponent<Experimental::c_Light2D>();

                light.color = Vector3(
                    component["color"]["r"],
                    component["color"]["g"],
                    component["color"]["b"]);

                light.range = component["range"];
            }

            if (type == "CameraComponent") {
                gameObject->AddComponent<Experimental::CameraComponent>();
                auto &transformg = gameObject->GetComponent<Experimental::Transform>();
                transformg.rotation = transform.rotation;
                auto &camera = gameObject->GetComponent<Experimental::CameraComponent>();

                camera.camera->cam_fov = component["fov"];
                camera.camera->cam_near = component["near"];
                camera.camera->cam_far = component["far"];
                camera.camera->main_camera = component["mainCamera"];
                camera.camera->mode2D = component["mode2D"];
                if (component.contains("depthCamera")) {
                    camera.depthCamera = component["depthCamera"];
                }

                for (auto &layer : component["layers"]) {
                    camera.camera->layers.push_back(layer);
                }

                if (camera.camera->main_camera) {
                    mainCamera = camera.camera;
                }
            }

            if (type == "LuaScriptComponent") {
                gameObject->AddComponent<Experimental::m_LuaScriptComponent>();
                auto &m_script = gameObject->GetComponent<Experimental::m_LuaScriptComponent>();
                for (auto &scriptPath : component["scripts"]) {
                    ScriptEngine::m_LuaScript script(scriptPath);
                    script.m_GameObject = gameObject;
                    script.ID = gameObject->ID;
                    // script.Init();
                    script.GetFields();
                    if (component.contains("script_fields")) {
                        for (auto &scriptField : script.m_Fields) {
                            auto &j_field = component["script_fields"][scriptPath];
                            if (j_field.contains(scriptField.first)) {

                                ScriptEngine::m_FieldValue value;
                                strcpy(value.value, j_field[scriptField.first].get<std::string>().c_str());
                                script.m_Fields[scriptField.first] = value;
                            }
                        }
                    }
                    m_script.scripts.push_back(script);
                }
            }

            if (type == "CppScriptManager") {
                gameObject->AddComponent<Experimental::CppScriptManager>();

                auto &scriptManager = gameObject->GetComponent<Experimental::CppScriptManager>();

                for (auto &scriptName : component["scripts"]) {
                    for (auto sharedObj : CppScripting::cpp_scripts) {
                        if (sharedObj.name == (std::string)scriptName) {
                            CppScripting::Script *script = sharedObj.create();
                            scriptManager.addedScripts.push_back(script);
                            scriptManager.addedScripts[scriptManager.addedScripts.size() - 1]->name = sharedObj.name;
                        }
                    }
                }
            }

            if (type == "CsharpScriptManager") {
                gameObject->AddComponent<Experimental::CsharpScriptManager>();

                auto &scriptManager = gameObject->GetComponent<Experimental::CsharpScriptManager>();

                for (auto klass : component["scripts"]) {
                    scriptManager.selectedScripts[klass] = klass;
                }
            }

            if (type == "SpriteRenderer") {
                gameObject->AddComponent<Experimental::SpriteRenderer>();
                auto &spriteRenderer = gameObject->GetComponent<Experimental::SpriteRenderer>();
                spriteRenderer.mesh->material.baseColor = Vector4(
                    component["color"]["r"],
                    component["color"]["g"],
                    component["color"]["b"],
                    component["color"]["a"]);

                if (component["sprite"] != "") {
                    spriteRenderer.mesh->material.diffuse = new Texture(((std::string)component["sprite"]).c_str(), 0, "texture_diffuse");
                }
            }

            if (type == "Rigidbody2D") {
                gameObject->AddComponent<Experimental::Rigidbody2D>();
                auto &rigidbody = gameObject->GetComponent<Experimental::Rigidbody2D>();

                rigidbody.type = component["bodyType"];
                rigidbody.gravityScale = component["gravityScale"];
                rigidbody.fixedRotation = component["fixedRotation"];
            }

            if (type == "BoxCollider2D") {
                gameObject->AddComponent<Experimental::BoxCollider2D>();
                auto &boxCollider = gameObject->GetComponent<Experimental::BoxCollider2D>();

                boxCollider.size = Vector2(
                    component["size"]["x"],
                    component["size"]["y"]);
                boxCollider.friction = component["friction"];
                boxCollider.density = component["density"];
                boxCollider.restitution = component["restitution"];
                boxCollider.restitutionThreshold = component["restitutionThreshold"];
                boxCollider.trigger = component["trigger"];
            }

            if (type == "SpritesheetRenderer") {
                gameObject->AddComponent<Experimental::SpritesheetRenderer>();
                auto &spritesheetRenderer = gameObject->GetComponent<Experimental::SpritesheetRenderer>();

                float alpha = 1;
                if (component["color"].contains("a")) {
                    alpha = component["a"];
                }

                spritesheetRenderer.mesh->material.baseColor = Vector4(
                    component["color"]["r"],
                    component["color"]["g"],
                    component["color"]["b"],
                    alpha);

                spritesheetRenderer.spriteOffset = Vector2(
                    component["spriteOffset"]["x"],
                    component["spriteOffset"]["y"]);

                spritesheetRenderer.spriteSize = Vector2(
                    component["spriteSize"]["x"],
                    component["spriteSize"]["y"]);

                spritesheetRenderer.spritesheetSize = Vector2(
                    component["spritesheetSize"]["x"],
                    component["spritesheetSize"]["y"]);

                delete spritesheetRenderer.mesh;
                delete spritesheetRenderer.sp;
                spritesheetRenderer.sp = new Spritesheet("", spritesheetRenderer.material, spritesheetRenderer.spritesheetSize, spritesheetRenderer.spriteSize, spritesheetRenderer.spriteOffset);
                spritesheetRenderer.mesh = spritesheetRenderer.sp->m_Mesh;
                if (component["spritesheet"] != "") {
                    spritesheetRenderer.mesh->material.diffuse = new Texture(((std::string)component["spritesheet"]).c_str(), 0, "texture_diffuse");
                }
            }

            if (type == "Rigidbody3D") {
                gameObject->AddComponent<Experimental::Rigidbody3D>();
                auto &rigidbody = gameObject->GetComponent<Experimental::Rigidbody3D>();

                rigidbody.mass = component["mass"];
                rigidbody.friction = component["friction"];
                rigidbody.restitution = component["restitution"];
                rigidbody.fixedRotation = component["fixedRotation"];
                rigidbody.trigger = component["trigger"];
            }

            if (type == "BoxCollider3D") {
                gameObject->AddComponent<Experimental::BoxCollider3D>();
                auto &boxCollider = gameObject->GetComponent<Experimental::BoxCollider3D>();

                boxCollider.size = Vector3(
                    component["size"]["x"],
                    component["size"]["y"],
                    component["size"]["z"]);
            }

            if (type == "MeshCollider3D") {
                gameObject->AddComponent<Experimental::MeshCollider3D>();
                auto &meshCollider = gameObject->GetComponent<Experimental::MeshCollider3D>();

                meshCollider.size = Vector3(
                    component["size"]["x"],
                    component["size"]["y"],
                    component["size"]["z"]);
            }

            if (type == "SpriteAnimation") {
                gameObject->AddComponent<Experimental::SpriteAnimation>();
                auto &spriteAnimation = gameObject->GetComponent<Experimental::SpriteAnimation>();
                strcpy(spriteAnimation.currAnim, component["currAnim"].get<std::string>().c_str());

                for (auto &animation : component["animations"]) {
                    Experimental::m_AnimationData animData;
                    strcpy(animData.name, animation["name"].get<std::string>().c_str());
                    animData.delay = animation["delay"];
                    animData.loop = animation["loop"];
                    animData.delay_counter = animation["delay_counter"];

                    for (auto &frame : animation["frames"]) {
                        Experimental::SpriteRenderer frameData;
                        if (frame["texture"] != "nullptr") {
                            frameData.mesh->material.diffuse = new Texture(((std::string)frame["texture"]).c_str(), 0, "texture_diffuse");
                        }
                        animData.frames.push_back(frameData);
                    }

                    spriteAnimation.anims.push_back(animData);
                }
            }

            if (type == "SpritesheetAnimation") {
                gameObject->AddComponent<Experimental::c_SpritesheetAnimation>();
                auto &spritesheetAnimation = gameObject->GetComponent<Experimental::c_SpritesheetAnimation>();

                strcpy(spritesheetAnimation.currAnim, component["currAnim"].get<std::string>().c_str());
                if (component["texture"] != "nullptr") {
                    spritesheetAnimation.mesh->material.diffuse = new Texture(((std::string)component["texture"]).c_str(), 0, "texture_diffuse");
                }

                spritesheetAnimation.spritesheetSize = Vector2(
                    component["spritesheetSize"]["x"],
                    component["spritesheetSize"]["y"]);

                float alpha = 1;

                if (component.contains("color")) {
                    if (component["color"].contains("a")) {
                        alpha = component["color"]["a"];
                    }
                    spritesheetAnimation.mesh->material.baseColor = Vector4(
                        component["color"]["r"],
                        component["color"]["g"],
                        component["color"]["b"],
                        alpha);
                }

                for (auto &animation : component["animations"]) {
                    Experimental::m_SpritesheetAnimationData animData;
                    strcpy(animData.name, animation["name"].get<std::string>().c_str());
                    animData.delay = animation["delay"];
                    animData.loop = animation["loop"];
                    animData.delay_counter = animation["delay_counter"];

                    for (auto &frame : animation["frames"]) {
                        Experimental::m_SpritesheetAnimationData::Frame frameData;
                        frameData.size = Vector2(
                            frame["size"]["x"],
                            frame["size"]["y"]);

                        frameData.offset = Vector2(
                            frame["offset"]["x"],
                            frame["offset"]["y"]);
                        animData.frames.push_back(frameData);
                    }

                    spritesheetAnimation.anims.push_back(animData);
                }
            }

            if (type == "Bloom") {
                auto &bloom = gameObject->AddComponent<Experimental::Bloom>();

                if (component.contains("dynamic_bloom")) {
                    bloom.dynamic_bloom = component["dynamic_bloom"];
                    std::cout << bloom.dynamic_bloom << "what\n";
                } else {
                    bloom.dynamic_bloom = false;
                    std::cout << bloom.dynamic_bloom << "whatnya\n";
                }

                if (component.contains("bloom_threshold"))
                    bloom.bloom_threshold = component["bloom_threshold"];

                bloom.bloomColor = Vector3(
                    component["bloomColor"]["x"],
                    component["bloomColor"]["y"],
                    component["bloomColor"]["z"]);
            }
        }

        void LoadScene(const std::string &scenePath, nlohmann::json &StateScene) {
            HYPER_LOG("scene0");
            stop_scripts = true;
            m_Object = nullptr;
            mainCamera = scene_camera;

            bool was_running = HyperAPI::isRunning;
            isRunning = false;
            isStopped = true;
            LoadingScene = true;

            if (scenePath != "")
                currentScenePath = scenePath;

            HYPER_LOG("scene1")

            try {
                HYPER_LOG("scene2")
                HYPER_LOG(m_GameObjects)
                for (auto &gameObject : *m_GameObjects) {
                    if (gameObject->HasComponent<Experimental::Transform>()) {
                        auto &comp = gameObject->GetComponent<Experimental::Transform>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::CameraComponent>()) {
                        auto &comp = gameObject->GetComponent<Experimental::CameraComponent>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::MeshRenderer>()) {
                        auto &comp = gameObject->GetComponent<Experimental::MeshRenderer>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::m_LuaScriptComponent>()) {
                        auto &comp = gameObject->GetComponent<Experimental::m_LuaScriptComponent>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::CppScriptManager>()) {
                        auto &comp = gameObject->GetComponent<Experimental::CppScriptManager>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::c_PointLight>()) {
                        auto &comp = gameObject->GetComponent<Experimental::c_PointLight>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::c_Light2D>()) {
                        auto &comp = gameObject->GetComponent<Experimental::c_Light2D>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::c_SpotLight>()) {
                        auto &comp = gameObject->GetComponent<Experimental::c_SpotLight>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::c_DirectionalLight>()) {
                        auto &comp = gameObject->GetComponent<Experimental::c_DirectionalLight>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::SpriteRenderer>()) {
                        auto &comp = gameObject->GetComponent<Experimental::SpriteRenderer>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::SpriteAnimation>()) {
                        auto &comp = gameObject->GetComponent<Experimental::SpriteAnimation>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::c_SpritesheetAnimation>()) {
                        auto &comp = gameObject->GetComponent<Experimental::c_SpritesheetAnimation>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::SpritesheetRenderer>()) {
                        auto &comp = gameObject->GetComponent<Experimental::SpritesheetRenderer>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::BoxCollider2D>()) {
                        auto &comp = gameObject->GetComponent<Experimental::BoxCollider2D>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::Rigidbody2D>()) {
                        auto &comp = gameObject->GetComponent<Experimental::Rigidbody2D>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::Rigidbody3D>()) {
                        auto &comp = gameObject->GetComponent<Experimental::Rigidbody3D>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::FixedJoint3D>()) {
                        auto &comp = gameObject->GetComponent<Experimental::FixedJoint3D>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::BoxCollider3D>()) {
                        auto &comp = gameObject->GetComponent<Experimental::BoxCollider3D>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::MeshCollider3D>()) {
                        auto &comp = gameObject->GetComponent<Experimental::MeshCollider3D>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::PathfindingAI>()) {
                        auto &comp = gameObject->GetComponent<Experimental::PathfindingAI>();
                        comp.DeleteComp();
                    }

                    if (gameObject->HasComponent<Experimental::Bloom>()) {
                        auto &comp = gameObject->GetComponent<Experimental::Bloom>();
                        comp.DeleteComp();
                    }

                    m_Registry.remove_all(gameObject->entity);
                    m_Registry.destroy(gameObject->entity);
                    delete gameObject;
                }
                m_GameObjects->clear();

                for (auto &light : DirLights) {
                    delete light;
                }
                DirLights.clear();

                for (auto &light : PointLights) {
                    delete light;
                }
                PointLights.clear();

                for (auto &light : SpotLights) {
                    delete light;
                }
                SpotLights.clear();

                for (auto &light : Lights2D) {
                    delete light;
                }
                Lights2D.clear();

                m_GameObjects->clear();
            } catch (...) {
            }

            std::ifstream file(scenePath);
            // get content
            std::string content;
            if (scenePath != "")
                content = std::string((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());

            nlohmann::json JSON;
            if (scenePath != "")
                JSON = nlohmann::json::parse(content);
            else
                JSON = StateScene;

            try {
                for (int i = 0; i < Scene::cameras.size(); i++) {
                    if (Scene::mainCamera == Scene::cameras[i]) {
                        Scene::mainCamera = nullptr;
                    }
                    delete Scene::cameras[i];
                }
                Scene::cameras.clear();
            } catch (std::exception &e) {
                std::cout << e.what() << std::endl;
            }

            HYPER_LOG("scene3")

            for (int i = 0; i < JSON.size(); i++) {
                Experimental::GameObject *gameObject = new Experimental::GameObject();
                std::string name = JSON[i]["name"];
                std::string ID = JSON[i]["ID"];
                std::string parentID = JSON[i]["parentID"];
                std::string tag = JSON[i]["tag"];
                std::string layer = JSON[i]["layer"];

                gameObject->name = name;
                gameObject->tag = tag;
                gameObject->ID = ID;
                gameObject->parentID = parentID;
                gameObject->layer = layer;

                if (JSON[i].contains("schedule")) {
                    gameObject->schedule = JSON[i]["schedule"];
                }

                if (JSON[i].contains("setted_schedule")) {
                    gameObject->setted_schedule = JSON[i]["setted_schedule"];
                }

                if (JSON[i].contains("active")) {
                    gameObject->SetActive(JSON[i]["active"]);
                }

                nlohmann::json components = JSON[i]["components"];

                std::string meshType = "";

                Experimental::Transform transform;
                m_GameObjects->push_back(gameObject);

                for (auto &component : components) {
                    std::string type = component["type"];

                    LoadComponents(component, gameObject, type, meshType, transform);
                }
            }

            file.close();
            // delete json so that it wont take any space
            JSON.clear();

            if (was_running) {
                Experimental::StartWorld(b2_listener);
                isRunning = true;
                isStopped = false;
            }

            if (Scene::mainCamera == nullptr) {
                Scene::mainCamera = scene_camera;
            }
            LoadingScene = false;
            HYPER_LOG("Loaded scene: " + scenePath);
            stop_scripts = false;
        }
        Experimental::GameObject *LoadPrefab(const std::string &scenePath) {
            std::ifstream file(scenePath);
            // get content
            std::string content((std::istreambuf_iterator<char>(file)), std::istreambuf_iterator<char>());
            nlohmann::json JSON;
            JSON = nlohmann::json::parse(content);

            Experimental::GameObject *parentObject = nullptr;
            for (int i = 0; i < JSON.size(); i++) {
                Experimental::GameObject *gameObject = new Experimental::GameObject();
                gameObject->prefab = true;
                std::string name = (std::string)JSON[i]["name"] + " (Prefab)";
                std::string oldID = JSON[i]["ID"];
                std::string ID = gameObject->ID;
                std::string parentID = JSON[i]["parentID"];
                std::string tag = JSON[i]["tag"];
                std::string layer = JSON[i]["layer"];

                gameObject->name = name;
                gameObject->tag = tag;
                gameObject->parentID = parentID;
                if (gameObject->parentID == "NO_PARENT") {
                    parentObject = gameObject;
                }
                gameObject->layer = layer;
                gameObject->entity = Scene::m_Registry.create();
                nlohmann::json components = JSON[i]["components"];

                std::string meshType = "";

                Experimental::Transform transform;
                m_GameObjects->push_back(gameObject);

                for (auto &component : components) {
                    std::string type = component["type"];

                    LoadComponents(component, gameObject, type, meshType, transform);
                }

                if (parentID != "NO_PARENT") {
                    Experimental::Transform childTransform;

                    delete gameObject;
                    // erase back
                    m_GameObjects->erase(m_GameObjects->begin() + m_GameObjects->size() - 1);
                    int amountOfChildren = 0;
                    std::vector<std::string> childNames;
                    for (int j = 0; j < m_GameObjects->size(); j++) {
                        if ((*m_GameObjects)[j]->ID == parentID) {
                            Experimental::Model *newEntity = new Experimental::Model((char *)meshType.c_str(), false);
                            auto &m_Transform = newEntity->mainGameObject->GetComponent<Experimental::Transform>();

                            for (int g = 0; g < JSON.size(); g++) {
                                std::string p_ID = JSON[g]["ID"];
                                std::string name = JSON[g]["name"];

                                if (parentID == p_ID) {
                                    for (int b = 0; b < JSON.size(); b++) {
                                        std::string c_ID = JSON[b]["parentID"];
                                        std::string c_name = JSON[b]["name"];

                                        if (p_ID == c_ID) {
                                            childNames.push_back(c_name);
                                            amountOfChildren++;
                                        }
                                    }

                                    for (auto &component : JSON[g]["components"]) {
                                        std::string type = component["type"];

                                        if (type == "Transform") {
                                            ApplyTransform(m_Transform, component);
                                            newEntity->mainGameObject->name = name;
                                        }
                                    }
                                }
                            }

                            for (int a = 0; a < amountOfChildren; a++) {
                                newEntity->m_gameObjects[a]->name = childNames[a];
                                newEntity->m_gameObjects[a]->GetComponent<Experimental::Transform>().position = transform.position;
                                newEntity->m_gameObjects[a]->GetComponent<Experimental::Transform>().rotation = transform.rotation;
                                newEntity->m_gameObjects[a]->GetComponent<Experimental::Transform>().scale = transform.scale;
                            }

                            m_GameObjects->erase(m_GameObjects->begin() + j);

                            break;
                        }
                    }
                }
            }

            file.close();

            HYPER_LOG("Loaded Prefab: " + scenePath);
            return parentObject;
        }
        Experimental::GameObject *LoadJSONPrefab(const nlohmann::json &JSON) {
            Experimental::GameObject *parentObject = nullptr;
            for (int i = 0; i < JSON.size(); i++) {
                Experimental::GameObject *gameObject = new Experimental::GameObject();
                std::string name = (std::string)JSON[i]["name"] + " (Prefab)";
                std::string oldID = JSON[i]["ID"];
                std::string ID = gameObject->ID;
                std::string parentID = JSON[i]["parentID"];
                std::string tag = JSON[i]["tag"];
                std::string layer = JSON[i]["layer"];

                gameObject->name = name;
                gameObject->tag = tag;
                gameObject->parentID = parentID;
                if (gameObject->parentID == "NO_PARENT") {
                    parentObject = gameObject;
                }
                gameObject->layer = layer;
                nlohmann::json components = JSON[i]["components"];

                std::string meshType = "";

                Experimental::Transform transform;
                m_GameObjects->push_back(gameObject);

                for (auto &component : components) {
                    std::string type = component["type"];

                    LoadComponents(component, gameObject, type, meshType, transform);
                }

                if (parentID != "NO_PARENT") {
                    Experimental::Transform childTransform;

                    delete gameObject;
                    // erase back
                    m_GameObjects->erase(m_GameObjects->begin() + m_GameObjects->size() - 1);
                    int amountOfChildren = 0;
                    std::vector<std::string> childNames;
                    for (int j = 0; j < m_GameObjects->size(); j++) {
                        if ((*m_GameObjects)[j]->ID == parentID) {
                            Experimental::Model *newEntity = new Experimental::Model((char *)meshType.c_str(), false);
                            auto &m_Transform = newEntity->mainGameObject->GetComponent<Experimental::Transform>();

                            for (int g = 0; g < JSON.size(); g++) {
                                std::string p_ID = JSON[g]["ID"];
                                std::string name = JSON[g]["name"];

                                if (parentID == p_ID) {
                                    for (int b = 0; b < JSON.size(); b++) {
                                        std::string c_ID = JSON[b]["parentID"];
                                        std::string c_name = JSON[b]["name"];

                                        if (p_ID == c_ID) {
                                            childNames.push_back(c_name);
                                            amountOfChildren++;
                                        }
                                    }

                                    for (auto &component : JSON[g]["components"]) {
                                        std::string type = component["type"];

                                        if (type == "Transform") {
                                            ApplyTransform(m_Transform, component);
                                            newEntity->mainGameObject->name = name;
                                        }
                                    }
                                }
                            }

                            for (int a = 0; a < amountOfChildren; a++) {
                                newEntity->m_gameObjects[a]->name = childNames[a];
                                newEntity->m_gameObjects[a]->GetComponent<Experimental::Transform>().position = transform.position;
                                newEntity->m_gameObjects[a]->GetComponent<Experimental::Transform>().rotation = transform.rotation;
                                newEntity->m_gameObjects[a]->GetComponent<Experimental::Transform>().scale = transform.scale;
                            }

                            m_GameObjects->erase(m_GameObjects->begin() + j);

                            break;
                        }
                    }
                }
            }

            HYPER_LOG("Loaded JSON Prefab")
            return parentObject;
        }

        void SaveComponents(nlohmann::json &JSON, Experimental::GameObject *gameObject, const int i, int &componentOffset) {
            if (gameObject->HasComponent<Experimental::Transform>()) {
                auto &transform = gameObject->GetComponent<Experimental::Transform>();
                JSON[i]["components"][componentOffset]["type"] = "Transform";
                JSON[i]["components"][componentOffset]["position"]["x"] = transform.position.x;
                JSON[i]["components"][componentOffset]["position"]["y"] = transform.position.y;
                JSON[i]["components"][componentOffset]["position"]["z"] = transform.position.z;

                JSON[i]["components"][componentOffset]["rotation"]["x"] = transform.rotation.x;
                JSON[i]["components"][componentOffset]["rotation"]["y"] = transform.rotation.y;
                JSON[i]["components"][componentOffset]["rotation"]["z"] = transform.rotation.z;

                JSON[i]["components"][componentOffset]["scale"]["x"] = transform.scale.x;
                JSON[i]["components"][componentOffset]["scale"]["y"] = transform.scale.y;
                JSON[i]["components"][componentOffset]["scale"]["z"] = transform.scale.z;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::MeshRenderer>()) {
                auto &meshRenderer = gameObject->GetComponent<Experimental::MeshRenderer>();

                JSON[i]["components"][componentOffset]["mesh"]["custom"] = meshRenderer.m_Model;
                if (meshRenderer.m_Model) {
                    //                     auto *pvert = &meshRenderer.m_Mesh->vertices;
                    //                     auto *pind = &meshRenderer.m_Mesh->indices;
                    // #ifdef _WIN32
                    //                     std::ofstream file("assets\\models\\scene_files\\" + gameObject->ID + ".vault.model");
                    // #else
                    //                     std::ofstream file("assets/models/scene_files/" + gameObject->ID + ".vault.model");
                    // #endif
                    //                     nlohmann::json j = json::object();
                    //                     j["vertices"] = json::array();
                    //                     j["indices"] = json::array();
                    //                     for (int vi = 0; vi < (*pvert).size(); vi++) {
                    //                         auto &vertex = (*pvert)[vi];
                    //                         j["vertices"][vi] = {
                    //                             {"position", {{"x", vertex.position.x}, {"y", vertex.position.y}, {"z", vertex.position.z}}},
                    //                             {"bitangent", {{"x", vertex.bitangent.x}, {"y", vertex.bitangent.y}, {"z", vertex.bitangent.z}}},
                    //                             {"tangent", {{"x", vertex.tangent.x}, {"y", vertex.tangent.y}, {"z", vertex.tangent.z}}},
                    //                             {"color", {{"x", vertex.color.x}, {"y", vertex.color.y}, {"z", vertex.color.z}}},
                    //                             {"normal", {{"x", vertex.normal.x}, {"y", vertex.normal.y}, {"z", vertex.normal.z}}},
                    //                             {"texUV", {{"x", vertex.texUV.x}, {"y", vertex.texUV.y}}},
                    //                         };
                    //                     }

                    //                     for (int ii = 0; ii < (*pind).size(); ii++) {
                    //                         auto &index = (*pind)[ii];
                    //                         j["indices"][ii] = index;
                    //                     }

                    //                     file << j.dump(4);
                }

                std::cout << meshRenderer.meshType << "\n";

                JSON[i]["components"][componentOffset]["type"] = "MeshRenderer";
                JSON[i]["components"][componentOffset]["mesh"]["mesh"] = meshRenderer.meshType;
                JSON[i]["components"][componentOffset]["material"] = meshRenderer.matPath;
                JSON[i]["components"][componentOffset]["mesh"]["mesh_index"] = meshRenderer.mesh_index;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::AudioListener>()) {
                JSON[i]["components"][componentOffset]["type"] = "AudioListener";
                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::Audio3D>()) {
                auto &aud = gameObject->GetComponent<Experimental::Audio3D>();
                JSON[i]["components"][componentOffset]["type"] = "Audio3D";
                JSON[i]["components"][componentOffset]["pitch"] = aud.src.pitch;
                JSON[i]["components"][componentOffset]["volume"] = aud.src.volume;
                JSON[i]["components"][componentOffset]["on_start"] = aud.on_start;
                JSON[i]["components"][componentOffset]["max_distance"] = aud.src.max_distance;
                JSON[i]["components"][componentOffset]["loop"] = aud.src.loop;
                JSON[i]["components"][componentOffset]["file"] = aud.file;
                JSON[i]["components"][componentOffset]["velocity"] = {
                    {"x", aud.src.velocity.x},
                    {"y", aud.src.velocity.y},
                    {"z", aud.src.velocity.z},
                };
                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::Text3D>()) {
                auto &text = gameObject->GetComponent<Experimental::Text3D>();
                JSON[i]["components"][componentOffset]["type"] = "Text3D";
                JSON[i]["components"][componentOffset]["text"] = text.text;
                JSON[i]["components"][componentOffset]["color"] = {
                    {"r", text.color.r},
                    {"g", text.color.g},
                    {"b", text.color.b}};

                JSON[i]["components"][componentOffset]["scale"] = text.scale;
                JSON[i]["components"][componentOffset]["y_offset"] = text.y_offset;
                JSON[i]["components"][componentOffset]["font"] = text.font->font_path;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::c_DirectionalLight>()) {
                auto &light = gameObject->GetComponent<Experimental::c_DirectionalLight>();

                JSON[i]["components"][componentOffset]["type"] = "DirectionalLight";
                JSON[i]["components"][componentOffset]["color"]["r"] = light.color.x;
                JSON[i]["components"][componentOffset]["color"]["g"] = light.color.y;
                JSON[i]["components"][componentOffset]["color"]["b"] = light.color.z;
                JSON[i]["components"][componentOffset]["intensity"] = light.intensity;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::c_PointLight>()) {
                auto &light = gameObject->GetComponent<Experimental::c_PointLight>();

                JSON[i]["components"][componentOffset]["type"] = "PointLight";
                JSON[i]["components"][componentOffset]["color"]["r"] = light.color.x;
                JSON[i]["components"][componentOffset]["color"]["g"] = light.color.y;
                JSON[i]["components"][componentOffset]["color"]["b"] = light.color.z;
                JSON[i]["components"][componentOffset]["intensity"] = light.intensity;
                JSON[i]["components"][componentOffset]["renderShadows"] = light.light->renderShadows;
                JSON[i]["components"][componentOffset]["farPlane"] = light.light->farPlane;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::c_SpotLight>()) {
                auto &light = gameObject->GetComponent<Experimental::c_SpotLight>();

                JSON[i]["components"][componentOffset]["type"] = "SpotLight";
                JSON[i]["components"][componentOffset]["color"]["r"] = light.color.x;
                JSON[i]["components"][componentOffset]["color"]["g"] = light.color.y;
                JSON[i]["components"][componentOffset]["color"]["b"] = light.color.z;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::c_Light2D>()) {
                auto &light = gameObject->GetComponent<Experimental::c_Light2D>();

                JSON[i]["components"][componentOffset]["type"] = "Light2D";
                JSON[i]["components"][componentOffset]["color"]["r"] = light.color.x;
                JSON[i]["components"][componentOffset]["color"]["g"] = light.color.y;
                JSON[i]["components"][componentOffset]["color"]["b"] = light.color.z;
                JSON[i]["components"][componentOffset]["range"] = light.range;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::CameraComponent>()) {
                auto &camera = gameObject->GetComponent<Experimental::CameraComponent>();

                JSON[i]["components"][componentOffset]["type"] = "CameraComponent";
                JSON[i]["components"][componentOffset]["mainCamera"] = camera.camera->main_camera;
                JSON[i]["components"][componentOffset]["mode2D"] = camera.camera->mode2D;
                JSON[i]["components"][componentOffset]["fov"] = camera.camera->cam_fov;
                JSON[i]["components"][componentOffset]["near"] = camera.camera->cam_near;
                JSON[i]["components"][componentOffset]["far"] = camera.camera->cam_far;
                JSON[i]["components"][componentOffset]["layers"] = camera.camera->layers;
                JSON[i]["components"][componentOffset]["depthCamera"] = camera.depthCamera;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::m_LuaScriptComponent>()) {
                auto &script = gameObject->GetComponent<Experimental::m_LuaScriptComponent>();

                JSON[i]["components"][componentOffset]["type"] = "LuaScriptComponent";
                for (int scr = 0; scr < script.scripts.size(); scr++) {
                    JSON[i]["components"][componentOffset]["scripts"][scr] = script.scripts[scr].pathToScript;
                    JSON[i]["components"][componentOffset]["script_fields"][script.scripts[scr].pathToScript] = json::object();

                    for (auto &field : script.scripts[scr].m_Fields) {
                        JSON[i]["components"][componentOffset]["script_fields"][script.scripts[scr].pathToScript][field.first] = field.second.value;
                    }
                }

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::CppScriptManager>()) {
                auto &scriptManager = gameObject->GetComponent<Experimental::CppScriptManager>();
                JSON[i]["components"][componentOffset]["type"] = "CppScriptManager";
                for (int scr = 0; scr < scriptManager.addedScripts.size(); scr++) {
                    JSON[i]["components"][componentOffset]["scripts"][scr] = scriptManager.addedScripts[scr]->name;
                }

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::CsharpScriptManager>()) {
                auto &scriptManager = gameObject->GetComponent<Experimental::CsharpScriptManager>();
                JSON[i]["components"][componentOffset]["type"] = "CsharpScriptManager";

                int index = 0;
                for (auto klass : scriptManager.selectedScripts) {
                    if (klass.second == "")
                        continue;
                    JSON[i]["components"][componentOffset]["scripts"][index] = klass.second;
                    index++;
                }

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::SpriteRenderer>()) {
                auto &spriteRenderer = gameObject->GetComponent<Experimental::SpriteRenderer>();

                JSON[i]["components"][componentOffset]["type"] = "SpriteRenderer";
                JSON[i]["components"][componentOffset]["sprite"] = spriteRenderer.mesh->material.diffuse == nullptr ? "" : spriteRenderer.mesh->material.diffuse->texPath;
                JSON[i]["components"][componentOffset]["color"]["r"] = spriteRenderer.mesh->material.baseColor.x;
                JSON[i]["components"][componentOffset]["color"]["g"] = spriteRenderer.mesh->material.baseColor.y;
                JSON[i]["components"][componentOffset]["color"]["b"] = spriteRenderer.mesh->material.baseColor.z;
                JSON[i]["components"][componentOffset]["color"]["a"] = spriteRenderer.mesh->material.baseColor.w;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::SpriteAnimation>()) {
                auto &spriteAnimation = gameObject->GetComponent<Experimental::SpriteAnimation>();

                JSON[i]["components"][componentOffset]["type"] = "SpriteAnimation";
                JSON[i]["components"][componentOffset]["currAnim"] = std::string(spriteAnimation.currAnim);
                JSON[i]["components"][componentOffset]["animations"] = json::array();

                for (auto &anim : spriteAnimation.anims) {
                    int in = &anim - &spriteAnimation.anims[0];

                    JSON[i]["components"][componentOffset]["animations"][in]["name"] = anim.name;
                    JSON[i]["components"][componentOffset]["animations"][in]["delay"] = anim.delay;
                    JSON[i]["components"][componentOffset]["animations"][in]["delay_counter"] = anim.delay_counter;
                    JSON[i]["components"][componentOffset]["animations"][in]["loop"] = anim.loop;
                    JSON[i]["components"][componentOffset]["animations"][in]["frames"] = json::array();

                    for (auto &frame : anim.frames) {
                        int fn = &frame - &anim.frames[0];
                        JSON[i]["components"][componentOffset]["animations"][in]["frames"][fn]["texture"] = frame.mesh->material.diffuse == nullptr ? "nullptr" : frame.mesh->material.diffuse->texPath;
                        JSON[i]["components"][componentOffset]["animations"][in]["frames"][fn]["color"]["r"] = frame.mesh->material.baseColor.x;
                        JSON[i]["components"][componentOffset]["animations"][in]["frames"][fn]["color"]["g"] = frame.mesh->material.baseColor.y;
                        JSON[i]["components"][componentOffset]["animations"][in]["frames"][fn]["color"]["b"] = frame.mesh->material.baseColor.z;
                        JSON[i]["components"][componentOffset]["animations"][in]["frames"][fn]["color"]["a"] = frame.mesh->material.baseColor.w;
                    }
                }
                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::c_SpritesheetAnimation>()) {
                auto &spriteAnimation = gameObject->GetComponent<Experimental::c_SpritesheetAnimation>();

                JSON[i]["components"][componentOffset]["type"] = "SpritesheetAnimation";
                JSON[i]["components"][componentOffset]["currAnim"] = std::string(spriteAnimation.currAnim);
                JSON[i]["components"][componentOffset]["animations"] = json::array();
                JSON[i]["components"][componentOffset]["spritesheetSize"]["x"] = spriteAnimation.spritesheetSize.x;
                JSON[i]["components"][componentOffset]["spritesheetSize"]["y"] = spriteAnimation.spritesheetSize.y;
                JSON[i]["components"][componentOffset]["texture"] = spriteAnimation.mesh->material.diffuse == nullptr ? "nullptr" : spriteAnimation.mesh->material.diffuse->texPath;

                JSON[i]["components"][componentOffset]["color"]["r"] = spriteAnimation.mesh->material.baseColor.x;
                JSON[i]["components"][componentOffset]["color"]["g"] = spriteAnimation.mesh->material.baseColor.y;
                JSON[i]["components"][componentOffset]["color"]["b"] = spriteAnimation.mesh->material.baseColor.z;
                JSON[i]["components"][componentOffset]["color"]["a"] = spriteAnimation.mesh->material.baseColor.w;

                for (auto &anim : spriteAnimation.anims) {
                    int in = &anim - &spriteAnimation.anims[0];
                    JSON[i]["components"][componentOffset]["animations"][in]["name"] = std::string(anim.name);
                    JSON[i]["components"][componentOffset]["animations"][in]["delay"] = anim.delay;
                    JSON[i]["components"][componentOffset]["animations"][in]["loop"] = anim.loop;
                    JSON[i]["components"][componentOffset]["animations"][in]["delay_counter"] = anim.delay_counter;
                    JSON[i]["components"][componentOffset]["animations"][in]["frames"] = json::array();
                    for (auto &frame : anim.frames) {
                        int index = &frame - &anim.frames[0];
                        JSON[i]["components"][componentOffset]["animations"][in]["frames"][index]["size"] = {
                            {"x", frame.size.x},
                            {"y", frame.size.y}};

                        JSON[i]["components"][componentOffset]["animations"][in]["frames"][index]["offset"] = {
                            {"x", frame.offset.x},
                            {"y", frame.offset.y}};
                    }
                }
                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::Rigidbody2D>()) {
                auto &rigidbody = gameObject->GetComponent<Experimental::Rigidbody2D>();

                JSON[i]["components"][componentOffset]["type"] = "Rigidbody2D";
                JSON[i]["components"][componentOffset]["gravityScale"] = rigidbody.gravityScale;
                JSON[i]["components"][componentOffset]["bodyType"] = rigidbody.type;
                JSON[i]["components"][componentOffset]["fixedRotation"] = rigidbody.fixedRotation;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::BoxCollider2D>()) {
                auto &collider = gameObject->GetComponent<Experimental::BoxCollider2D>();

                JSON[i]["components"][componentOffset]["type"] = "BoxCollider2D";
                JSON[i]["components"][componentOffset]["density"] = collider.density;
                JSON[i]["components"][componentOffset]["friction"] = collider.friction;
                JSON[i]["components"][componentOffset]["trigger"] = collider.trigger;
                JSON[i]["components"][componentOffset]["restitution"] = collider.restitution;
                JSON[i]["components"][componentOffset]["restitutionThreshold"] = collider.restitutionThreshold;
                JSON[i]["components"][componentOffset]["size"] = {
                    {"x", collider.size.x},
                    {"y", collider.size.y}};

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::SpritesheetRenderer>()) {
                auto &spritesheetRenderer = gameObject->GetComponent<Experimental::SpritesheetRenderer>();

                JSON[i]["components"][componentOffset]["type"] = "SpritesheetRenderer";
                JSON[i]["components"][componentOffset]["color"]["r"] = spritesheetRenderer.material.baseColor.x;
                JSON[i]["components"][componentOffset]["color"]["g"] = spritesheetRenderer.material.baseColor.y;
                JSON[i]["components"][componentOffset]["color"]["b"] = spritesheetRenderer.material.baseColor.z;

                JSON[i]["components"][componentOffset]["spritesheetSize"] = {
                    {"x", spritesheetRenderer.spritesheetSize.x},
                    {"y", spritesheetRenderer.spritesheetSize.y}};

                JSON[i]["components"][componentOffset]["spriteSize"] = {
                    {"x", spritesheetRenderer.spriteSize.x},
                    {"y", spritesheetRenderer.spriteSize.y}};

                JSON[i]["components"][componentOffset]["spriteOffset"] = {
                    {"x", spritesheetRenderer.spriteOffset.x},
                    {"y", spritesheetRenderer.spriteOffset.y}};

                JSON[i]["components"][componentOffset]["spritesheet"] = spritesheetRenderer.mesh->material.diffuse == nullptr ? "" : spritesheetRenderer.mesh->material.diffuse->texPath;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::Rigidbody3D>()) {
                auto &rigidbody = gameObject->GetComponent<Experimental::Rigidbody3D>();

                JSON[i]["components"][componentOffset]["type"] = "Rigidbody3D";
                JSON[i]["components"][componentOffset]["mass"] = rigidbody.mass;
                JSON[i]["components"][componentOffset]["friction"] = rigidbody.friction;
                JSON[i]["components"][componentOffset]["restitution"] = rigidbody.restitution;
                JSON[i]["components"][componentOffset]["fixedRotation"] = rigidbody.fixedRotation;
                JSON[i]["components"][componentOffset]["trigger"] = rigidbody.trigger;

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::BoxCollider3D>()) {
                auto &collider = gameObject->GetComponent<Experimental::BoxCollider3D>();

                JSON[i]["components"][componentOffset]["type"] = "BoxCollider3D";
                JSON[i]["components"][componentOffset]["size"] = {
                    {"x", collider.size.x},
                    {"y", collider.size.y},
                    {"z", collider.size.z}};

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::MeshCollider3D>()) {
                auto &collider = gameObject->GetComponent<Experimental::MeshCollider3D>();
                JSON[i]["components"][componentOffset]["type"] = "MeshCollider3D";
                JSON[i]["components"][componentOffset]["size"] = {
                    {"x", collider.size.x},
                    {"y", collider.size.y},
                    {"z", collider.size.z}};

                componentOffset++;
            }

            if (gameObject->HasComponent<Experimental::Bloom>()) {
                auto &bloom = gameObject->GetComponent<Experimental::Bloom>();
                JSON[i]["components"][componentOffset]["type"] = "Bloom";
                JSON[i]["components"][componentOffset]["bloom_threshold"] = bloom.bloom_threshold;
                JSON[i]["components"][componentOffset]["dynamic_bloom"] = bloom.dynamic_bloom;
                JSON[i]["components"][componentOffset]["bloomColor"] = {
                    {"x", bloom.bloomColor.x},
                    {"y", bloom.bloomColor.y},
                    {"z", bloom.bloomColor.z}};

                componentOffset++;
            }
        }

        void SaveScene(const std::string &path, nlohmann::json &StateScene) {
            std::ofstream file(path);
            nlohmann::json JSON;

            int offset = 0;

            for (int i = 0; i < m_GameObjects->size(); i++) {
                auto &gameObject = (*m_GameObjects)[i];

                std::string name = gameObject->name;
                std::string ID = gameObject->ID;
                std::string parentID = gameObject->parentID;
                std::string tag = gameObject->tag;
                std::string layer = gameObject->layer;

                JSON[i]["name"] = name;
                JSON[i]["ID"] = ID;
                JSON[i]["tag"] = tag;
                JSON[i]["parentID"] = parentID;
                JSON[i]["layer"] = layer;
                JSON[i]["schedule"] = gameObject->schedule;
                JSON[i]["setted_schedule"] = gameObject->setted_schedule;
                JSON[i]["components"] = nlohmann::json::array();
                JSON[i]["active"] = gameObject->enabled;

                int componentOffset = 0;
                SaveComponents(JSON, gameObject, i, componentOffset);
            }

            if (path != "")
                file << JSON;
            else
                StateScene = JSON;

            file.close();
            JSON.clear();

            HYPER_LOG("Scene saved to " + path);
        }

        bool DropTargetMat(DragType type, Mesh *currEntity, Texture *otherData) {
            if (ImGui::BeginDragDropTarget()) {
                if (type == DragType::DRAG_GAMEOBJECT) {
                    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("game_object")) {
                        for (auto &obj : *m_GameObjects) {
                            if (obj->ID == dirPayloadData && type == DRAG_GAMEOBJECT) {
                                SavePrefab((std::string("assets/") + obj->name + std::string(".prefab")).c_str(), obj);
                                break;
                            }
                        }
                    }
                }

                if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("file")) {
                    // const char* path = (const char*)payload->Data;
                    if (type != DRAG_GAMEOBJECT) {
                        dirPayloadData.erase(0, cwd.length() + 1);
                    }

                    if (
                        G_END_WITH(dirPayloadData, ".png") ||
                        G_END_WITH(dirPayloadData, ".jpg") ||
                        G_END_WITH(dirPayloadData, ".jpeg")) {
                        switch (type) {
                        case DRAG_DIFFUSE: {
                            currEntity->material.diffuse = new Texture((char *)dirPayloadData.c_str(), 0, "texture_diffuse");
                            break;
                        }
                        case DRAG_SPECULAR: {
                            currEntity->material.specular = new Texture((char *)dirPayloadData.c_str(), 1, "texture_specular");
                            break;
                        }
                        case DRAG_NORMAL: {
                            currEntity->material.normal = new Texture((char *)dirPayloadData.c_str(), 2, "texture_normal");
                            break;
                        }
                        case DRAG_SPRITE: {
                            if (currEntity) {
                                currEntity->material.diffuse = new Texture((char *)dirPayloadData.c_str(), 0, "texture_diffuse");
                            }

                            if (otherData) {
                                otherData = new Texture((char *)dirPayloadData.c_str(), 0, "texture_diffuse");
                            }

                            break;
                        }
                        case DRAG_SPRITE_NO_MESH: {
                            otherData = new Texture((char *)dirPayloadData.c_str(), 0, "texture_diffuse");
                        }
                        }
                    }

                    std::cout << "Dropped " << dirPayloadData << std::endl;
                    if (
                        G_END_WITH(dirPayloadData, ".obj") ||
                        G_END_WITH(dirPayloadData, ".fbx") ||
                        G_END_WITH(dirPayloadData, ".gltf") ||
                        G_END_WITH(dirPayloadData, ".glb") ||
                        G_END_WITH(dirPayloadData, ".blend") ||
                        G_END_WITH(dirPayloadData, ".dae") ||
                        G_END_WITH(dirPayloadData, ".3ds") ||
                        G_END_WITH(dirPayloadData, ".ase") ||
                        G_END_WITH(dirPayloadData, ".ifc") ||
                        G_END_WITH(dirPayloadData, ".xgl") ||
                        G_END_WITH(dirPayloadData, ".zgl")) {
                        switch (type) {
                        case DRAG_MODEL: {
                            Experimental::Model *newEntity = new Experimental::Model((char *)dirPayloadData.c_str(), false);
                            break;
                        }
                        }
                    }

                    if (
                        G_END_WITH(dirPayloadData, ".vault") && type == DRAG_SCENE) {
                        nlohmann::json J;
                        LoadScene(dirPayloadData, J);
                    }

                    if (
                        G_END_WITH(dirPayloadData, ".prefab") && type == DRAG_MODEL) {
                        LoadPrefab(dirPayloadData);
                    }

                    for (auto &obj : *m_GameObjects) {
                        if (obj->ID == dirPayloadData && type == DRAG_GAMEOBJECT) {
                            SavePrefab((std::string("assets/") + obj->name + std::string(".prefab")).c_str(), obj);
                            break;
                        }
                    }

                    return true;
                }

                ImGui::EndDragDropTarget();
            }

            return false;
        }

        DLL_API std::string currentScenePath = "";
        DLL_API ENTT_EXPORT entt::registry m_Registry;
        DLL_API std::map<std::string, bool> layers;
        DLL_API std::vector<Experimental::GameObject *> *m_GameObjects = new std::vector<Experimental::GameObject *>({});

        DLL_API Experimental::GameObject *m_Object = nullptr;
        DLL_API char name[499];
        DLL_API char tag[499];
        DLL_API char layer[32];

        DLL_API std::vector<Mesh *> entities = {};
        DLL_API std::vector<Model> models = {};
        DLL_API Camera *mainCamera;
        DLL_API Camera *scene_camera;
        DLL_API bool stop_scripts = false;
        DLL_API std::vector<Camera *> cameras = {};
        DLL_API std::vector<Log> logs = {};
#ifdef _WIN32
#ifndef BUILD_DLL
        std::vector<Log> *GetLogs() {
            return &logs;
        }
#endif
#endif
        glm::mat4 projection = glm::mat4(1.0f);

        std::vector<entt::entity> backup_entities = {};

        DLL_API std::vector<HyperAPI::PointLight *> PointLights = {};
        DLL_API std::vector<HyperAPI::Light2D *> Lights2D = {};
        DLL_API std::vector<HyperAPI::SpotLight *> SpotLights = {};
        DLL_API std::vector<HyperAPI::DirectionalLight *> DirLights = {};
        DLL_API std::vector<HyperAPI::Mesh *> hyperEntities = {};
        DLL_API std::map<std::string, std::map<std::string, int>> currFrames;
        DLL_API std::map<std::string, std::map<std::string, float>> currDelays;

        DLL_API bool LoadingScene = false;

        DLL_API b2World *world = nullptr;
        DLL_API std::vector<Experimental::GameObject *> m_UIObjects;
        DLL_API int aspect_width = 1920, aspect_height = 1080;
    } // namespace Scene
} // namespace HyperAPI