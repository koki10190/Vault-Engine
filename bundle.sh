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
    
    cp win_build.exe dist/windows/win_build.exe
    cp win_proj.exe dist/windows/Vault\ Engine.exe
    cp *.dll dist/windows/
    cp -r assets dist/windows/assets
    cp -r imgui.ini dist/windows/imgui.ini
    cp -r shaders dist/windows/shaders
    cp -r build dist/windows/build
else 
    if [ -d dist/linux ]; then
        rm -r dist/linux
        mkdir dist/linux
    else
        mkdir dist/linux
    fi

    cp -r lib dist/linux/lib
    cp -r build.out dist/linux/build.out
    cp -r build_proj.out dist/linux/build_proj.out
    cp -r Vault\ Engine.sh dist/linux/Vault\ Engine

    cp -r assets dist/linux/assets
    cp -r imgui.ini dist/linux/imgui.ini
    cp -r shaders dist/linux/shaders
    cp -r build dist/linux/build
fi