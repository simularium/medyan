#ifndef MEDYAN_Visual_Window_Hpp
#define MEDYAN_Visual_Window_Hpp

#include <array>
#include <iostream> // cout, endl
#include <stdexcept> // runtime_error
#include <vector>

#include "Util/Environment.hpp"
#include "Util/Io/Log.hpp"
#include "Visual/Camera.hpp"
#include "Visual/Common.hpp"
#include "Visual/Shader.hpp"
#include "Visual/ShaderSrc.hpp"
#include "Visual/SharedData.hpp"
#include "Visual/VisualElement.hpp"

#ifdef VISUAL

// For best portability, the window signal handling could only be done from the
// main thread (due to MacOS Cocoa framework).

namespace visual {

struct Window {
    GLFWwindow* window;
}; // Currently not used

inline void glfwError(int id, const char* description) {
    LOG(ERROR) << description;
    throw std::runtime_error("Error in GLFW environment");
}


template< typename T >
inline void replaceBuffer(GLenum target, const std::vector<T>& source) {
    GLint prevSize;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &prevSize);

    const std::size_t newSize = sizeof(T) * source.size();

    if(newSize > prevSize) {
        glBufferData(target, newSize, source.data(), GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(target, 0, newSize, source.data());
    }
} 


// The RAII object for managing visualization context and window
//
// Note:
//   - Only one object of this type can be created at a time.
//   - The window signal handling should only be done from the main thread (due
//     to MacOS Cocoa framework).
class VisualContext {
public:
    struct WindowStates {
        struct Transformation {
            enum class ProjectionType { Orthographic, Perspective };

            ProjectionType projType = ProjectionType::Perspective;
            float fov               = glm::radians(45.0f); // perspective
            float zNear      = 10.0f;
            float zFar       = 20000.0f;
            glm::mat4 projection;
            glm::mat4 model;

            Camera camera;
        };

        // Size
        int width = 1200;
        int height = 800;

        // Model, view, perspective...
        Transformation trans;

        // Other states
        float deltaTime = 0.01f;
        float lastTime  = 0.0f;
        bool mouseLeftAlreadyPressed = false;
        double mouseLastX;
        double mouseLastY;
    };

    VisualContext() {
        // GLFW initializing
        LOG(DEBUG) << "Initializing GLFW";
        glfwSetErrorCallback(&glfwError);
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

        // Window initializing
        window_ = glfwCreateWindow(windowStates_.width, windowStates_.height, "MEDYAN", NULL, NULL);
        if(window_ == NULL) {
            LOG(ERROR) << "Failed to create GLFW window";
            glfwTerminate();
            return;
        }
        glfwMakeContextCurrent(window_);

        // GLAD initializing
        LOG(DEBUG) << "initializing GLAD";
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
            LOG(ERROR) << "Failed to initialize GLAD";
            return;
        }

        glViewport(0, 0, windowStates_.width, windowStates_.height);
        glfwSetWindowUserPointer(window_, &windowStates_);

        // Set window callbacks
        const auto framebufferSizeCallback = [](GLFWwindow* window, int width, int height) {
            auto ws = static_cast< WindowStates* >(glfwGetWindowUserPointer(window));
            ws->width = width;
            ws->height = height;
            glViewport(0, 0, width, height);
        };
        const auto cursorPositionCallback = [](GLFWwindow* window, double xpos, double ypos) {
            auto ws = static_cast< WindowStates* >(glfwGetWindowUserPointer(window));
            auto& camera = ws->trans.camera;

            const int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if(mouseState == GLFW_PRESS) {
                if(ws->mouseLeftAlreadyPressed) {
                    // Transform
                    const double dist = glm::distance(camera.target, camera.position);

                    camera.position -= (camera.right * float(xpos - ws->mouseLastX) + camera.up * float(ws->mouseLastY - ypos)) * (float)camera.mouseControlSpeed;
                    camera.position = camera.target + glm::normalize(camera.position - camera.target) * (float)dist;
                    
                    // Update direction
                    camera.right = glm::normalize(glm::cross(camera.target - camera.position, camera.up));
                    camera.up = glm::normalize(glm::cross(camera.right, camera.target - camera.position));

                } else {
                    ws->mouseLeftAlreadyPressed = true;
                }
                ws->mouseLastX = xpos;
                ws->mouseLastY = ypos;
            } else {
                ws->mouseLeftAlreadyPressed = false;
            }
        };
        const auto scrollCallback = [](GLFWwindow* window, double xoffset, double yoffset) {
            auto ws = static_cast< WindowStates* >(glfwGetWindowUserPointer(window));
            auto& fov = ws->trans.fov;

            fov -= 0.02 * yoffset;
            if(fov < 0.01f) fov = 0.01f;
            if(fov > 3.13f) fov = 3.13f;
        };

        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
        glfwSetCursorPosCallback(window_, cursorPositionCallback);
        glfwSetScrollCallback(window_, scrollCallback);

    } // VisualContext()

