#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <cmath>
#include <iostream>
#include <string>
#include <fstream>
#include "scene.hpp"

uint32_t compile_shader(const char *src, GLenum type) {
    uint32_t id = glCreateShader(type);
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int success = 0;
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if(success) {
        return id;
    }

    int len = 0;
    glGetShaderiv(id, GL_INFO_LOG_LENGTH, &len);

    char *log = new char[len];
    glGetShaderInfoLog(id, len, nullptr, log);
    std::cerr << log << std::endl;
    delete[] log;

    return 0;
}

uint32_t link_program(uint32_t vs, uint32_t fs) {
    uint32_t id = glCreateProgram();
    glAttachShader(id, vs);
    glAttachShader(id, fs);
    glLinkProgram(id);
    glDetachShader(id, vs);
    glDetachShader(id, fs);

    int success = 0;
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if(success) {
        return id;
    }

    int len = 0;
    glGetProgramiv(id, GL_INFO_LOG_LENGTH, &len);

    char *log = new char[len];
    glGetProgramInfoLog(id, len, nullptr, log);
    std::cerr << log << std::endl;
    delete[] log;

    return 0;
}

uint32_t create_program(const char *vsrc, const char *fsrc) {
    uint32_t vs = compile_shader(vsrc, GL_VERTEX_SHADER);
    uint32_t fs = compile_shader(fsrc, GL_FRAGMENT_SHADER);
    uint32_t program = link_program(vs, fs);
    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

std::string read_txt_file(const char *path) {
    std::ifstream file(path);
    if(!file.is_open()) {
        return "";
    }

    std::string line, content;
    while(std::getline(file, line)) {
        content.append(line);
        content.append("\n");
    }

    return content;
}

static void _key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    if(key == GLFW_KEY_ESCAPE && action == GLFW_RELEASE) {
        glfwSetInputMode(window, GLFW_CURSOR, 
            glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED ? GLFW_CURSOR_NORMAL : GLFW_CURSOR_DISABLED
        );
    }
}

static void _frame_buffer_callback(GLFWwindow *window, int width, int height) {
    glViewport(0, 0, width, height);
}

int main(void) {
    if(!glfwInit()) {
        return 1;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
    GLFWwindow *window = glfwCreateWindow(900, 900, "raymarcher", nullptr, nullptr);

    if(!window) {
        return 1;
    }

    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);
    glfwSetKeyCallback(window, _key_callback);
    glfwSetFramebufferSizeCallback(window, _frame_buffer_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io = ImGui::GetIO();
    (void) io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;

    if(glewInit() != GLEW_OK) {
        return 1;
    }

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    uint32_t vao = 0;
    uint32_t vbo = 0;
    uint32_t ibo = 0;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ibo);

    glBindVertexArray(vao);

    float vertices[] = {
        -1.0f, -1.0f,
        -1.0f,  1.0f,
         1.0f, -1.0f,
         1.0f,  1.0f
    };

    uint32_t indices[] = {
        0, 1, 2,
        2, 3, 1
    };

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), nullptr);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    Scene scene {};
    scene.program = create_program(read_txt_file("shaders/display.glsl").c_str(), read_txt_file("shaders/scene.glsl").c_str());

    scene.add_object({
        .type = SPHERE,
        .pos = glm::vec3(3.0f, 3.0f, 0.0f),
        .material = 1,
        .radius = 1.0f
    });
    scene.add_object({
        .type = BOX,
        .pos = glm::vec3(1.0f, 2.0f, 5.0f),
        .material = 2,
        .size = glm::vec3(1.0f, 2.0f, 1.0f)
    });
    scene.add_object({
        .type = PLANE,
        .material = 3,
        .normal = glm::vec4(0.0f, 1.0f, 0.0f, 1.0f)
    });
    scene.add_object({
        .type = TORUS,
        .pos = glm::vec3(-3.0f, 5.0f, 9.0f),
        .material = 2,
        .torus = glm::vec2(0.5f, 0.25f)
    });

    glfwShowWindow(window);

    glm::vec3 pos(0.0f);
    glm::vec3 dir(0.0f);
    glm::vec3 up(0.0f, 1.0f, 0.0f);
    const float speed = 2.0f;

    float yaw = -90.0f;
    float pitch = 0.0f;

    float dx = 0.0f;
    float dy = 0.0f;
    float last_x = 0.0f;
    float last_y = 0.0f;

    float dt = 0.0f;
    float last_time = 0.0f;
    while(!glfwWindowShouldClose(window)) {
        glfwPollEvents();

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        if(glfwGetInputMode(window, GLFW_CURSOR) == GLFW_CURSOR_DISABLED) {
            double x = 0.0;
            double y = 0.0;
            glfwGetCursorPos(window, &x, &y);

            dx = x - last_x;
            dy = last_y - y;
            last_x = x;
            last_y = y;

            yaw -= dx * 0.1f;
            pitch -= dy * 0.1f;

            if(pitch > 89.0f) {
                pitch = 89.0f;
            } else if(pitch < -89.0f) {
                pitch = -89.0f;
            }

            dir.x = std::cos(glm::radians(yaw)) * std::cos(glm::radians(pitch));
            dir.y = std::sin(glm::radians(pitch));
            dir.z = std::sin(glm::radians(yaw)) * std::cos(glm::radians(pitch));
            dir = glm::normalize(dir);
        }

        glm::mat4 view = glm::inverse(glm::lookAt(pos, pos + dir, up));

        glm::vec3 right = glm::normalize(glm::cross(dir, up));
        if(glfwGetKey(window, GLFW_KEY_W) != GLFW_RELEASE) {
            pos -= dir * speed * dt;
        }
        if(glfwGetKey(window, GLFW_KEY_S) != GLFW_RELEASE) {
            pos += dir * speed * dt;
        }
        if(glfwGetKey(window, GLFW_KEY_D) != GLFW_RELEASE) {
            pos += right * speed * dt;
        }
        if(glfwGetKey(window, GLFW_KEY_A) != GLFW_RELEASE) {
            pos -= right * speed * dt;
        }

        glClear(GL_COLOR_BUFFER_BIT);

        glUseProgram(scene.program);

        glUniform3fv(glGetUniformLocation(scene.program, "cam_pos"), 1, glm::value_ptr(pos));
        glUniformMatrix4fv(glGetUniformLocation(scene.program, "view"), 1, GL_FALSE, glm::value_ptr(view));

        glBindVertexArray(vao);

        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);

        glUseProgram(0);
        glBindVertexArray(0);

        ImGui::Begin("Debug Menu");

        ImGui::DragFloat3("Position", glm::value_ptr(pos));

        ImGui::End();

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        glfwSwapBuffers(window);

        float now = glfwGetTime();
        dt = now - last_time;
        last_time = now;
    }

    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    glDeleteVertexArrays(1, &vao);
    glDeleteBuffers(1, &vbo);
    glDeleteBuffers(1, &ibo);
    glDeleteProgram(scene.program);

    glfwDestroyWindow(window);
    glfwTerminate();

    return 0;
}
