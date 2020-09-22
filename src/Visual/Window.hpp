#ifndef MEDYAN_Visual_Window_Hpp
#define MEDYAN_Visual_Window_Hpp

#include <algorithm>
#include <array>
#include <iostream> // cout, endl
#include <stdexcept> // runtime_error
#include <vector>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_WRITE_STATIC
#include "stb/stb_image_write.h"

#include "Util/Environment.hpp"
#include "Util/Io/Log.hpp"
#include "Visual/Common.hpp"
#include "Visual/Control.hpp"
#include "Visual/Gui.hpp"
#include "Visual/Shader.hpp"
#include "Visual/ShaderSrc.hpp"
#include "Visual/VisualElement.hpp"


// For best portability, the window signal handling could only be done from the
// main thread (due to MacOS Cocoa framework).

namespace medyan::visual {


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
    struct OffscreenBufferGuard {
        bool bufferBuilt = false;
        unsigned offscreenFbo = 0;
        unsigned offscreenRbo = 0;
        unsigned offscreenColorRbo = 0;

        // destructor will deallocate the frame buffers
        ~OffscreenBufferGuard() {
            unbuild();
        }

        // build a new buffer
        void build(int width, int height) {
            if(bufferBuilt) unbuild();

            bufferBuilt = true;

            glGenFramebuffers(1, &offscreenFbo);
            glBindFramebuffer(GL_FRAMEBUFFER, offscreenFbo);

            glGenRenderbuffers(1, &offscreenRbo);
            glBindRenderbuffer(GL_RENDERBUFFER, offscreenRbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

            glGenRenderbuffers(1, &offscreenColorRbo);
            glBindRenderbuffer(GL_RENDERBUFFER, offscreenColorRbo);
            glRenderbufferStorage(GL_RENDERBUFFER, GL_RGBA8, width, height);

            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, offscreenRbo);
            glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_RENDERBUFFER, offscreenColorRbo);

