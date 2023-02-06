PLATFORM=$1

if [ "$PLATFORM" = "windows" ]; then
    # check if dist/windows exists
    # if not, create it
    
    if [ -d dist/windows ]; then
        rm -r dist/windows
        mkdir dist/windows
    else
        mkdir dist/windows
    fi

    # if [ -d dist/windows/cpp_headers ]; then
    #     rm -r dist/windows/cpp_headers
    #     mkdir dist/windows/cpp_headers
    # else
    #     mkdir dist/windows/cpp_headers
    # fi
    
    cp windows/win_build.exe dist/windows/win_build.exe
    cp windows/win_proj.exe dist/windows/Vault\ Engine.exe
    cp windows/*.dll dist/windows/
    cp -r assets dist/windows/assets
    cp -r imgui.ini dist/windows/imgui.ini
    cp -r shaders dist/windows/shaders
    cp -r build dist/windows/build 
    cp -r cs-assembly dist/windows/cs-assembly 
    cp -r mono dist/windows/mono
else 
    if [ -d dist/linux ]; then
        rm -r dist/linux
        mkdir dist/linux
    else
        mkdir dist/linux
    fi

    cp -r linux_lib dist/linux/linux_lib
    cp -r build.out dist/linux/build.out
    cp -r build_proj.out dist/linux/build_proj.out
    cp -r Vault\ Engine.sh dist/linux/Vault\ Engine

    cp -r assets dist/linux/assets
    cp -r imgui.ini dist/linux/imgui.ini
    cp -r shaders dist/linux/shaders
    cp -r build dist/linux/build
    cp -r cs-assembly dist/linux/cs-assembly
    cp -r lib dist/linux/lib
fi