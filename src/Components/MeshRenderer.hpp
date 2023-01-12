#pragma once
#include <libs.hpp>
#include "Exp_Base.hpp"
#include "../Renderer/Structures.hpp"
#include "../Renderer/OldStuff.hpp"
#include "Bloom.hpp"
#include "../Renderer/Shapes.hpp"

namespace HyperAPI::Experimental {
    struct MeshRenderer : public BaseComponent {
        Mesh *m_Mesh;
        bool m_Model = false;

        glm::mat4 extraMatrix = glm::mat4(1.0f);
        std::string matPath = "";
        std::string meshType = "";

        struct CustomShader {
            bool usingCustomShader = false;
            Shader *shader = nullptr;
        } customShader;

        MeshRenderer() = default;
        void DeleteComp() override {
            if (m_Mesh != nullptr) {
                delete m_Mesh;
            }
        }

        void GUI() override {
            if (ImGui::TreeNode("Mesh Renderer")) {
                if (!m_Model) {
                    if (ImGui::TreeNode("Mesh")) {
                        ImVec2 windowSize = ImGui::GetWindowSize();
                        if (ImGui::Button("Select Mesh")) {
                            ImGui::OpenPopup("Select Mesh");
                        }

                        if (ImGui::BeginPopup("Select Mesh")) {
                            ImVec2 windowSize = ImGui::GetWindowSize();

                            if (ImGui::Button("Plane", ImVec2(200, 0))) {
                                meshType = "Plane";
                                m_Mesh = Plane(Vector4(1, 1, 1, 1)).m_Mesh;
                                ImGui::CloseCurrentPopup();
                            }

                            if (ImGui::Button("Cube", ImVec2(200, 0))) {
                                meshType = "Cube";
                                m_Mesh = Cube(Vector4(1, 1, 1, 1)).meshes[0];
                                ImGui::CloseCurrentPopup();
                            }

                            if (ImGui::Button("Sphere", ImVec2(200, 0))) {
                                meshType = "Sphere";
                                m_Mesh = Sphere(Vector4(1, 1, 1, 1)).meshes[0];
                                ImGui::CloseCurrentPopup();
                            }

                            if (ImGui::Button("Cone", ImVec2(200, 0))) {
                                meshType = "Cone";
                                m_Mesh = Cone(Vector4(1, 1, 1, 1)).meshes[0];
                                ImGui::CloseCurrentPopup();
                            }

                            if (ImGui::Button("Capsule", ImVec2(200, 0))) {
                                meshType = "Capsule";
                                m_Mesh = Capsule(Vector4(1, 1, 1, 1)).meshes[0];
                                ImGui::CloseCurrentPopup();
                            }

                            if (ImGui::Button("Torus", ImVec2(200, 0))) {
                                meshType = "Torus";
                                m_Mesh = Torus(Vector4(1, 1, 1, 1)).meshes[0];
                                ImGui::CloseCurrentPopup();
                            }

                            if (ImGui::Button("Cylinder", ImVec2(200, 0))) {
                                meshType = "Cylinder";
                                m_Mesh =
                                    Cylinder(Vector4(1, 1, 1, 1)).meshes[0];
                                ImGui::CloseCurrentPopup();
                            }

                            ImGui::EndPopup();
                        }

                        ImGui::TreePop();
                    }
                }

                if (m_Mesh != nullptr) {
                    if (ImGui::Button("Select Material")) {
                        ImGuiFileDialog::Instance()->OpenDialog(
                            "SelectMaterial", "Select Material", ".material",
                            ".");
                    }
                    ImGui::NewLine();

                    if (!customShader.usingCustomShader) {
                        ImGui::Button("Drag Shader Here");
                    } else {
                        // set std::experimental::filesystem as fs
                        namespace fs = std::experimental::filesystem;
                        fs::path path = customShader.shader->path;

                        if (ImGui::Button(
                                std::string(
                                    std::string("Click to remove shader: ") +
                                    path.filename().string())
                                    .c_str())) {
                            customShader.usingCustomShader = false;
                            delete customShader.shader;
                            customShader.shader = nullptr;
                        }
                    }

                    if (ImGui::BeginDragDropTarget()) {
                        if (const ImGuiPayload *payload =
                                ImGui::AcceptDragDropPayload("file")) {
                            if (G_END_WITH(dirPayloadData, ".glsl")) {
                                customShader.usingCustomShader = true;
                                customShader.shader =
                                    new Shader(dirPayloadData.c_str());
                            }
                        }
                    }
                }

                if (matPath != "") {
                    ImGui::Text("Material: %s", matPath.c_str());
                    if (ImGui::Button("Remove Material")) {
                        matPath = "";
                        if (m_Mesh->material.diffuse != nullptr) {
                            delete m_Mesh->material.diffuse;
                        }

                        if (m_Mesh->material.specular != nullptr) {
                            delete m_Mesh->material.specular;
                        }

                        if (m_Mesh->material.normal != nullptr) {
                            delete m_Mesh->material.normal;
                        }

                        m_Mesh->material.roughness = 0.0f;
                        m_Mesh->material.metallic = 0.0f;
                        m_Mesh->material.texUVs = Vector2(0, 0);
                    }
                }

                if (ImGuiFileDialog::Instance()->Display("SelectMaterial")) {
                    // action if OK
                    if (ImGuiFileDialog::Instance()->IsOk()) {
                        std::string filePathName =
                            ImGuiFileDialog::Instance()->GetFilePathName();
                        // remove cwd from filePathName
                        filePathName.erase(0, cwd.length() + 1);
                        std::string filePath =
                            ImGuiFileDialog::Instance()->GetCurrentPath();

                        std::ifstream file(filePathName);
                        nlohmann::json JSON = nlohmann::json::parse(file);

                        const std::string diffuseTexture = JSON["diffuse"];
                        const std::string specularTexture = JSON["specular"];
                        const std::string normalTexture = JSON["normal"];
                        std::string heightTexture = "nullptr";
                        if (JSON.contains("height")) {
                            heightTexture = JSON["height"];
                        }

                        if (diffuseTexture != "nullptr") {
                            if (m_Mesh->material.diffuse != nullptr) {
                                delete m_Mesh->material.diffuse;
                            }

                            m_Mesh->material.diffuse = new Texture(
                                diffuseTexture.c_str(), 0, "texture_diffuse");
                        }

                        if (specularTexture != "nullptr") {
                            if (m_Mesh->material.specular != nullptr) {
                                delete m_Mesh->material.specular;
                            }

                            m_Mesh->material.specular = new Texture(
                                specularTexture.c_str(), 1, "texture_specular");
                        }

                        if (normalTexture != "nullptr") {
                            if (m_Mesh->material.normal != nullptr) {
                                delete m_Mesh->material.normal;
                            }

                            m_Mesh->material.normal = new Texture(
                                normalTexture.c_str(), 2, "texture_normal");
                        }

                        if (heightTexture != "nullptr") {
                            if (m_Mesh->material.height != nullptr) {
                                delete m_Mesh->material.height;
                            }

                            m_Mesh->material.height = new Texture(
                                heightTexture.c_str(), 2, "texture_normal");
                        }

                        m_Mesh->material.baseColor = Vector4(
                            JSON["baseColor"]["r"], JSON["baseColor"]["g"],
                            JSON["baseColor"]["b"], JSON["baseColor"]["a"]);

                        m_Mesh->material.roughness = JSON["roughness"];
                        m_Mesh->material.metallic = JSON["metallic"];
                        m_Mesh->material.texUVs =
                            Vector2(JSON["texUV"]["x"], JSON["texUV"]["y"]);

                        matPath = filePathName;
                        file.close();
                    }

                    // close
                    ImGuiFileDialog::Instance()->Close();
                }

                ImGui::NewLine();
                if (ImGui::Button(ICON_FA_TRASH " Remove Component")) {
                    DeleteComp();
                    Scene::m_Registry.remove<MeshRenderer>(entity);
                }

                ImGui::TreePop();
            }
        }

