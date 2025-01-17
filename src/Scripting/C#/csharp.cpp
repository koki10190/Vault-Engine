#include <api.hpp>
#include "mono/metadata/appdomain.h"
#include "mono/metadata/assembly.h"
#include "mono/metadata/class.h"
#include <csharp.hpp>
#include "mono/metadata/image.h"
#include "mono/metadata/loader.h"
#include "mono/metadata/object-forward.h"
#include "mono/metadata/object.h"
#include "mono/metadata/mono-debug.h"
#include <cstddef>
#include <cstring>
#include <experimental/bits/fs_fwd.h>
#include <experimental/bits/fs_ops.h>
#include <functional>
#include <sstream>
#include <string>

#ifdef _WIN32
#include <winbase.h>
#endif

// Functions
#include "InputFunctions.hpp"
#include "LogFunctions.hpp"
#include "AudioFunctions.hpp"
#include "TransformFunctions.hpp"
#include "EntityFunctions.hpp"
#include "3DTextFunctions.hpp"
#include "SpriteRendererFunctions.hpp"
#include "SpriteAnimationFunctions.hpp"
#include "SpritesheetAnimationFunctions.hpp"
#include "BloomFunctions.hpp"
#include "Rigidbody2DFunctions.hpp"
#include "Rigidbody3DFunctions.hpp"
#include "BoxCollider2DFunctions.hpp"
#include "SceneFunctions.hpp"
#include "SaveFunctions.hpp"
#include "GameObjectFunctions.hpp"
#include "CameraFunctions.hpp"
#include "Audio3DFunctions.hpp"
#include "MainFunctions.hpp"
#include "DiscordFunctions.hpp"
#include "ImGuiFunctions.hpp"
#include "LightsFunctions.hpp"
#include "MathfFunctions.hpp"

namespace CsharpVariables {
    DLL_API MonoDomain *rootDomain;
    DLL_API MonoDomain *appDomain;
    DLL_API MonoAssembly *coreAssembly;
    DLL_API bool compiledAssembly = false;
    DLL_API std::string oldCwd;
    DLL_API std::vector<PrefabSchedule> schedule_prefab_spawn;
    DLL_API SceneSchedule scene_schedule;
    DLL_API std::string dotnet_path;
} // namespace CsharpVariables

namespace HyperAPI::CsharpScriptEngine::Functions {
    void GetComponent(MonoString *type, void *out) {
        using namespace Experimental;
        std::string str = mono_string_to_utf8(type);
        std::cout << str << std::endl;
        if (str == "Transform") {
            out = &(*Scene::m_GameObjects)[0]->GetComponent<Transform>();
        }
    }

    struct DLL_API m_Vec3 {
        float x;
        float y;
        float z;

        m_Vec3(float x, float y, float z) {
            this->x = x;
            this->y = y;
            this->z = z;
        }
    };