    ~VisualContext() {
        glfwTerminate();
    }

    auto window() const { return window_; }
    auto&       windowStates()       { return windowStates_; }
    const auto& windowStates() const { return windowStates_; }

    // Helper function to process window inputs
    void processInput() {

        auto& camera = windowStates_.trans.camera;

        const float currentTime = glfwGetTime();
        windowStates_.deltaTime = currentTime - windowStates_.lastTime;
        windowStates_.lastTime = currentTime;
        const float cameraMove = camera.keyControlSpeed * windowStates_.deltaTime;

        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            LOG(INFO) << "Escape key hit!";
            glfwSetWindowShouldClose(window_, true);
        }

        if(glfwGetKey(window_, GLFW_KEY_W) == GLFW_PRESS) {
            const auto change = cameraMove * glm::normalize(camera.target - camera.position);
            camera.position += change;
            camera.target += change;
        }
        if(glfwGetKey(window_, GLFW_KEY_S) == GLFW_PRESS) {
            const auto change = cameraMove * glm::normalize(camera.target - camera.position);
            camera.position -= change;
            camera.target -= change;
        }
        if(glfwGetKey(window_, GLFW_KEY_A) == GLFW_PRESS) {
            const auto change = glm::normalize(glm::cross(glm::normalize(camera.target - camera.position), camera.up)) * cameraMove;
            camera.position -= change;
            camera.target -= change;
        }
        if(glfwGetKey(window_, GLFW_KEY_D) == GLFW_PRESS) {
            const auto change = glm::normalize(glm::cross(glm::normalize(camera.target - camera.position), camera.up)) * cameraMove;
            camera.position += change;
            camera.target += change;
        }
    }

private:
    GLFWwindow* window_;
    WindowStates windowStates_;
}; // VisualContext


// The RAII object for all the rendering process
struct VisualDisplay {
    // The overall opengl context. Must be at top
    VisualContext vc;

    // Visual presets
    std::array< VisualPreset, 2 > vps {{
        { GlSize {9, 0, 3, 3, 3, 6, 3}, shader::VertexElementLight, shader::FragElementLight },
        { GlSize {6, 0, 3, 3, 0, 3, 3}, shader::VertexElementLine,  shader::FragElementLine  }
    }};
    VisualPreset& vpLight = vps[0];
    VisualPreset& vpLine  = vps[1];