        void Update() {
            if (m_Mesh != nullptr) {
                if (Scene::m_Registry.has<Bloom>(entity)) {
                    auto &bloom = Scene::m_Registry.get<Bloom>(entity);
                    m_Mesh->material.bloomColor = bloom.bloomColor;
                } else {
                    m_Mesh->material.bloomColor = Vector3(0, 0, 0);
                }
            }

            if (std::experimental::filesystem::exists(matPath) &&
                matPath != "" && m_Mesh != nullptr) {
                std::ifstream file(matPath);
                nlohmann::json JSON = nlohmann::json::parse(file);

                const std::string diffuseTexture = JSON["diffuse"];
                const std::string specularTexture = JSON["specular"];
                const std::string normalTexture = JSON["normal"];
                std::string heightTexture = "nullptr";
                if (JSON.contains("height")) {
                    heightTexture = JSON["height"];
                }

                if (diffuseTexture != "nullptr") {
                    if (m_Mesh->material.diffuse != nullptr) {
                        if (m_Mesh->material.diffuse->texPath !=
                            diffuseTexture) {
                            delete m_Mesh->material.diffuse;
                            m_Mesh->material.diffuse = new Texture(
                                diffuseTexture.c_str(), 0, "texture_diffuse");
                        }
                    } else {
                        m_Mesh->material.diffuse = new Texture(
                            diffuseTexture.c_str(), 0, "texture_diffuse");
                    }
                } else {
                    if (m_Mesh->material.diffuse != nullptr) {
                        delete m_Mesh->material.diffuse;
                        m_Mesh->material.diffuse = nullptr;
                    }
                }

                if (specularTexture != "nullptr") {
                    if (m_Mesh->material.specular != nullptr) {
                        if (m_Mesh->material.specular->texPath !=
                            specularTexture) {
                            delete m_Mesh->material.specular;
                            m_Mesh->material.specular = new Texture(
                                specularTexture.c_str(), 1, "texture_specular");
                        }
                    } else {
                        m_Mesh->material.specular = new Texture(
                            specularTexture.c_str(), 1, "texture_specular");
                    }
                } else {
                    if (m_Mesh->material.specular != nullptr) {
                        delete m_Mesh->material.specular;
                        m_Mesh->material.specular = nullptr;
                    }
                }

                if (normalTexture != "nullptr") {
                    if (m_Mesh->material.normal != nullptr) {
                        if (m_Mesh->material.normal->texPath != normalTexture) {
                            delete m_Mesh->material.normal;
                            m_Mesh->material.normal = new Texture(
                                normalTexture.c_str(), 2, "texture_normal");
                        }
                    } else {
                        m_Mesh->material.normal = new Texture(
                            normalTexture.c_str(), 2, "texture_normal");
                    }
                } else {
                    if (m_Mesh->material.normal != nullptr) {
                        delete m_Mesh->material.normal;
                        m_Mesh->material.normal = nullptr;
                    }
                }

                if (heightTexture != "nullptr") {
                    if (m_Mesh->material.emission != nullptr) {
                        if (m_Mesh->material.emission->texPath !=
                            heightTexture) {
                            delete m_Mesh->material.emission;
                            m_Mesh->material.emission = new Texture(
                                heightTexture.c_str(), 4, "texture_emission");
                        }
                    } else {
                        m_Mesh->material.emission = new Texture(
                            heightTexture.c_str(), 4, "texture_emission");
                    }
                } else {
                    if (m_Mesh->material.emission != nullptr) {
                        delete m_Mesh->material.emission;
                        m_Mesh->material.emission = nullptr;
                    }
                }

                m_Mesh->material.baseColor =
                    Vector4(JSON["baseColor"]["r"], JSON["baseColor"]["g"],
                            JSON["baseColor"]["b"], JSON["baseColor"]["a"]);

                m_Mesh->material.roughness = JSON["roughness"];
                m_Mesh->material.metallic = JSON["metallic"];
                m_Mesh->material.texUVs =
                    Vector2(JSON["texUV"]["x"], JSON["texUV"]["y"]);

                file.close();
            } else if (m_Mesh != nullptr) {
                m_Mesh->material.diffuse = nullptr;
                m_Mesh->material.specular = nullptr;
                m_Mesh->material.normal = nullptr;
                m_Mesh->material.height = nullptr;
            }
        }
    };
} // namespace HyperAPI::Experimental