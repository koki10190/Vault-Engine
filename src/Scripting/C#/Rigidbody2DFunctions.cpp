#include "Rigidbody2DFunctions.hpp"
#include "box2d/b2_body.h"
#include "mono/metadata/object.h"

namespace HyperAPI::CsharpScriptEngine::Functions {
    void Rigidbody2D_GetKey(MonoString *key, MonoString *id, MonoString **result) {
        using namespace Experimental;
        using namespace CsharpVariables;
        using namespace std;

        const std::string m_key = mono_string_to_utf8(key);
        const std::string m_id = mono_string_to_utf8(id);

        auto *gameObject = f_GameObject::FindGameObjectByID(m_id);
        if (!gameObject) {
            Log log(("C#: Couldn't find game object with ID: " + m_id), LOG_ERROR);
            return;
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody2D>();

        if (m_key == "velocity") {
            *result = mono_string_new(appDomain, (to_string(((b2Body *)rigidbody.body)->GetLinearVelocity().x) + " " + to_string(((b2Body *)rigidbody.body)->GetLinearVelocity().y)).c_str());
        }

        if (m_key == "type") {
            *result = mono_string_new(appDomain, (to_string(rigidbody.type).c_str()));
        }
    }
    void Rigidbody2D_SetVelocity(float x, float y, MonoString *id) {
        using namespace Experimental;
        using namespace CsharpVariables;

        const std::string m_id = mono_string_to_utf8(id);
        auto *gameObject = f_GameObject::FindGameObjectByID(m_id);
        if (!gameObject) {
            Log log(("C#: Couldn't find game object with ID: " + m_id), LOG_ERROR);
            return;
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody2D>();

        rigidbody.SetVelocity(x, y);
    }
    void Rigidbody2D_SetAngularVelocity(float velocity, MonoString *id) {
        using namespace Experimental;
        using namespace CsharpVariables;

        const std::string m_id = mono_string_to_utf8(id);
        auto *gameObject = f_GameObject::FindGameObjectByID(m_id);
        if (!gameObject) {
            Log log(("C#: Couldn't find game object with ID: " + m_id), LOG_ERROR);
            return;
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody2D>();

        rigidbody.SetAngularVelocity(velocity);
    }
    void Rigidbody2D_SetPosition(float x, float y, MonoString *id) {
        using namespace Experimental;
        using namespace CsharpVariables;

        const std::string m_id = mono_string_to_utf8(id);
        auto *gameObject = f_GameObject::FindGameObjectByID(m_id);
        if (!gameObject) {
            Log log(("C#: Couldn't find game object with ID: " + m_id), LOG_ERROR);
            return;
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody2D>();

        rigidbody.SetPosition(x, y);
    }
    void Rigidbody2D_Force(float x, float y, MonoString *id) {
        using namespace Experimental;
        using namespace CsharpVariables;

        const std::string m_id = mono_string_to_utf8(id);
        auto *gameObject = f_GameObject::FindGameObjectByID(m_id);
        if (!gameObject) {
            Log log(("C#: Couldn't find game object with ID: " + m_id), LOG_ERROR);
            return;
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody2D>();

        rigidbody.Force(x, y);
    }
    void Rigidbody2D_Torque(float torque, MonoString *id) {
        using namespace Experimental;
        using namespace CsharpVariables;

        const std::string m_id = mono_string_to_utf8(id);
        auto *gameObject = f_GameObject::FindGameObjectByID(m_id);
        if (!gameObject) {
            Log log(("C#: Couldn't find game object with ID: " + m_id), LOG_ERROR);
            return;
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody2D>();

        rigidbody.Torque(torque);
    }

    void Rigidbody2D_SetType(int type, MonoString *id) {
        using namespace Experimental;
        using namespace CsharpVariables;

        const std::string m_id = mono_string_to_utf8(id);
        auto *gameObject = f_GameObject::FindGameObjectByID(m_id);
        if (!gameObject) {
            Log log(("C#: Couldn't find game object with ID: " + m_id), LOG_ERROR);
            return;
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody2D>();
        rigidbody.type = (b2BodyType)type;
    }

    void Rigidbody2D_MoveToPosition(float tx, float ty, float velocity, MonoString *id) {
        using namespace Experimental;
        using namespace CsharpVariables;

        const std::string m_id = mono_string_to_utf8(id);
        auto *gameObject = f_GameObject::FindGameObjectByID(m_id);
        if (!gameObject) {
            Log log(("C#: Couldn't find game object with ID: " + m_id), LOG_ERROR);
            return;
        }

        auto &rigidbody = gameObject->GetComponent<Rigidbody2D>();
        rigidbody.MoveToPosition(glm::vec2(tx, ty), velocity);
    }
} // namespace HyperAPI::CsharpScriptEngine::Functions