    void RegisterFunctions() {
        mono_add_internal_call("Vault.Terminal::Log", reinterpret_cast<void *(*)>(NativeLog));
        mono_add_internal_call("Vault.Debug::TestVector", reinterpret_cast<void *(*)>(TestVector));
        // ImGui Functions
        mono_add_internal_call("ImGui.Window::Begin", reinterpret_cast<void *(*)>(ImGui_WindowBegin));
        mono_add_internal_call("ImGui.Window::End", reinterpret_cast<void *(*)>(ImGui_WindowEnd));
        mono_add_internal_call("ImGui.Window::cpp_get_win_size", reinterpret_cast<void *(*)>(ImGui_WindowGetSize));
        mono_add_internal_call("ImGui.Window::cpp_get_win_pos", reinterpret_cast<void *(*)>(ImGui_WindowGetPos));
        mono_add_internal_call("ImGui.Window::IsWindowHovered", reinterpret_cast<void *(*)>(ImGui_IsWindowHovered));
        mono_add_internal_call("ImGui.Window::SetWindowPosition", reinterpret_cast<void *(*)>(ImGui_SetWindowPos));
        mono_add_internal_call("ImGui.Window::SetWindowSize", reinterpret_cast<void *(*)>(ImGui_SetWindowSize));
        mono_add_internal_call("ImGui.TreeNode::Begin", reinterpret_cast<void *(*)>(ImGui_TreeNodeBegin));
        mono_add_internal_call("ImGui.TreeNode::End", reinterpret_cast<void *(*)>(ImGui_TreeNodeEnd));
        mono_add_internal_call("ImGui.Window::Child::Begin", reinterpret_cast<void *(*)>(ImGui_WindowChildBegin));
        mono_add_internal_call("ImGui.Window::Child::End", reinterpret_cast<void *(*)>(ImGui_WindowChildEnd));
        mono_add_internal_call("ImGui.GUI::Text", reinterpret_cast<void *(*)>(ImGui_Text));
        mono_add_internal_call("ImGui.GUI::DragFloat", reinterpret_cast<void *(*)>(ImGui_DragFloat));
        mono_add_internal_call("ImGui.GUI::DragInt", reinterpret_cast<void *(*)>(ImGui_DragInt));
        mono_add_internal_call("ImGui.GUI::NewLine", reinterpret_cast<void *(*)>(ImGui_NewLine));
        mono_add_internal_call("ImGui.GUI::Separator", reinterpret_cast<void *(*)>(ImGui_Separator));
        mono_add_internal_call("ImGui.GUI::EnableWindowDocking", reinterpret_cast<void *(*)>(ImGui_EnableWindowDocking));
        mono_add_internal_call("ImGui.GUI::Button", reinterpret_cast<void *(*)>(ImGui_Button));
        mono_add_internal_call("ImGui.GUI::Image", reinterpret_cast<void *(*)>(ImGui_Image));
        mono_add_internal_call("ImGui.GUI::SameLine", reinterpret_cast<void *(*)>(ImGui_SameLine));
        mono_add_internal_call("ImGui.Styles::Push", reinterpret_cast<void *(*)>(ImGui_StylesPush));
        mono_add_internal_call("ImGui.Styles::Pop", reinterpret_cast<void *(*)>(ImGui_StylesPop));
        mono_add_internal_call("ImGui.Colors::Push", reinterpret_cast<void *(*)>(ImGui_ColorsPush));
        mono_add_internal_call("ImGui.Colors::Pop", reinterpret_cast<void *(*)>(ImGui_ColorsPop));

        // Mathf Functions
        mono_add_internal_call("Vault.Mathf::Deg2Rad", reinterpret_cast<void *(*)>(Deg2Rad));
        mono_add_internal_call("Vault.Mathf::Rad2Deg", reinterpret_cast<void *(*)>(Rad2Deg));
        mono_add_internal_call("Vault.Mathf::Abs", reinterpret_cast<void *(*)>(Abs));
        mono_add_internal_call("Vault.Mathf::Acos", reinterpret_cast<void *(*)>(Acos));
        mono_add_internal_call("Vault.Mathf::Asin", reinterpret_cast<void *(*)>(Asin));
        mono_add_internal_call("Vault.Mathf::Atan", reinterpret_cast<void *(*)>(Atan));
        mono_add_internal_call("Vault.Mathf::Atan2", reinterpret_cast<void *(*)>(Atan2));
        mono_add_internal_call("Vault.Mathf::Ceil", reinterpret_cast<void *(*)>(Ceil));
        mono_add_internal_call("Vault.Mathf::Clamp", reinterpret_cast<void *(*)>(Clamp));
        mono_add_internal_call("Vault.Mathf::Cos", reinterpret_cast<void *(*)>(Cos));
        mono_add_internal_call("Vault.Mathf::Sin", reinterpret_cast<void *(*)>(Sin));
        mono_add_internal_call("Vault.Mathf::Sqrt", reinterpret_cast<void *(*)>(Sqrt));
        mono_add_internal_call("Vault.Mathf::Tan", reinterpret_cast<void *(*)>(Tan));
        mono_add_internal_call("Vault.Mathf::Round", reinterpret_cast<void *(*)>(Round));
        mono_add_internal_call("Vault.Mathf::Pow", reinterpret_cast<void *(*)>(Pow));
        mono_add_internal_call("Vault.Mathf::Log", reinterpret_cast<void *(*)>(Log));
        mono_add_internal_call("Vault.Mathf::Log10", reinterpret_cast<void *(*)>(Log10));
        mono_add_internal_call("Vault.Mathf::Max", reinterpret_cast<void *(*)>(Max));
        mono_add_internal_call("Vault.Mathf::Min", reinterpret_cast<void *(*)>(Min));
        mono_add_internal_call("Vault.Mathf::Exp", reinterpret_cast<void *(*)>(Exp));
        mono_add_internal_call("Vault.Mathf::Lerp", reinterpret_cast<void *(*)>(Lerp));
        mono_add_internal_call("Vault.Mathf::RandomRange", reinterpret_cast<void *(*)>(RandomRange));
        mono_add_internal_call("Vault.Mathf::FloatRandomRange", reinterpret_cast<void *(*)>(FloatRandomRange));

        // Main Functions
        mono_add_internal_call("Vault.Main::cpp_DeltaTime", reinterpret_cast<void *(*)>(cpp_DeltaTime));
        mono_add_internal_call("Vault.Main::cpp_Ambient", reinterpret_cast<void *(*)>(cpp_Ambient));
        mono_add_internal_call("Vault.Main::ExitProgram", reinterpret_cast<void *(*)>(ExitProgram));
        mono_add_internal_call("Vault.Main::RandomInt", reinterpret_cast<void *(*)>(cpp_RandomInt));
        mono_add_internal_call("Vault.Format::cpp_tstrf", reinterpret_cast<void *(*)>(cpp_ToStringF));
        mono_add_internal_call("Vault.Format::cpp_tstrd", reinterpret_cast<void *(*)>(cpp_ToStringD));

        // Discord Functions
        mono_add_internal_call("Vault.Discord::Init", reinterpret_cast<void *(*)>(Discord_InitRPC));
        mono_add_internal_call("Vault.Discord::SetPresence", reinterpret_cast<void *(*)>(Discord_SetPresence));

        // Editor Logs
        mono_add_internal_call("Vault.Debug::Log", reinterpret_cast<void *(*)>(EditorLog));
        mono_add_internal_call("Vault.Debug::Error", reinterpret_cast<void *(*)>(EditorError));
        mono_add_internal_call("Vault.Debug::Warning", reinterpret_cast<void *(*)>(EditorWarning));

        // Components
        mono_add_internal_call("Vault.GameObject::CPP_GetComponent", reinterpret_cast<void *(*)>(GetComponent));

        // Audio
        mono_add_internal_call("Vault.Audio::Play", reinterpret_cast<void *(*)>(AudioPlay));
        mono_add_internal_call("Vault.Audio::Music", reinterpret_cast<void *(*)>(AudioMusic));
        mono_add_internal_call("Vault.Audio::PlayingMusic", reinterpret_cast<void *(*)>(AudioPlayingMusic));
        mono_add_internal_call("Vault.Audio::PlayingChannel", reinterpret_cast<void *(*)>(AudioPlayingChannel));
        mono_add_internal_call("Vault.Audio::StopChannel", reinterpret_cast<void *(*)>(AudioStopChannel));

        // Transform Component
        mono_add_internal_call("Vault.Transform::GetKey", reinterpret_cast<void *(*)>(Transform_GetKey));
        mono_add_internal_call("Vault.Transform::SetKey", reinterpret_cast<void *(*)>(Transform_SetKey));
        mono_add_internal_call("Vault.Transform::cpp_LookAt", reinterpret_cast<void *(*)>(Transform_LookAt));

        // ID Shit
        mono_add_internal_call("Vault.Entity::GetID", reinterpret_cast<void *(*)>(Entity_GetID));
        mono_add_internal_call("Vault.Entity::cpp_GetDataFromID", reinterpret_cast<void *(*)>(Entity_GetDataFromID));
        mono_add_internal_call("Vault.Entity::cpp_RemoveGO", reinterpret_cast<void *(*)>(Entity_RemoveGO));
        mono_add_internal_call("Vault.Entity::cpp_AddComponent", reinterpret_cast<void *(*)>(Entity_AddComponent));
        mono_add_internal_call("Vault.Entity::cpp_GetEnabled", reinterpret_cast<void *(*)>(Entity_GetEnabled));
        mono_add_internal_call("Vault.Entity::cpp_SetEnabled", reinterpret_cast<void *(*)>(Entity_SetEnabled));
        mono_add_internal_call("Vault.Entity::cpp_GetClassInstance", reinterpret_cast<void *(*)>(Entity_GetClassInstance));

        // Input Keyboard
        mono_add_internal_call("Vault.Input::IsKeyPressed", reinterpret_cast<void *(*)>(Input_IsKeyPressed));
        mono_add_internal_call("Vault.Input::IsKeyReleased", reinterpret_cast<void *(*)>(Input_IsKeyReleased));
        mono_add_internal_call("Vault.Input::IsMouseButtonPressed", reinterpret_cast<void *(*)>(Input_IsMouseButtonPressed));
        mono_add_internal_call("Vault.Input::IsMouseButtonReleased", reinterpret_cast<void *(*)>(Input_IsMouseButtonReleased));
        mono_add_internal_call("Vault.Input::GetHorizontalAxis", reinterpret_cast<void *(*)>(Input_GetHorizontalAxis));
        mono_add_internal_call("Vault.Input::GetVerticalAxis", reinterpret_cast<void *(*)>(Input_GetVerticalAxis));
        mono_add_internal_call("Vault.Input::GetMouseXAxis", reinterpret_cast<void *(*)>(Input_GetMouseXAxis));
        mono_add_internal_call("Vault.Input::GetMouseYAxis", reinterpret_cast<void *(*)>(Input_GetMouseYAxis));
        mono_add_internal_call("Vault.Input::SetMouseHidden", reinterpret_cast<void *(*)>(Input_SetMouseHidden));
        mono_add_internal_call("Vault.Input::SetMousePosition", reinterpret_cast<void *(*)>(Input_SetMousePosition));
        mono_add_internal_call("Vault.Input::cpp_GetMouseWorldPosition", reinterpret_cast<void *(*)>(Input_GetMouseWorldPosition));
        mono_add_internal_call("Vault.Input::atan2", reinterpret_cast<void *(*)>(Input_atan2));
        mono_add_internal_call("Vault.Input::cpp_normalize", reinterpret_cast<void *(*)>(Input_normalize));

        // GameObject Functions
        mono_add_internal_call("Vault.GameObject::GetIDByName", reinterpret_cast<void *(*)>(GameObject_GetIDByName));
        mono_add_internal_call("Vault.GameObject::GetIDByTag", reinterpret_cast<void *(*)>(GameObject_GetIDByTag));
        mono_add_internal_call("Vault.GameObject::cpp_AddGameObject", reinterpret_cast<void *(*)>(GameObject_AddGameObject));
        mono_add_internal_call("Vault.GameObject::cpp_RemoveGameObject", reinterpret_cast<void *(*)>(GameObject_RemoveGameObject));
        mono_add_internal_call("Vault.GameObject::cpp_InstantiatePrefab", reinterpret_cast<void *(*)>(GameObject_InstantiatePrefab));
        mono_add_internal_call("Vault.GameObject::cpp_InstantiatePrefabWithProperties", reinterpret_cast<void *(*)>(GameObject_InstantiatePrefabWithProperties));

        // Text3D Component
        mono_add_internal_call("Vault.Text3D::GetKey", reinterpret_cast<void *(*)>(Text3D_GetKey));
        mono_add_internal_call("Vault.Text3D::SetText", reinterpret_cast<void *(*)>(Text3D_SetText));
        mono_add_internal_call("Vault.Text3D::SetOthers", reinterpret_cast<void *(*)>(Text3D_SetOthers));

        // SpriteRenderer Component
        mono_add_internal_call("Vault.SpriteRenderer::GetKey", reinterpret_cast<void *(*)>(SpriteRenderer_GetKey));
        mono_add_internal_call("Vault.SpriteRenderer::SetTexture", reinterpret_cast<void *(*)>(SpriteRenderer_SetTexture));
        mono_add_internal_call("Vault.SpriteRenderer::SetColor", reinterpret_cast<void *(*)>(SpriteRenderer_SetColor));

        // SpriteAnimation Component
        mono_add_internal_call("Vault.SpriteAnimation::GetKey", reinterpret_cast<void *(*)>(SpriteAnimation_GetKey));
        mono_add_internal_call("Vault.SpriteAnimation::SetCurrAnimation", reinterpret_cast<void *(*)>(SpriteAnimation_SetCurrAnimation));

        // SpritesheetAnimation Component
        mono_add_internal_call("Vault.SpritesheetAnimation::GetKey", reinterpret_cast<void *(*)>(SpritesheetAnimation_GetKey));
        mono_add_internal_call("Vault.SpritesheetAnimation::SetCurrAnimation", reinterpret_cast<void *(*)>(SpritesheetAnimation_SetCurrAnimation));

        // Bloom Component
        mono_add_internal_call("Vault.Bloom::GetColor", reinterpret_cast<void *(*)>(Bloom_GetColor));
        mono_add_internal_call("Vault.Bloom::SetColor", reinterpret_cast<void *(*)>(Bloom_SetColor));

        // Rigidbody2D Component
        mono_add_internal_call("Vault.Rigidbody2D::cpp_GetKey", reinterpret_cast<void *(*)>(Rigidbody2D_GetKey));
        mono_add_internal_call("Vault.Rigidbody2D::cpp_SetType", reinterpret_cast<void *(*)>(Rigidbody2D_SetType));
        mono_add_internal_call("Vault.Rigidbody2D::cpp_SetVelocity", reinterpret_cast<void *(*)>(Rigidbody2D_SetVelocity));
        mono_add_internal_call("Vault.Rigidbody2D::cpp_SetAngularVelocity", reinterpret_cast<void *(*)>(Rigidbody2D_SetAngularVelocity));
        mono_add_internal_call("Vault.Rigidbody2D::cpp_SetPosition", reinterpret_cast<void *(*)>(Rigidbody2D_SetPosition));
        mono_add_internal_call("Vault.Rigidbody2D::cpp_Force", reinterpret_cast<void *(*)>(Rigidbody2D_Force));
        mono_add_internal_call("Vault.Rigidbody2D::cpp_MoveToPosition", reinterpret_cast<void *(*)>(Rigidbody2D_MoveToPosition));
        mono_add_internal_call("Vault.Rigidbody2D::cpp_Torque", reinterpret_cast<void *(*)>(Rigidbody2D_Torque));

        // Rigidbody3D Component
        mono_add_internal_call("Vault.Rigidbody3D::cpp_GetKey", reinterpret_cast<void *(*)>(Rigidbody3D_GetKey));
        mono_add_internal_call("Vault.Rigidbody3D::cpp_SetPosition", reinterpret_cast<void *(*)>(Rigidbody3D_SetPosition));
        mono_add_internal_call("Vault.Rigidbody3D::cpp_AddForce", reinterpret_cast<void *(*)>(Rigidbody3D_AddForce));
        mono_add_internal_call("Vault.Rigidbody3D::cpp_AddTorque", reinterpret_cast<void *(*)>(Rigidbody3D_AddTorque));
        mono_add_internal_call("Vault.Rigidbody3D::cpp_AddForceAtPosition", reinterpret_cast<void *(*)>(Rigidbody3D_AddForceAtPosition));
        mono_add_internal_call("Vault.Rigidbody3D::cpp_SetVelocity", reinterpret_cast<void *(*)>(Rigidbody3D_SetVelocity));
        mono_add_internal_call("Vault.Rigidbody3D::cpp_SetAngularVelocity", reinterpret_cast<void *(*)>(Rigidbody3D_SetAngularVelocity));

        mono_add_internal_call("Vault.BoxCollider2D::cpp_GetKey", reinterpret_cast<void *(*)>(BoxCollider2D_GetKey));
        mono_add_internal_call("Vault.BoxCollider2D::cpp_SetKey", reinterpret_cast<void *(*)>(BoxCollider2D_SetKey));
        mono_add_internal_call("Vault.BoxCollider2D::cpp_SetOffset", reinterpret_cast<void *(*)>(BoxCollider2D_SetOffset));
        mono_add_internal_call("Vault.BoxCollider2D::cpp_SetSize", reinterpret_cast<void *(*)>(BoxCollider2D_SetSize));
        mono_add_internal_call("Vault.BoxCollider2D::cpp_GetTrigger", reinterpret_cast<void *(*)>(BoxCollider2D_GetTrigger));
        mono_add_internal_call("Vault.BoxCollider2D::cpp_SetTrigger", reinterpret_cast<void *(*)>(BoxCollider2D_SetTrigger));

        // Camera Component
        mono_add_internal_call("Vault.CameraComponent::cpp_GetKey", reinterpret_cast<void *(*)>(Camera_GetKey));
        mono_add_internal_call("Vault.CameraComponent::cpp_SetKey", reinterpret_cast<void *(*)>(Camera_SetKey));
        mono_add_internal_call("Vault.CameraComponent::cpp_Set2D", reinterpret_cast<void *(*)>(Camera_Set2D));

        // Audio3D Component
        mono_add_internal_call("Vault.Audio3D::cpp_GetKey", reinterpret_cast<void *(*)>(Audio3D_GetKey));
        mono_add_internal_call("Vault.Audio3D::cpp_SetFloatKey", reinterpret_cast<void *(*)>(Audio3D_SetFloatKey));
        mono_add_internal_call("Vault.Audio3D::cpp_SetVelocity", reinterpret_cast<void *(*)>(Audio3D_SetVelocity));
        mono_add_internal_call("Vault.Audio3D::cpp_GetLoop", reinterpret_cast<void *(*)>(Audio3D_GetLoop));
        mono_add_internal_call("Vault.Audio3D::cpp_SetLoop", reinterpret_cast<void *(*)>(Audio3D_SetLoop));
        mono_add_internal_call("Vault.Audio3D::cpp_Play", reinterpret_cast<void *(*)>(Audio3D_Play));
        mono_add_internal_call("Vault.Audio3D::cpp_Stop", reinterpret_cast<void *(*)>(Audio3D_Stop));
        mono_add_internal_call("Vault.Audio3D::cpp_SetClip", reinterpret_cast<void *(*)>(Audio3D_SetClip));
        mono_add_internal_call("Vault.Audio3D::cpp_GetClip", reinterpret_cast<void *(*)>(Audio3D_GetClip));

        // Lights
        mono_add_internal_call("Vault.PointLight::GetKey", reinterpret_cast<void *(*)>(PointLight_GetKey));
        mono_add_internal_call("Vault.PointLight::SetKey", reinterpret_cast<void *(*)>(PointLight_SetKey));

        // Scene Functions
        mono_add_internal_call("Vault.Scene::LoadScene", reinterpret_cast<void *(*)>(Scene_LoadScene));

        // Save Functions
        mono_add_internal_call("Vault.SaveFile::cpp_GetVariable", reinterpret_cast<void *(*)>(Save_GetVariable));
        mono_add_internal_call("Vault.SaveFile::cpp_SaveVariable", reinterpret_cast<void *(*)>(Save_SaveVariable));
        mono_add_internal_call("Vault.SaveFile::cpp_DeleteVariable", reinterpret_cast<void *(*)>(Save_DeleteVariable));
    }
} // namespace HyperAPI::CsharpScriptEngine::Functions

