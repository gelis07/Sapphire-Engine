cmake_minimum_required(VERSION 3.5)

project(Sapphire-Engine)

# Set the C++ standard (change as needed)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Add the executable target
add_executable(Sapphire-Engine
    src/main.cpp
    src/Graphics/Shapes.cpp
    src/Engine/Scenes.cpp
    src/Engine/Engine.cpp
    src/Engine/Views/PlayMode.cpp
    src/RunTime/RunTime.cpp
    src/Engine/Views/SceneEditor.cpp
    src/Graphics/Grid.cpp
    src/Scripting/Scripting.cpp
    src/Scripting/Variables.cpp
    src/Objects/Objects.cpp
    src/Scripting/LuaUtilities.cpp
    src/Scripting/Components.cpp
    src/UI/FileExplorer/File.cpp
    src/UI/Windows.cpp
    src/UI/FileExplorer/FileExplorer.cpp
    src/UI/ProjectManager.cpp
    src/stb_image.cpp
    include/imgui/imgui_draw.cpp
    include/imgui/imgui_impl_glfw.cpp
    include/imgui/imgui_impl_opengl3.cpp
    include/imgui/imgui.cpp
    include/imgui/imgui_stdlib.cpp
    include/imgui/imgui_tables.cpp
    include/imgui/ImGuiFileDialog.cpp
    include/imgui/imgui_widgets.cpp
    include/imgui/ImGuizmo.cpp
    src/PhysicsEngine/RigidBody.cpp
    src/PhysicsEngine/CollisionDetection.cpp
)

# Add include directories
target_include_directories(Sapphire-Engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include/
    ${CMAKE_CURRENT_SOURCE_DIR}/src/
)

# Add library directories
target_link_directories(Sapphire-Engine PRIVATE
    ${CMAKE_SOURCE_DIR}/lib
)

# Add library dependencies
target_link_libraries(Sapphire-Engine PRIVATE
    stdc++fs
    glfw3dll
    glew32s
    lua54
    opengl32
)

# Set additional compiler options and flags
target_compile_options(Sapphire-Engine PRIVATE
    -fdiagnostics-color=always
    -g
    -std=c++17
)

target_precompile_headers(Sapphire-Engine
  PRIVATE src/pch.h)

# Set the output directory for the executable
set_target_properties(Sapphire-Engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/bin"
)

# Specify the CMake generator for GCC
if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-dev")
endif()