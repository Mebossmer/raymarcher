cmake_minimum_required(VERSION 3.0.0)

project("raymarcher" LANGUAGES CXX)

add_executable("raymarcher"
    "main.cpp"
    "scene.cpp"
    "imgui/imgui_demo.cpp"
    "imgui/imgui_draw.cpp"
    "imgui/imgui_impl_glfw.cpp"
    "imgui/imgui_impl_opengl3.cpp"
    "imgui/imgui_tables.cpp"
    "imgui/imgui_widgets.cpp"
    "imgui/imgui.cpp"
)
target_link_libraries("raymarcher"
    "glfw3" "opengl32" "glew32"
)
target_include_directories("raymarcher"
    PRIVATE "imgui"
)