    VisualDisplay() {
        // Configure global opengl state
        glEnable(GL_DEPTH_TEST);

        // Setup visual with light
        {
            auto& vp = vpLight;
            std::lock_guard< std::mutex > guard(vp.veMutex);

            const auto newVe = [&]() -> auto& {
                vp.visualElements.emplace_back(new VisualElement(vp.size));
                return vp.visualElements.back();
            };

            {
                auto& ve = newVe();
                ve->profile.enabled = true;
                ve->profile.flag = Profile::targetMembrane;
                ve->profile.colorAmbient = glm::vec3(0.4f, 0.6f, 0.95f);
                ve->profile.colorDiffuse = glm::vec3(0.4f, 0.6f, 0.95f);
            }
            {
                auto& ve = newVe();
                ve->profile.enabled = true;
                ve->profile.flag = Profile::targetFilament;
                ve->profile.pathMode = Profile::PathMode::Extrude;
                ve->profile.polygonMode = GL_FILL;
                ve->profile.pathExtrudeRadius = 7.5f;
                ve->profile.pathExtrudeSides = 15;
                ve->profile.colorAmbient = glm::vec3(0.95f, 0.1f, 0.15f);
                ve->profile.colorDiffuse = glm::vec3(0.95f, 0.1f, 0.15f);
            }
            {
                auto& ve = newVe();
                ve->profile.enabled = true;
                ve->profile.flag = Profile::targetLinker;
                ve->profile.polygonMode = GL_FILL;
                ve->profile.pathExtrudeRadius = 8.0f;
                ve->profile.pathExtrudeSides = 15;
                ve->profile.colorAmbient = glm::vec3(0.1f, 0.9f, 0.0f);
                ve->profile.colorDiffuse = glm::vec3(0.1f, 0.9f, 0.0f);
            }
            {
                auto& ve = newVe();
                ve->profile.enabled = true;
                ve->profile.flag = Profile::targetMotor;
                ve->profile.polygonMode = GL_FILL;
                ve->profile.pathExtrudeRadius = 7.5f;
                ve->profile.pathExtrudeSides = 10;
                ve->profile.colorAmbient = glm::vec3(0.1f, 0.1f, 0.99f);
                ve->profile.colorDiffuse = glm::vec3(0.1f, 0.1f, 0.99f);
            }
            {
                auto& ve = newVe();
                ve->profile.enabled = true;
                ve->profile.flag = Profile::targetBrancher;
                ve->profile.polygonMode = GL_FILL;
                ve->profile.pathExtrudeRadius = 10.0f;
                ve->profile.pathExtrudeSides = 15;
                ve->profile.colorAmbient = glm::vec3(0.95f, 0.9f, 0.05f);
                ve->profile.colorDiffuse = glm::vec3(0.95f, 0.9f, 0.05f);
            }
        } // ~lock_guard (vpLight)

        // Setup visual for non-lights
        {
            auto& vp = vpLine;
            std::lock_guard< std::mutex > guard(vp.veMutex);

            const auto newVe = [&]() -> auto& {
                vp.visualElements.emplace_back(new VisualElement(vp.size));
                return vp.visualElements.back();
            };

            {
                auto& ve = newVe();
                ve->state.eleMode = GL_LINES;
                ve->profile.enabled = true;
                ve->profile.flag = Profile::targetCompartment;
                ve->profile.colorDiffuse = glm::vec3(0.8f, 0.8f, 0.8f);
            }
            {
                auto& ve = newVe();
                ve->profile.enabled = true;
                ve->profile.flag = Profile::targetMembrane | Profile::displayForce;
                ve->profile.colorDiffuse = glm::vec3(0.3f, 0.6f, 0.95f);
            }
            {
                auto& ve = newVe();
                ve->profile.enabled = true;
                ve->profile.flag = Profile::targetFilament | Profile::displayForce;
                ve->profile.colorDiffuse = glm::vec3(0.95f, 0.1f, 0.15f);
            }
        } // ~lock_guard (vpLine)

    } // VisualDisplay()

