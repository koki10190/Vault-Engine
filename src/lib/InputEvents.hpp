#pragma once
#include <dllapi.hpp>
#include "../vendor/glad/include/glad/glad.h"
#include "../vendor/GLFW/glfw3.h"
#include "../vendor/glm/glm.hpp"
#include "../vendor/glm/gtc/matrix_transform.hpp"
#include "../vendor/glm/gtc/type_ptr.hpp"
#include "../vendor/glm/ext.hpp"
#include "../vendor/glm/gtx/rotate_vector.hpp"
#include "../vendor/glm/gtx/vector_angle.hpp"

/* The unknown key */
#define KEY_UNKNOWN -1

/* Printable keys */
#define KEY_SPACE 32
#define KEY_APOSTROPHE 39 /* ' */
#define KEY_COMMA 44      /* , */
#define KEY_MINUS 45      /* - */
#define KEY_PERIOD 46     /* . */
#define KEY_SLASH 47      /* / */
#define KEY_0 48
#define KEY_1 49
#define KEY_2 50
#define KEY_3 51
#define KEY_4 52
#define KEY_5 53
#define KEY_6 54
#define KEY_7 55
#define KEY_8 56
#define KEY_9 57
#define KEY_SEMICOLON 59 /* ; */
#define KEY_EQUAL 61     /* = */
#define KEY_A 65
#define KEY_B 66
#define KEY_C 67
#define KEY_D 68
#define KEY_E 69
#define KEY_F 70
#define KEY_G 71
#define KEY_H 72
#define KEY_I 73
#define KEY_J 74
#define KEY_K 75
#define KEY_L 76
#define KEY_M 77
#define KEY_N 78
#define KEY_O 79
#define KEY_P 80
#define KEY_Q 81
#define KEY_R 82
#define KEY_S 83
#define KEY_T 84
#define KEY_U 85
#define KEY_V 86
#define KEY_W 87
#define KEY_X 88
#define KEY_Y 89
#define KEY_Z 90
#define KEY_LEFT_BRACKET 91  /* [ */
#define KEY_BACKSLASH 92     /* \ */
#define KEY_RIGHT_BRACKET 93 /* ] */
#define KEY_GRAVE_ACCENT 96  /* ` */
#define KEY_WORLD_1 161      /* non-US #1 */
#define KEY_WORLD_2 162      /* non-US #2 */

/* Function keys */
#define KEY_ESCAPE 256
#define KEY_ENTER 257
#define KEY_TAB 258
#define KEY_BACKSPACE 259
#define KEY_INSERT 260
#define KEY_DELETE 261
#define KEY_RIGHT 262
#define KEY_LEFT 263
#define KEY_DOWN 264
#define KEY_UP 265
#define KEY_PAGE_UP 266
#define KEY_PAGE_DOWN 267
#define KEY_HOME 268
#define KEY_END 269
#define KEY_CAPS_LOCK 280
#define KEY_SCROLL_LOCK 281
#define KEY_NUM_LOCK 282
#define KEY_PRINT_SCREEN 283
#define KEY_PAUSE 284
#define KEY_F1 290
#define KEY_F2 291
#define KEY_F3 292
#define KEY_F4 293
#define KEY_F5 294
#define KEY_F6 295
#define KEY_F7 296
#define KEY_F8 297
#define KEY_F9 298
#define KEY_F10 299
#define KEY_F11 300
#define KEY_F12 301
#define KEY_F13 302
#define KEY_F14 303
#define KEY_F15 304
#define KEY_F16 305
#define KEY_F17 306
#define KEY_F18 307
#define KEY_F19 308
#define KEY_F20 309
#define KEY_F21 310
#define KEY_F22 311
#define KEY_F23 312
#define KEY_F24 313
#define KEY_F25 314
#define KEY_KP_0 320
#define KEY_KP_1 321
#define KEY_KP_2 322
#define KEY_KP_3 323
#define KEY_KP_4 324
#define KEY_KP_5 325
#define KEY_KP_6 326
#define KEY_KP_7 327
#define KEY_KP_8 328
#define KEY_KP_9 329
#define KEY_KP_DECIMAL 330
#define KEY_KP_DIVIDE 331
#define KEY_KP_MULTIPLY 332
#define KEY_KP_SUBTRACT 333
#define KEY_KP_ADD 334
#define KEY_KP_ENTER 335
#define KEY_KP_EQUAL 336
#define KEY_LEFT_SHIFT 340
#define KEY_LEFT_CONTROL 341
#define KEY_LEFT_ALT 342
#define KEY_LEFT_SUPER 343
#define KEY_RIGHT_SHIFT 344
#define KEY_RIGHT_CONTROL 345
#define KEY_RIGHT_ALT 346
#define KEY_RIGHT_SUPER 347
#define KEY_MENU 348

#define KEY_LAST KEY_MENU

// controller keys
#define KEY_CONTROLLER_A GLFW_GAMEPAD_BUTTON_A
#define KEY_CONTROLLER_B GLFW_GAMEPAD_BUTTON_B
#define KEY_CONTROLLER_Y GLFW_GAMEPAD_BUTTON_Y
#define KEY_CONTROLLER_X GLFW_GAMEPAD_BUTTON_X
#define KEY_CONTROLLER_L1 GLFW_GAMEPAD_BUTTON_LEFT_BUMPER
#define KEY_CONTROLLER_R1 GLFW_GAMEPAD_BUTTON_RIGHT_BUMPER
// start, select
#define KEY_CONTROLLER_START GLFW_GAMEPAD_BUTTON_START
#define KEY_CONTROLLER_SELECT GLFW_GAMEPAD_BUTTON_BACK
// L3, R3
#define KEY_CONTROLLER_L3 GLFW_GAMEPAD_BUTTON_LEFT_THUMB
#define KEY_CONTROLLER_R3 GLFW_GAMEPAD_BUTTON_RIGHT_THUMB
// up, down, left, right
#define KEY_CONTROLLER_UP GLFW_GAMEPAD_BUTTON_DPAD_UP
#define KEY_CONTROLLER_DOWN GLFW_GAMEPAD_BUTTON_DPAD_DOWN
#define KEY_CONTROLLER_LEFT GLFW_GAMEPAD_BUTTON_DPAD_LEFT
#define KEY_CONTROLLER_RIGHT GLFW_GAMEPAD_BUTTON_DPAD_RIGHT

namespace HyperAPI {
    namespace Input {
        DLL_API extern GLFWwindow *window;
        DLL_API extern glm::vec3 winPos;
        DLL_API extern glm::vec3 winSize;
        DLL_API extern glm::vec3 mouseRay;

        void set_ray(glm::vec2 &pos, glm::vec3 &scale, glm::vec2 &renderer);
        bool IsKeyPressed(int key);
        bool IsKeyReleased(int key);
        bool IsMouseButtonPressed(int button);
        bool IsMouseButtonReleased(int button);
        float GetMouseX();
        float GetMouseY();
        int GetHorizontalAxis();
        int GetVerticalAxis();
        int GetMouseXAxis();
        int GetMouseYAxis();
        bool SetMouseHidden(bool hidden);
        int SetMousePosition(float x, float y);

        // controller input
        namespace Controller {
            DLL_API extern int currentController;

            void SetCurrentController(int controller);

            float GetLeftAnalogX();
            float GetLeftAnalogY();
            float GetRightAnalogX();
            float GetRightAnalogY();
            float GetL2();
            float GetR2();
            bool IsButtonPressed(int button);
            bool IsButtonReleased(int button);
        } // namespace Controller
    }     // namespace Input
} // namespace HyperAPI