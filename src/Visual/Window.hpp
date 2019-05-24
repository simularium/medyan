#ifndef MEDYAN_Visual_Window_Hpp
#define MEDYAN_Visual_Window_Hpp

#include <iostream> // cout, endl
#include <vector>

#include "Util/Environment.h"
#include "util/io/log.h"
#include "Visual/Common.hpp"
#include "Visual/Shader.hpp"

#ifdef VISUAL

// For best portability, the window signal handling could only be done from the
// main thread (due to MacOS Cocoa framework).

namespace visual {

struct Window {
    GLFWwindow* window;
}; // Currently not used

namespace state {
// Defines variables used in the main thread

enum class ProjectionType { Orthographic, Perspective };

// settings
int windowWidth = 1200;
int windowHeight = 800;
ProjectionType projType = ProjectionType::Perspective;
float fov = glm::radians(45.0f); // perspective
float nearDistance = 0.1f;
float farDistance = 100.0f;
glm::mat4 projection;
glm::mat4 view;
glm::mat4 model;

GLFWwindow* window;
unsigned int vao;
unsigned int vbo;
unsigned int ebo;
Shader sd;

} // namespace state

inline void glfwError(int id, const char* description) {
    LOG(ERROR) << description;
    system("pause");
}

inline void framebuffer_size_callback(GLFWwindow* window, int width, int height) {
    state::windowWidth = width;
    state::windowHeight = height;
    glViewport(0, 0, width, height);
}
inline void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
        LOG(INFO) << "Escape key hit!";
        glfwSetWindowShouldClose(window, true);
    }
}

inline void createWindow() {
    LOG(INFO) << "Initializing GLFW";
    glfwSetErrorCallback(&glfwError);
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    state::window = glfwCreateWindow(state::windowWidth, state::windowHeight, "MEDYAN", NULL, NULL);
    if(state::window == NULL) {
        LOG(ERROR) << "Failed to create GLFW window";
        glfwTerminate();
        return;
    }
    glfwMakeContextCurrent(state::window);

    LOG(INFO) << "initializing GLAD";
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
        LOG(ERROR) << "Failed to initialize GLAD";
        return;
    }
    glViewport(0, 0, state::windowWidth, state::windowHeight);
    glfwSetFramebufferSizeCallback(state::window, framebuffer_size_callback);

    // Configure global opengl state
    glEnable(GL_DEPTH_TEST);

    // Shader
    const char* const vertexshader = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main() {
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

    const char* const fragmentshader = R"(
#version 330 core
out vec4 FragColor;

void main() {
    FragColor = vec4(0.5f, 0.25f, 0.1f, 1.0f);
}
)";
    state::sd.init(vertexshader, fragmentshader);

    // Set up vertex
    glGenBuffers(1, &state::vbo);
    glGenVertexArrays(1, &state::vao);
    glGenBuffers(1, &state::ebo);
    glBindVertexArray(state::vao); // Bind this first!

    glBindBuffer(GL_ARRAY_BUFFER, state::vbo);
    float vertices[]{
        -15.5f, -25.9f, 8.0f,
        30.0f,-0.5f,0.0f,
        0.0f, 30.0f,-7.0f,
        -9.9f,0.4f,0.1f
    };
    unsigned int indices[]{
        0, 1, 2,
        1, 2, 3
    };
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, state::ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Vertex attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    // ^^^ also register vbo as bound
    glEnableVertexAttribArray(0);

    // temporarily retarget
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // Draw wireframe
    //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);

}

// The main loop for all windows.
// Note:
//   - This function must be called from the main thread.
inline void mainLoop() {
    // Loop
    LOG(INFO) << "Entering main loop";

    while (!glfwWindowShouldClose(state::window)) {
        // input
        processInput(state::window);

        // rendering
        glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // transform
        state::projection = glm::perspective(state::fov, (float)state::windowWidth / (float)state::windowHeight, state::nearDistance, state::farDistance);
        state::sd.setMat4("projection", state::projection);
        state::model = glm::mat4(1.0f);
        state::model = glm::rotate(state::model, (float)glfwGetTime(), glm::vec3(0.6f, 0.8f, 0.0f));
        state::sd.setMat4("model", state::model);
        state::view = glm::mat4(1.0f);
        state::view = glm::translate(state::view, glm::vec3(0.0f, 0.0f, -60.0f));
        state::sd.setMat4("view", state::view);

        glUseProgram(state::sd.id);
        glBindVertexArray(state::vao);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        // glBindVertexArray(0); // no need to unbind every time

        // check
        glfwSwapBuffers(state::window);
        glfwPollEvents();
    }
}

inline void deallocate() {

    // Deallocate resources
    glDeleteVertexArrays(1, &state::vao);
    glDeleteBuffers(1, &state::vbo);

    glfwTerminate();

}

} // namespace visual

#endif // ifdef VISUAL

#endif