            if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
                LOG(ERROR) << "Framebuffer is not complete.";
            }
        }

        void unbuild() {
            if(bufferBuilt) {
                glDeleteFramebuffers(1, &offscreenFbo);
                glDeleteRenderbuffers(1, &offscreenRbo);
                glDeleteRenderbuffers(1, &offscreenColorRbo);
                bufferBuilt = false;
            }
        }
    };



    // Display settings and states
    DisplaySettings displaySettings;
    DisplayStates   displayStates;


    VisualContext() {
        // GLFW initializing
        LOG(DEBUG) << "Initializing GLFW";
        glfwSetErrorCallback(&glfwError);
        glfwInit();
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
        glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
        // The following line is generally useless, but it is fun to see a transparent window.
        // glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);

        // Window initializing
        window_ = glfwCreateWindow(displaySettings.mainView.canvas.width, displaySettings.mainView.canvas.height, "MEDYAN", NULL, NULL);
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

        glViewport(0, 0, displaySettings.mainView.canvas.width, displaySettings.mainView.canvas.height);
        glfwSetWindowUserPointer(window_, this);

        // Set window callbacks
        const auto framebufferSizeCallback = [](GLFWwindow* window, int width, int height) {
            auto& vc = *static_cast< VisualContext* >(glfwGetWindowUserPointer(window));
            auto& canvas = vc.displaySettings.mainView.canvas;
            canvas.width = width;
            canvas.height = height;
            glViewport(0, 0, width, height);
        };
        const auto cursorPositionCallback = [](GLFWwindow* window, double xpos, double ypos) {
            auto& vc = *static_cast< VisualContext* >(glfwGetWindowUserPointer(window));
            auto& controlStates = vc.displayStates.mainView.control;
            auto& camera = vc.displaySettings.mainView.camera;
            const auto& control = vc.displaySettings.mainView.control;

            const int mouseState = glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT);
            if(mouseState == GLFW_PRESS) {
                if(controlStates.mouseLeftAlreadyPressed) {
                    mouseDragCamera(
                        vc.displaySettings.mainView.camera,
                        vc.displayStates.mainView.control.mouseLastX,
                        vc.displayStates.mainView.control.mouseLastY,
                        xpos,
                        ypos,
                        vc.displaySettings.mainView.control
                    );

                } else {
                    controlStates.mouseLeftAlreadyPressed = true;
                }
                controlStates.mouseLastX = xpos;
                controlStates.mouseLastY = ypos;
            } else {
                controlStates.mouseLeftAlreadyPressed = false;
            }
        };
        const auto scrollCallback = [](GLFWwindow* window, double xoffset, double yoffset) {
            auto& vc = *static_cast< VisualContext* >(glfwGetWindowUserPointer(window));
            auto& proj = vc.displaySettings.mainView.projection;
            auto& fov = proj.fov;
            auto& scale = proj.scale;

            fov -= 0.02 * yoffset;
            fov = std::clamp(fov, 0.01f, 3.13f);

            scale -= 0.05 * yoffset;
            scale = std::clamp(scale, 0.05f, 20.0f);
        };
        const auto keyCallback = [](GLFWwindow* window, int key, int scancode, int action, int mods) {
            auto& vc = *static_cast< VisualContext* >(glfwGetWindowUserPointer(window));
            if(key == GLFW_KEY_F && action == GLFW_PRESS) {
                vc.displayStates.mainView.control.snapshotRenderingNextFrame = true;
            }
            if(key == GLFW_KEY_G && action == GLFW_PRESS) {
                vc.displaySettings.gui.enabled = !vc.displaySettings.gui.enabled;
            }
        };

        glfwSetFramebufferSizeCallback(window_, framebufferSizeCallback);
        glfwSetCursorPosCallback(window_, cursorPositionCallback);
        glfwSetScrollCallback(window_, scrollCallback);
        glfwSetKeyCallback(window_, keyCallback);

    } // VisualContext()

    ~VisualContext() {
        glfwTerminate();
    }

    auto window() const { return window_; }

    // Helper function to process window inputs
    void processInput() {

        auto& camera = displaySettings.mainView.camera;

        const float currentTime = glfwGetTime();
        const float deltaTime = currentTime - displayStates.timing.glfwTimeLastFrame;
        displayStates.timing.update(currentTime);
        const float cameraMove = displaySettings.mainView.control.cameraKeyPositionPerFrame * deltaTime;

        if (glfwGetKey(window_, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
            // Do nothing
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

    // Member variables
    GLFWwindow* window_;

}; // VisualContext


// The RAII object for all the rendering process
struct VisualDisplay {
    // The overall opengl context. Must be at top
    VisualContext vc;

    // The ImGui context. Must be below the opengl context
    ImguiGuard guard;

    // Visual presets
    std::array< VisualPreset, 2 > vps {{
        { GlSize {9, 0, 3, 3, 3, 6, 3}, shader::VertexElementLight, shader::FragElementLight },
        { GlSize {6, 0, 3, 3, 0, 3, 3}, shader::VertexElementLine,  shader::FragElementLine  }
    }};
    VisualPreset& vpLight = vps[0];
    VisualPreset& vpLine  = vps[1];

    VisualDisplay() :
        guard(vc.window())
    {
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
        } // ~lock_guard (vpLine)

    } // VisualDisplay()

    void run() {

        while (!glfwWindowShouldClose(vc.window())) {
            // input
            vc.processInput();

            // select frame buffer: on screen display / offscreen snapshot
            const bool offscreen = vc.displayStates.mainView.control.snapshotRenderingNextFrame;
            vc.displayStates.mainView.control.snapshotRenderingNextFrame = false;
            auto width  = vc.displaySettings.mainView.canvas.width;
            auto height = vc.displaySettings.mainView.canvas.height;
            VisualContext::OffscreenBufferGuard offscreenBufferGuard;
            if(offscreen) {
                offscreenBufferGuard.build(width, height);
            }
            glBindFramebuffer(GL_FRAMEBUFFER, offscreen ? offscreenBufferGuard.offscreenFbo : 0);

            glViewport(0, 0, width, height);
            {
                const auto& c = vc.displaySettings.mainView.canvas.bgColor;
                glClearColor(c[0], c[1], c[2], c[3]);
            }
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // transform
            const auto model          = glm::mat4(1.0f);
            const auto modelInvTrans3 = glm::transpose(glm::inverse(model));
            const auto view           = vc.displaySettings.mainView.camera.view();
            const auto projection     = vc.displaySettings.mainView.projection.proj(width, height);

            for(auto& vp : vps) {
                std::lock_guard< std::mutex > guard(vp.veMutex);

                if(!vp.visualElements.empty()) {
                    glUseProgram(vp.shader.id());

                    vp.shader.setMat4("projection",     projection);
                    vp.shader.setMat4("model",          model);
                    vp.shader.setMat3("modelInvTrans3", modelInvTrans3);
                    vp.shader.setMat4("view",           view);

                    if(&vp == &vpLight) {
                        vp.shader.setVec3("CameraPos",   vc.displaySettings.mainView.camera.position);

                        vp.shader.setVec3("dirLights[0].direction", glm::vec3 {1.0f, 1.0f, 1.0f});
                        vp.shader.setVec3("dirLights[0].ambient",   glm::vec3 {0.1f, 0.1f, 0.1f});
                        vp.shader.setVec3("dirLights[0].diffuse",   glm::vec3 {0.3f, 0.3f, 0.3f});
                        vp.shader.setVec3("dirLights[0].specular",  glm::vec3 {0.5f, 0.5f, 0.5f});
                        vp.shader.setVec3("dirLights[1].direction", glm::vec3 {-1.0f, -1.0f, -1.0f});
                        vp.shader.setVec3("dirLights[1].ambient",   glm::vec3 {0.1f, 0.1f, 0.1f});
                        vp.shader.setVec3("dirLights[1].diffuse",   glm::vec3 {0.3f, 0.3f, 0.3f});
                        vp.shader.setVec3("dirLights[1].specular",  glm::vec3 {0.5f, 0.5f, 0.5f});

                        const glm::vec3 pointLightPositions[4] {
                            { -500.0f, -500.0f, -500.0f },
                            { -500.0f, 3500.0f, 3500.0f },
                            { 3500.0f, -500.0f, 3500.0f },
                            { 3500.0f, 3500.0f, -500.0f }
                        };
                        vp.shader.setVec3("pointLights[0].position", pointLightPositions[0]);
                        vp.shader.setVec3("pointLights[0].ambient",  glm::vec3 { 0.05f, 0.05f, 0.05f });
                        vp.shader.setVec3("pointLights[0].diffuse",  glm::vec3 { 0.6f, 0.6f, 0.6f });
                        vp.shader.setVec3("pointLights[0].specular", glm::vec3 { 1.0f, 1.0f, 1.0f });
                        vp.shader.setFloat("pointLights[0].constant",  1.0f);
                        vp.shader.setFloat("pointLights[0].linear",    1.4e-4f);
                        vp.shader.setFloat("pointLights[0].quadratic", 7.2e-8f);
                        vp.shader.setVec3("pointLights[1].position", pointLightPositions[1]);
                        vp.shader.setVec3("pointLights[1].ambient",  glm::vec3 { 0.05f, 0.05f, 0.05f });
                        vp.shader.setVec3("pointLights[1].diffuse",  glm::vec3 { 0.6f, 0.6f, 0.6f });
                        vp.shader.setVec3("pointLights[1].specular", glm::vec3 { 1.0f, 1.0f, 1.0f });
                        vp.shader.setFloat("pointLights[1].constant",  1.0f);
                        vp.shader.setFloat("pointLights[1].linear",    1.4e-4f);
                        vp.shader.setFloat("pointLights[1].quadratic", 7.2e-8f);
                        vp.shader.setVec3("pointLights[2].position", pointLightPositions[2]);
                        vp.shader.setVec3("pointLights[2].ambient",  glm::vec3 { 0.05f, 0.05f, 0.05f });
                        vp.shader.setVec3("pointLights[2].diffuse",  glm::vec3 { 0.1f, 0.1f, 0.1f });
                        vp.shader.setVec3("pointLights[2].specular", glm::vec3 { 0.2f, 0.2f, 0.2f });
                        vp.shader.setFloat("pointLights[2].constant",  1.0f);
                        vp.shader.setFloat("pointLights[2].linear",    1.4e-4f);
                        vp.shader.setFloat("pointLights[2].quadratic", 7.2e-8f);
                        vp.shader.setVec3("pointLights[3].position", pointLightPositions[3]);
                        vp.shader.setVec3("pointLights[3].ambient",  glm::vec3 { 0.05f, 0.05f, 0.05f });
                        vp.shader.setVec3("pointLights[3].diffuse",  glm::vec3 { 0.1f, 0.1f, 0.1f });
                        vp.shader.setVec3("pointLights[3].specular", glm::vec3 { 0.2f, 0.2f, 0.2f });
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

                        if(&vp == &vpLight && (ve->profile.flag & Profile::targetMembrane)) {
                            // TODO sort triangles
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

            // Output offscreen render results
            if(offscreen) {
                std::vector< std::uint8_t > data(width * height * 4); // 8-bit color
                glReadBuffer(GL_COLOR_ATTACHMENT0);
                glReadPixels(0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data.data());

                stbi_write_png(vc.displaySettings.mainView.control.snapshotFile.string().c_str(), width, height, 4, data.data(), 4 * width);
                LOG(INFO) << "Snapshot saved to " << vc.displaySettings.mainView.control.snapshotFile;
            }

            // Update GUI
            imguiLoopRender(vc.displaySettings, vc.displayStates);
            // Perform async tasks
            dispatchAnAsyncTask(vc.displayStates.sync);

            // check
            glfwSwapBuffers(vc.window());
            glfwPollEvents();

        } // End main loop

    } // void run() const
}; // VisualDisplay

} // namespace medyan::visual

#endif
