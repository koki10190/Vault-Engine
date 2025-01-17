#pragma once
#include <dllapi.hpp>
#include <libs.hpp>
#include "Exp_Base.hpp"
#include "../Renderer/Structures.hpp"
#include "../Renderer/OldStuff.hpp"

#include "Transform.hpp"
#include "GameObject.hpp"
#include "../Renderer/BulletPhysicsWorld.hpp"
#include "../Renderer/DecomposeTransform.hpp"

namespace HyperAPI::Experimental {
    struct DLL_API Rigidbody3D : public BaseComponent {
        float mass = 1;
        float restitution = 0.0f;
        float friction = 0.5f;
        bool trigger = false;
        bool fixedRotation = false;

        GameObject *gameObject;
        Transform *transform;

        btRigidBody *body = nullptr;
        btDefaultMotionState *motionState = nullptr;
        btTransform *bt_transform = nullptr;
        btCollisionShape *ref;

        btVector3 inertia = btVector3(0, 0, 0);

        Rigidbody3D() = default;

        void Init() override {
            for (auto &m_Object : (*Scene::m_GameObjects)) {
                if (m_Object->ID == ID) {
                    gameObject = m_Object;
                    transform = &gameObject->GetComponent<Transform>();

                    break;
                }
            }
        }

        void MoveForward(float speed, const Vector3 &rotation) {
            if (body) {
                AddForce(speed * rotation);
            }
        }

        void CreateBody(btCollisionShape *shape);

        void DeleteBody() const {
            BulletPhysicsWorld::dynamicsWorld->removeRigidBody(body);
            delete body;
            delete motionState;
            delete bt_transform;
        }

        void Update();

        void GUI() override;

        // Rigidbody functions
        void
        AddForce(const Vector3 &force) {
            body->applyCentralForce(btVector3(force.x, force.y, force.z));
        }

        void AddTorque(const Vector3 &torque) {
            body->applyTorque(btVector3(torque.x, torque.y, torque.z));
        }

        void AddForceAtPosition(const Vector3 &force, const Vector3 &position) {
            body->applyForce(btVector3(force.x, force.y, force.z),
                             btVector3(position.x, position.y, position.z));
        }

        // velocity functions
        void SetVelocity(const Vector3 &velocity) {
            body->setLinearVelocity(
                btVector3(velocity.x, velocity.y, velocity.z));
        }

        void SetAngularVelocity(const Vector3 &velocity) {
            body->setAngularVelocity(
                btVector3(velocity.x, velocity.y, velocity.z));
        }

        Vector3 GetVelocity() {
            auto velocity = body->getLinearVelocity();
            return Vector3(velocity.x(), velocity.y(), velocity.z());
        }

        Vector3 GetAngularVelocity() {
            auto velocity = body->getAngularVelocity();
            return Vector3(velocity.x(), velocity.y(), velocity.z());
        }

        // make rigidbody move like a fps controller
        void Move(const Vector3 &direction, float speed) {
            auto velocity = body->getLinearVelocity();
            body->setLinearVelocity(btVector3(direction.x * speed, velocity.y(),
                                              direction.z * speed));
        }
    };

    struct DLL_API FixedJoint3D : public BaseComponent {
        GameObject *gameObject = nullptr;
        GameObject *connectedGameObject = nullptr;
        Transform *transform;

        btRigidBody *body = nullptr;
        btRigidBody *body2 = nullptr;
        btTypedConstraint *joint = nullptr;

        FixedJoint3D() = default;

        void Init() override {
            for (auto &m_Object : (*Scene::m_GameObjects)) {
                if (m_Object->ID == ID) {
                    gameObject = m_Object;
                    transform = &gameObject->GetComponent<Transform>();

                    break;
                }
            }
        }

        void CreateJoint() {
            if (!connectedGameObject)
                return;

            body = gameObject->GetComponent<Rigidbody3D>().body;
            body2 = connectedGameObject->GetComponent<Rigidbody3D>().body;

            btTransform frameInA = btTransform::getIdentity();
            btTransform frameInB = btTransform::getIdentity();

            joint = new btFixedConstraint(*body, *body2, frameInA, frameInB);
            BulletPhysicsWorld::dynamicsWorld->addConstraint(joint);
        }

        void DeleteJoint() const {
            if (joint) {
                BulletPhysicsWorld::dynamicsWorld->removeConstraint(joint);
                delete joint;
            }
        }

        void GUI() override {
            if (ImGui::TreeNode("Fixed Joint 3D")) {
                if (!connectedGameObject) {
                    ImGui::Selectable("Drag and drop another object with "
                                      "Rigidbody3D to connect");
                } else {
                    ImGui::Selectable(std::string("Connected with: " +
                                                  connectedGameObject->name)
                                          .c_str());
                }

                if (ImGui::BeginDragDropTarget()) {
                    if (const ImGuiPayload *payload =
                            ImGui::AcceptDragDropPayload("game_object")) {
                        for (auto &m_Object : (*Scene::m_GameObjects)) {
                            if (m_Object->ID ==
                                    *((std::string *)payload->Data) &&
                                m_Object->HasComponent<Rigidbody3D>()) {
                                connectedGameObject = m_Object;
                                break;
                            }
                        }
                    }
                }

                ImGui::NewLine();
                if (ImGui::Button(ICON_FA_TRASH " Remove Component")) {
                    Scene::m_Registry.remove<FixedJoint3D>(entity);
                }
                ImGui::TreePop();
            }
        }
    };
} // namespace HyperAPI::Experimental