    void run() {

        while (!glfwWindowShouldClose(vc.window())) {
            // input
            vc.processInput();

            auto& ws = vc.windowStates();

            // rendering
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // transform
            ws.trans.projection = glm::perspective(ws.trans.fov, (float)ws.width / (float)ws.height, ws.trans.zNear, ws.trans.zFar);
            ws.trans.model      = glm::mat4(1.0f);
            glm::mat3 modelInvTrans3(glm::transpose(glm::inverse(ws.trans.model)));

            for(auto& vp : vps) {
                std::lock_guard< std::mutex > guard(vp.veMutex);

                if(!vp.visualElements.empty()) {
                    glUseProgram(vp.shader.id());

                    vp.shader.setMat4("projection", ws.trans.projection);
                    vp.shader.setMat4("model",      ws.trans.model);
                    vp.shader.setMat3("modelInvTrans3", modelInvTrans3);
                    vp.shader.setMat4("view",       ws.trans.camera.view());

                    if(&vp == &vpLight) {
                        vp.shader.setVec3("CameraPos",  ws.trans.camera.position);

                        vp.shader.setVec3("dirLights[0].direction", glm::vec3 {1.0f, 1.0f, 1.0f});
                        vp.shader.setVec3("dirLights[0].ambient",   glm::vec3 {0.05f, 0.05f, 0.05f});
                        vp.shader.setVec3("dirLights[0].diffuse",   glm::vec3 {0.3f, 0.3f, 0.3f});
                        vp.shader.setVec3("dirLights[0].specular",  glm::vec3 {0.5f, 0.5f, 0.5f});
                        vp.shader.setVec3("dirLights[1].direction", glm::vec3 {-1.0f, -1.0f, -1.0f});
                        vp.shader.setVec3("dirLights[1].ambient",   glm::vec3 {0.05f, 0.05f, 0.05f});
                        vp.shader.setVec3("dirLights[1].diffuse",   glm::vec3 {0.3f, 0.3f, 0.3f});
                        vp.shader.setVec3("dirLights[1].specular",  glm::vec3 {0.5f, 0.5f, 0.5f});

                        const glm::vec3 pointLightPositions[4] {
                            { -500.0f, -500.0f, -500.0f },
                            { -500.0f, 3500.0f, 3500.0f },
                            { 3500.0f, -500.0f, 3500.0f },
                            { 3500.0f, 3500.0f, -500.0f }
                        };
                        vp.shader.setVec3("pointLights[0].position", pointLightPositions[0]);
                        vp.shader.setVec3("pointLights[0].ambient",  glm::vec3 { 0.0f, 0.0f, 0.0f });
                        vp.shader.setVec3("pointLights[0].diffuse",  glm::vec3 { 0.6f, 0.6f, 0.6f });
                        vp.shader.setVec3("pointLights[0].specular", glm::vec3 { 1.0f, 1.0f, 1.0f });
                        vp.shader.setFloat("pointLights[0].constant",  1.0f);
                        vp.shader.setFloat("pointLights[0].linear",    1.4e-4f);
                        vp.shader.setFloat("pointLights[0].quadratic", 7.2e-8f);
                        vp.shader.setVec3("pointLights[1].position", pointLightPositions[1]);
                        vp.shader.setVec3("pointLights[1].ambient",  glm::vec3 { 0.0f, 0.0f, 0.0f });
                        vp.shader.setVec3("pointLights[1].diffuse",  glm::vec3 { 0.6f, 0.6f, 0.6f });
                        vp.shader.setVec3("pointLights[1].specular", glm::vec3 { 1.0f, 1.0f, 1.0f });
                        vp.shader.setFloat("pointLights[1].constant",  1.0f);
                        vp.shader.setFloat("pointLights[1].linear",    1.4e-4f);
                        vp.shader.setFloat("pointLights[1].quadratic", 7.2e-8f);
                        vp.shader.setVec3("pointLights[2].position", pointLightPositions[2]);
                        vp.shader.setVec3("pointLights[2].ambient",  glm::vec3 { 0.0f, 0.0f, 0.0f });
                        vp.shader.setVec3("pointLights[2].diffuse",  glm::vec3 { 0.0f, 0.0f, 0.0f });
                        vp.shader.setVec3("pointLights[2].specular", glm::vec3 { 0.0f, 0.0f, 0.0f });
                        vp.shader.setFloat("pointLights[2].constant",  1.0f);
                        vp.shader.setFloat("pointLights[2].linear",    1.4e-4f);
                        vp.shader.setFloat("pointLights[2].quadratic", 7.2e-8f);
                        vp.shader.setVec3("pointLights[3].position", pointLightPositions[3]);
                        vp.shader.setVec3("pointLights[3].ambient",  glm::vec3 { 0.0f, 0.0f, 0.0f });
                        vp.shader.setVec3("pointLights[3].diffuse",  glm::vec3 { 0.0f, 0.0f, 0.0f });
                        vp.shader.setVec3("pointLights[3].specular", glm::vec3 { 0.0f, 0.0f, 0.0f });
                        vp.shader.setFloat("pointLights[3].constant",  1.0f);
                        vp.shader.setFloat("pointLights[3].linear",    1.4e-4f);
                        vp.shader.setFloat("pointLights[3].quadratic", 7.2e-8f);
                    }

                    for(const auto& ve : vp.visualElements) {
                        std::lock_guard< std::mutex > guard(ve->me);

                        if(&vp == &vpLight) {
                            vp.shader.setVec3("material.diffuse",  ve->profile.colorDiffuse);
                            vp.shader.setVec3("material.specular", ve->profile.colorSpecular);
                            vp.shader.setFloat("material.shininess", ve->profile.colorShininess);
                        }

                        glBindVertexArray(ve->state.vao);

                        glPolygonMode(GL_FRONT_AND_BACK, ve->profile.polygonMode);

                        // Update data
                        if(ve->state.attribChanged) {
                            glBindBuffer(GL_ARRAY_BUFFER, ve->state.vbo);
                            replaceBuffer(GL_ARRAY_BUFFER, ve->state.vertexAttribs);
                            ve->state.attribChanged = false;
                        }

                        // Draw
                        glDrawArrays(ve->state.eleMode, 0, ve->state.vertexAttribs.size() / ve->state.size.vaStride);
                        // glDrawElements(ve->state.eleMode, ve->state.vertexIndices.size(), GL_UNSIGNED_INT, (void*)0);
                    }
                }
            } // End loop visual presets
            glBindVertexArray(0);

            // check
            glfwSwapBuffers(vc.window());
            glfwPollEvents();

        } // End main loop

    } // void run() const
}; // VisualDisplay

} // namespace visual

#endif // ifdef VISUAL

#endif
