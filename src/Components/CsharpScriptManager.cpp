#pragma once
#include "CsharpScriptManager.hpp"
#include "GameObject.hpp"
#include "libs.hpp"
#include "scene.hpp"
#include "../Renderer/Log.hpp"
#include "../f_GameObject/f_GameObject.hpp"

namespace HyperAPI::Experimental {
    void CsharpScriptManager::Update() {
        GameObject *g = f_GameObject::FindGameObjectByID(ID);
        if (Scene::stop_scripts)
            return;
        for (auto klass : selectedScripts) {
            if (klass.second == "")
                continue;

            MonoObject *exception = nullptr;
            CsharpScript *behaviour = nullptr;
            if (behaviours.find(klass.first) != behaviours.end()) {
                behaviour = &behaviours[klass.first];
            } else {
                Start();
                behaviour = &behaviours[klass.first];
            }

            if (!behaviour)
                continue;

            typedef void (*OnUpdateType)(MonoObject *, MonoObject **);
            behaviour->onUpdateThunk(behaviour->behaviour->f_GetObjectGC(), &exception);

            // void *params[0] = {};

            // mono_runtime_invoke(onUpdate, behaviour->f_GetObjectGC(), params, &exception);
            if (exception) {
                MonoObject *exc = NULL;
                MonoString *str = mono_object_to_string(exception, &exc);
                if (exc) {
                    mono_print_unhandled_exception(exc);
                } else {
                    Log log(mono_string_to_utf8(str), LOG_ERROR);
                }
            }
        }
    }

    void CsharpScriptManager::DeleteComp() {
        selectedScripts.clear();
        HYPER_LOG("CLEARED");
    }

    void CsharpScriptManager::Start() {
        behaviours.clear();
        for (auto klass : selectedScripts) {

            if (klass.second == "")
                continue;
            MonoObject *exception = nullptr;
            std::istringstream iss(klass.first);
            std::vector<std::string> tokens;
            std::string token;
            while (std::getline(iss, token, '.')) {
                if (!token.empty())
                    tokens.push_back(token);
            }

            CsharpScript *behaviour = nullptr;
            bool is_new = false;
            if (behaviours.find(klass.second) != behaviours.end()) {
                behaviour = &behaviours[klass.second];
            } else {
                CsharpScript script;
                script.behaviour = new MonoScriptClass(tokens[0], tokens[1]);
                script.updateMethod = script.behaviour->GetMethod("OnUpdate", 0);
                script.startMethod = script.behaviour->GetMethod("OnStart", 0);
                script.onUpdateThunk = (OnUpdateType)mono_method_get_unmanaged_thunk(script.updateMethod);
                script.onStartThunk = (OnStartType)mono_method_get_unmanaged_thunk(script.startMethod);
                behaviours[klass.second] = script;
                behaviour = &behaviours[klass.second];
                is_new = true;
            }

            behaviour->behaviour->CallConstructor();
            CsharpScriptEngine::nextId = ID;
            void *params[0] = {};
            behaviour->onStartThunk(behaviour->behaviour->f_GetObjectGC(), &exception);

            // void *iterator = nullptr;
            // while(MonoClassField *field =
            // mono_class_get_fields(behaviour->GetClass(), &iterator))
            // {
            //     const char *name = mono_field_get_name(field);
            //     const char *type =
            //     mono_type_get_name(mono_field_get_type(field));

            //     std::cout << "Name: " << name << " Type: " << type <<
            //     std::endl;
            // }

            // mono_runtime_invoke(onStart, behaviour->f_GetObjectGC(), params, &exception);
            if (exception) {
                MonoObject *exc = NULL;
                MonoString *str = mono_object_to_string(exception, &exc);
                if (exc) {
                    mono_print_unhandled_exception(exc);
                } else {
                    Log log(mono_string_to_utf8(str), LOG_ERROR);
                }
            }
            CsharpScriptEngine::instances[klass.first] = behaviour->behaviour;
        }
    }

    void CsharpScriptManager::GUI() {
        if (ImGui::CollapsingHeader("C# Script Manager")) {
            ImGui::ListBoxHeader("C# Scripts");

            for (auto klass : CsharpScriptEngine::entityClasses) {
                if (ImGui::Selectable(selectedScripts[klass.first] ==
                                              klass.first
                                          ? (std::string(ICON_FA_CHECK) +
                                             " " + klass.first)
                                                .c_str()
                                          : klass.first.c_str(),
                                      false)) {
                    if (selectedScripts[klass.first] == klass.first) {
                        if (behaviours.find(klass.first) != behaviours.end()) {
                            behaviours.erase(behaviours.find(klass.first));
                        }

                        selectedScripts.erase(klass.first);
                    } else {
                        std::istringstream iss(klass.first);
                        std::vector<std::string> tokens;
                        std::string token;
                        while (std::getline(iss, token, '.')) {
                            if (!token.empty())
                                tokens.push_back(token);
                        }

                        CsharpScript script;
                        script.behaviour = new MonoScriptClass(tokens[0], tokens[1]);
                        script.updateMethod = script.behaviour->GetMethod("OnUpdate", 0);
                        script.startMethod = script.behaviour->GetMethod("OnStart", 0);
                        script.onUpdateThunk = (OnStartType)mono_method_get_unmanaged_thunk(script.updateMethod);
                        script.onStartThunk = (OnStartType)mono_method_get_unmanaged_thunk(script.startMethod);
                        behaviours[klass.first] = script;
                        selectedScripts[klass.first] = klass.first;
                    }
                }
            }
            ImGui::ListBoxFooter();
            ImGui::NewLine();

            // for (auto klass : behaviours) {
            //     if (!behaviours[klass.first])
            //         continue;

            //     if (ImGui::TreeNode((klass.first).c_str())) {
            //         int num = mono_class_num_fields(klass.second->GetClass());
            //         std::cout << klass.first << " Has " << num << " Fiels\n";
            //         void *iterator;

            //         ImGui::TreePop();
            //     }
            // }

            ImGui::NewLine();
            if (ImGui::Button(ICON_FA_TRASH " Remove Component")) {
                Scene::m_Registry.remove<CsharpScriptManager>(entity);
            }
        }
    }

} // namespace HyperAPI::Experimental