namespace HyperAPI::CsharpScriptEngine {
    DLL_API std::string nextId;

    DLL_API std::unordered_map<std::string, MonoScriptClass *> entityClasses;
    DLL_API std::unordered_map<std::string, MonoScriptClass *> instances;

    char *ReadBytes(const std::string &filepath, uint32_t *outSize) {
        std::ifstream stream(filepath, std::ios::binary | std::ios::ate);

        if (!stream) {
            return nullptr;
        }

        std::streampos end = stream.tellg();
        stream.seekg(0, std::ios::beg);
        uint32_t size = end - stream.tellg();

        if (size == 0) {
            return nullptr;
        }

        char *buffer = new char[size];
        stream.read((char *)buffer, size);
        stream.close();

        *outSize = size;
        return buffer;
    }

    MonoAssembly *LoadCSharpAssembly(const std::string &assemblyPath) {
        uint32_t fileSize = 0;
        char *fileData = ReadBytes(assemblyPath, &fileSize);

        // NOTE: We can't use this image for anything other than loading the assembly because this image doesn't have a reference to the assembly
        MonoImageOpenStatus status;
        MonoImage *image = mono_image_open_from_data_full(fileData, fileSize, 1, &status, 0);

        if (status != MONO_IMAGE_OK) {
            const char *errorMessage = mono_image_strerror(status);
            // Log some error message using the errorMessage data
            return nullptr;
        }

        MonoAssembly *assembly = mono_assembly_load_from_full(image, assemblyPath.c_str(), &status, 0);
        mono_image_close(image);

        // Don't forget to free the file data
        delete[] fileData;

        return assembly;
    }

