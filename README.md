<div style="
display: flex;
justify-content: center;
align-items: center;
flex-direction: column;
">
<img style="" src="./build/logo2.png">

<p style="width: 100%; text-align: center; margin-top: 40px; font-size: 50px; color: #ff3b3b; margin-bottom: -10px">Static Engine</p>

## An Open Source 2D and 3D Game Engine

### Built with C++

</div>

---

## Supported Platforms:

-   **Linux**
-   **Windows**

## Features

-   **2D and 3D Rendering**
-   **Script Engine: Lua**
-   **Physics Engine: Box2D, Bullet Physics (3D)**
-   **Audio (3D): OpenAL, (2D) SDL_mixer**
-   **Level Editor**
-   **Native Scripting**
-   **2D Lighting**
-   **3D Lighting**
-   **More coming soon**

## Used Dependencies

-   **Assimp**
-   **Box2D**
-   **EnTT**
-   **GLAD**
-   **GLFW**
-   **glm**
-   **IconFontCppHeaders**
-   **ImGui**
-   **ImGuiFileDialog**
-   **nlohmann json**
-   **SDL/SDL_mixer**
-   **stb_image**
-   **tinyxml**
-   **Bullet Physics**
-   **OpenAL**
-   **luajit**

# Installation

### **Linux**

```bash
git clone https://github.com/koki10190/Static-Engine.git
cd Static-Engine
make linux
./Static\ Engine.sh
```

### **Windows**

First! Go to Makefile and get to line 64 and change x86_64-w64-mingw32-g++ to g++ (MUST HAVE MINGW INSTALLED FOR THIS AND MINGW MAKE, you do not have to change this if you are on linux and want to compile it for windows, you just need to install x86_64-w64-mingw32)

```bash
git clone https://github.com/koki10190/Static-Engine.git
cd Static-Engine
make win
./win_build.exe
```

### **Bundling**

Bundling the application is very easy, this only works on linux since i do not have .bat file for this created

```bash
./bundle.sh # this will bundle for linux, in dist/linux
./bundle.sh windows # this will bundle for windows, in dist/windows
```

# Upcoming Features

-   **Vulkan Rendering**
-   **Android Support**
-   **Flowchart Editor (like in Unreal Engine)**
-   **Video Player**