    void PrintAssemblyTypes(MonoAssembly *assembly) {
        MonoImage *image = mono_assembly_get_image(assembly);
        const MonoTableInfo *typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);

        for (int32_t i = 0; i < numTypes; i++) {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char *nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char *name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

            printf("%s.%s\n", nameSpace, name);
        }
    }

    void LoadAssemblyClasses(MonoAssembly *assembly) {
        for (auto klass : entityClasses) {
            delete klass.second;
        }
        entityClasses.clear();

        MonoImage *image = mono_assembly_get_image(assembly);
        const MonoTableInfo *typeDefinitionsTable = mono_image_get_table_info(image, MONO_TABLE_TYPEDEF);
        int32_t numTypes = mono_table_info_get_rows(typeDefinitionsTable);
        MonoClass *monoClass = mono_class_from_name(image, "Vault", "Entity");

        for (int32_t i = 0; i < numTypes; i++) {
            uint32_t cols[MONO_TYPEDEF_SIZE];
            mono_metadata_decode_row(typeDefinitionsTable, i, cols, MONO_TYPEDEF_SIZE);

            const char *nameSpace = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAMESPACE]);
            const char *name = mono_metadata_string_heap(image, cols[MONO_TYPEDEF_NAME]);

            MonoClass *entityClass = mono_class_from_name(image, nameSpace, name);

            bool isSubclass = mono_class_is_subclass_of(entityClass, monoClass, false);

            if (entityClass == monoClass)
                continue;

            if (isSubclass) {
                entityClasses[std::string(std::string(nameSpace) + "." + name)] = new MonoScriptClass(nameSpace, name);
            }
        }
    }

    void RuntimeInit() {
        using namespace CsharpVariables;
        if (rootDomain == nullptr) {
            // const char *argv[2] = {
            //     "--debugger-agent=transport=dt_socket,server=y,address=127.0.0.1:55555,suspend=n,loglevel=3,logfile=logs/MonoDebugger.log",
            //     "--soft-breakpoints"};

            // mono_jit_parse_options(2, (char **)argv);
            // mono_debug_init(MONO_DEBUG_FORMAT_MONO);
            rootDomain = mono_jit_init("VaultJITRuntime");

            mono_debug_domain_create(rootDomain);
            mono_thread_set_main(mono_thread_current());

            if (rootDomain == nullptr)
                exit(1);
        }
    }

    void InitMono() {
        using namespace Functions;
        using namespace CsharpVariables;

        if (fs::exists("assets/VAULT_OUT/cs-assembly.dll")) {

#ifdef _WIN32
            mono_set_assemblies_path((std::string(CsharpVariables::oldCwd) + "\\mono\\lib").c_str());
#else
            mono_set_assemblies_path((std::string(CsharpVariables::oldCwd) + "/mono/lib").c_str());
            setenv("MONO_PATH", (std::string(CsharpVariables::oldCwd) + "/mono/lib").c_str(), 0);
#endif
            // const char *argv[2] = {
            // "--debugger-agent=transport=dt_socket,address=127.0.0.1:2550,server=y,suspend=n,loglevel=3,logfile=logs/VaultMonoDebugger.log",
            // "--soft-breakpoints"};

            // mono_jit_parse_options(2, (char **)argv);
            RuntimeInit();
            // mono_debug_init(MONO_DEBUG_FORMAT_MONO);
            // mono_debug_domain_create(rootDomain);

            appDomain = mono_domain_create_appdomain("VaultScriptRuntime", nullptr);
            mono_domain_set(appDomain, true);

            RegisterFunctions();

            coreAssembly = LoadCSharpAssembly("assets/VAULT_OUT/cs-assembly.dll");
            PrintAssemblyTypes(coreAssembly);
            LoadAssemblyClasses(coreAssembly);

            MonoScriptClass mainClass("Vault", "Terminal");
            mainClass.CallConstructor();

            MonoScriptClass debugClass("Vault", "Debug");
            debugClass.CallConstructor();

            MonoScriptClass audioClass("Vault", "Audio");
            audioClass.CallConstructor();

            MonoScriptClass discordClass("Vault", "Discord");
            audioClass.CallConstructor();

            // MonoScriptClass  sceneClass("Vault", "Scene");
            // sceneClass.CallConstructor();
        }
    }

    void ReloadAssembly() {
        using namespace Functions;
        using namespace CsharpVariables;

        if (!fs::exists("assets/VAULT_OUT/cs-assembly.dll"))
            return;

        mono_domain_set(mono_get_root_domain(), false);
        mono_domain_unload(appDomain);

        InitMono();
    }

    void CompileAssemblies() {
        Scene::logs.clear();
        std::thread *compilerThread = new std::thread([&] {
            std::array<char, 1000> buffer;
            std::string result;
            std::string build_command = "cd assets";
            build_command += " && \"";
            build_command += CsharpVariables::dotnet_path;
            build_command += "\" build --property WarningLevel=0 -o VAULT_OUT";
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(build_command.c_str(), "r"), pclose);

            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }

            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                using namespace HyperAPI;
                const std::string output = buffer.data();

                if (output.find("error") != std::string::npos) {
                    Log errorLog(output, LOG_ERROR);
                } else if (output.find("warning") != std::string::npos) {
                    Log warningLog(output, LOG_WARNING);
                } else if (output.find("no warning") != std::string::npos) {
                    Log log(output, LOG_INFO);
                } else {
                    Log log(output, LOG_INFO);
                }

                if (output.find("Build succeeded.") != std::string::npos) {
                    CsharpVariables::compiledAssembly = true;
                }
            }

            using namespace CsharpVariables;
        });
    }

    void CreateCsharpProject() {
        Scene::logs.clear();
        HYPER_LOG(CsharpVariables::oldCwd)
        if (fs::exists("cs-assembly"))
            fs::remove_all("cs-assmebly");

#ifdef _WIN32
        if (!fs::exists("cs-assembly"))
            fs::remove_all("cs-assmebly");

        fs::copy(CsharpVariables::oldCwd + "\\cs-assembly", "cs-assembly", fs::copy_options::recursive);
#else

        fs::copy(CsharpVariables::oldCwd + "/cs-assembly", "cs-assembly", fs::copy_options::recursive);
#endif

        std::thread *compilerThread = new std::thread([&] {
            std::array<char, 1000> buffer;
            std::string result;
            std::unique_ptr<FILE, decltype(&pclose)> pipe(popen("cd cs-assembly && dotnet new classlib", "r"), pclose);

            if (!pipe) {
                throw std::runtime_error("popen() failed!");
            }

            while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
                using namespace HyperAPI;
                const std::string output = buffer.data();

                if (output.find("error") != std::string::npos) {
                    Log errorLog(output, LOG_ERROR);
                } else if (output.find("warning") != std::string::npos) {
                    Log warningLog(output, LOG_WARNING);
                } else if (output.find("no warning") != std::string::npos) {
                    Log log(output, LOG_INFO);
                } else {
                    Log log(output, LOG_INFO);
                }

                if (output.find("Build succeeded.") != std::string::npos) {
                    CsharpVariables::compiledAssembly = true;
                }
            }

            using namespace CsharpVariables;
        });
    }
} // namespace HyperAPI::CsharpScriptEngine

MonoScriptClass::MonoScriptClass(const std::string &nameSpace, const std::string &name) : name_space(nameSpace), name(name) {
    using namespace CsharpVariables;

    MonoImage *image = mono_assembly_get_image(coreAssembly);
    klass = mono_class_from_name(image, nameSpace.c_str(), name.c_str());
    instance = mono_object_new(appDomain, klass);
    gc_handle = mono_gchandle_new(instance, false);
}

MonoObject *MonoScriptClass::f_GetObject() const {
    return instance;
}

MonoObject *MonoScriptClass::f_GetObjectGC() const {
    return mono_gchandle_get_target(gc_handle);
}

MonoMethod *MonoScriptClass::GetMethod(const std::string method, int parameterCount) const {

    return mono_class_get_method_from_name(klass, method.c_str(), parameterCount);
};

MonoClass *MonoScriptClass::GetClass() const {
    return klass;
};

void MonoScriptClass::CallConstructor() {
    mono_runtime_object_init(f_GetObjectGC());
}