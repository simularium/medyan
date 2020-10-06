#ifndef MEDYAN_Visual_Gui_hpp
#define MEDYAN_Visual_Gui_hpp

#include <type_traits>

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <imgui_stdlib.h>

#include "Visual/DisplaySettings.hpp"
#include "Visual/DisplayStates.hpp"
#include "Visual/FrameData.hpp"

namespace medyan::visual {


// Note:
//   - This must be used while OpenGL and GLFW environments are live
class ImguiGuard {

private:
    ImGuiContext* context_ = nullptr;

public:
    ImguiGuard(GLFWwindow* window) {
        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        context_ = ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        //io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        // Setup Dear ImGui style
        ImGui::StyleColorsDark();
        //ImGui::StyleColorsClassic();

        // Setup Platform/Renderer bindings
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 330 core");

        // Load Fonts
        // - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
        // - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
        // - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
        // - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
        // - Read 'docs/FONTS.md' for more instructions and details.
        // - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
        //io.Fonts->AddFontDefault();
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Roboto-Medium.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/Cousine-Regular.ttf", 15.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/DroidSans.ttf", 16.0f);
        //io.Fonts->AddFontFromFileTTF("../../misc/fonts/ProggyTiny.ttf", 10.0f);
        //ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
        //IM_ASSERT(font != NULL);
    }

    ~ImguiGuard() {
        // Cleanup
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext(context_);

    }
};


//-----------------------------------------------------------------------------
// Auxiliary functions
//-----------------------------------------------------------------------------

// Returns whether the color is changed.
template<
    int colorDof,
    std::enable_if_t< colorDof == 3 || colorDof == 4 >* = nullptr
>
inline bool guiAuxColorPickerPopup(
    const char*        strId, 
    float*             pColor
) {
    bool changed = false;

    const bool bgColorPopup = ImGui::ColorButton(
        strId,
        ImVec4(
            pColor[0],
            pColor[1],
            pColor[2],
            colorDof == 3 ? 1.0f : pColor[3]
        ),
        0
    );
    ImGui::SameLine();
    ImGui::Text(strId);

    if(bgColorPopup) {
        ImGui::OpenPopup(strId);
    }

    if(ImGui::BeginPopup(strId)) {
        if(colorDof == 3) {
            changed = ImGui::ColorPicker3(strId, pColor, ImGuiColorEditFlags_PickerHueWheel);
        } else {
            changed = ImGui::ColorPicker4(strId, pColor, ImGuiColorEditFlags_PickerHueWheel);
        }

        ImGui::EndPopup();
    }

    return changed;
}

inline bool guiAuxColorPicker3Popup(const char* strId, float* pColor) {
    return guiAuxColorPickerPopup<3>(strId, pColor);
}
inline bool guiAuxColorPicker4Popup(const char* strId, float* pColor) {
    return guiAuxColorPickerPopup<4>(strId, pColor);
}

// Function to build combo box automatically for an enumeration type.
//
// Returns whether a new value is selected.
//
// Notes:
//   - The elements in the enum type must be automatically valued (ie the
//     values are automatically 0, 1, 2, ...).
//   - The type must have "last_" as the last element.
//   - A "text(Enum)" function must be implemented to display the elements.
template<
    typename Enum,
    typename Reselect,              // function void(Enum old, Enum new) to execute when selected
    std::enable_if_t<
        std::is_enum_v< Enum > &&
        std::is_invocable_r_v< void, Reselect, Enum, Enum > // Reselect: (Enum, Enum) -> void
    >* = nullptr                    // type requirements
>
inline bool guiAuxEnumComboBox(
    const char*          name,
    Enum&                value,
    Reselect&&           reselect,
    ImGuiSelectableFlags flags = 0
) {
    bool changed = false;

    if(ImGui::BeginCombo(name, text(value), 0)) {
        for (int i = 0; i < underlying(Enum::last_); ++i) {

            const Enum valueI = static_cast<Enum>(i);

            const bool isSelected = (value == valueI);
            if (ImGui::Selectable(text(valueI), isSelected, flags)) {
                const auto oldValue = value;
                value = valueI;
                reselect(oldValue, valueI);

                if(!isSelected) {
                    // selected one item that was previously not selected
                    changed = true;
                }
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    return changed;
}
// Where Reselect function is a no-op.
template<
    typename Enum,
    std::enable_if_t< std::is_enum_v< Enum > >* = nullptr     // type requirements
>
inline bool guiAuxEnumComboBox(
    const char*          name,
    Enum&                value,
    ImGuiSelectableFlags flags = 0
) {
    return guiAuxEnumComboBox(name, value, [](Enum, Enum) {}, flags);
}


//-----------------------------------------------------------------------------
// Main GUI functions
//-----------------------------------------------------------------------------

inline void guiHelpWindow(DisplaySettings& displaySettings) {
    if(!displaySettings.gui.helpWindow) return;

    ImGui::SetNextWindowSize(ImVec2(360, 400), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("help", &displaySettings.gui.helpWindow)) {

        if(ImGui::CollapsingHeader("controls", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("key controls:");
            ImGui::BulletText("f: take snapshot of current screen");
            ImGui::BulletText("g: toggle gui on/off");
            ImGui::BulletText("w/a/s/d: move camera horizontally");
        }
    }


    // End of help window, no matter Begin returns true or false.
    ImGui::End();
}

// Functions for profile configuration
// Returns whether the profile is changed.
inline bool guiGeometryDisplaySettings(SurfaceDisplaySettings& settings) {
    bool changed = false;

    changed |= ImGui::Checkbox("enabled", &settings.enabled);
    changed |= guiAuxEnumComboBox("polygon", settings.polygonMode);

    return changed;
}
inline bool guiGeometryDisplaySettings(LineDisplaySettings& settings) {
    bool changed = false;

    changed |= ImGui::Checkbox("enabled", &settings.enabled);

    return changed;
}

inline bool guiActiveProfileConfig(MembraneProfile& profile) {
    bool changed = false;

    changed |= guiGeometryDisplaySettings(profile.displaySettings.surface);

    changed |= guiAuxColorPicker3Popup("fixed color", profile.displaySettings.colorFixed.value.data());

    return changed;
}
inline bool guiActiveProfileConfig(FilamentProfile& profile) {
    bool changed = false;

    if(displayGeometryType(profile.displaySettings) == DisplayGeometryType::line) {
        changed |= guiGeometryDisplaySettings(profile.displaySettings.line);
    } else {
        changed |= guiGeometryDisplaySettings(profile.displaySettings.surface);
    }

    changed |= guiAuxColorPicker3Popup("fixed color", profile.displaySettings.colorFixed.value.data());

    return changed;
}
inline bool guiActiveProfileConfig(LinkerProfile& profile) {
    bool changed = false;

    if(displayGeometryType(profile.displaySettings) == DisplayGeometryType::line) {
        changed |= guiGeometryDisplaySettings(profile.displaySettings.line);
    } else {
        changed |= guiGeometryDisplaySettings(profile.displaySettings.surface);
    }

    // selector
    {
        // on/off checkbox
        bool filterOn = profile.selector.name.has_value();
        const bool filterOnChanged = ImGui::Checkbox(filterOn ? "##filter check" : "filter##filter check", &filterOn);
        changed |= filterOnChanged;

        if(filterOnChanged) {
            if(filterOn) profile.selector.name.emplace();
            else         profile.selector.name.reset();
        }

        // show text box
        if(filterOn) {
            ImGui::SameLine();
            changed |= ImGui::InputText("filter##filter text", &*profile.selector.name);
        }
    }

    // color
    changed |= guiAuxColorPicker3Popup("fixed color", profile.displaySettings.colorFixed.value.data());

    return changed;
}
inline bool guiActiveProfileConfig(AuxLineProfile& profile) {
    bool changed = false;
    changed |= guiGeometryDisplaySettings(profile.displaySettings.line);
    return changed;
}

// Returns whether the profile is changed
inline bool guiActiveProfileConfig(ElementProfile& profile) {
    bool changed = false;

    // Combo box to select target
    if(ImGui::BeginCombo("target", elementProfileDisplayName(profile.index()), 0)) {
        for (int i = 0; i < std::variant_size_v< ElementProfile >; ++i) {

            const bool isSelected = (profile.index() == i);
            if (ImGui::Selectable(elementProfileDisplayName(i), isSelected, 0)) {
                if(!isSelected) {
                    // A new profile is chosen
                    profile = makeProfileWithIndex(i);

                    // Because we selected a profile that was not previously selected
                    changed = true;
                }
            }

            // Set the initial focus when opening the combo (scrolling + keyboard navigation focus)
            if (isSelected) {
                ImGui::SetItemDefaultFocus();
            }
        }
        ImGui::EndCombo();
    }

    // Display actual settings
    changed |= std::visit([](auto& actualProfile) { return guiActiveProfileConfig(actualProfile); }, profile);

    return changed;
}

inline void guiProfileWindow(
    DisplaySettings& displaySettings,
    DisplayStates&   displayStates
) {
    if(!displaySettings.gui.profileWindow) return;

    ImGui::SetNextWindowSize(ImVec2(600, 300), ImGuiCond_FirstUseEver);

    if (ImGui::Begin("profile", &displaySettings.gui.profileWindow)) {

        if(displaySettings.displayMode == DisplayMode::realtime) {
            if(displayStates.realtimeDataStates.profileData.size() > 0) {
                if(guiActiveProfileConfig(displayStates.realtimeDataStates.profileData[0].profile)) {
                    displayStates.realtimeDataStates.profileData[0].shouldUpdateMeshData = true;
                }
            }
        }
    }

    // End of help window, no matter Begin returns true or false.
    ImGui::End();
}

inline void guiTrajectoryControlPanel(
    DisplaySettings& displaySettings,
    DisplayStates&   displayStates
) {
    // Playback
    ImGui::Checkbox("play", &displayStates.playback.isPlaying);
    ImGui::SliderFloat("speed", &displaySettings.playback.fps, 1, 20, "%.1f");
    ImGui::SliderInt("playback", &displayStates.playback.currentFrame, 0, displayStates.playback.maxFrame);
}

inline void guiViewSettings(ObjectViewSettings& viewSettings) {
    using PT = ObjectViewSettings::Projection::Type;

    guiAuxColorPicker4Popup("background", viewSettings.canvas.bgColor.data());
    guiAuxEnumComboBox("projection", viewSettings.projection.type);

    ImGui::SliderFloat("z near", &viewSettings.projection.zNear, 1.0, 200.0, "%.1f");
    ImGui::SliderFloat("z far", &viewSettings.projection.zFar, 1000.0, 20000.0, "%.1f");
    ImGui::SliderFloat("camera key speed", &viewSettings.control.cameraKeyPositionPerFrame, 50.0, 500.0, "%.1f");

    guiAuxEnumComboBox("mouse mode", viewSettings.control.cameraMouseMode);

}

inline void guiMainWindow(
    DisplaySettings& displaySettings,
    DisplayStates  & displayStates
) {
    const bool busy = displayStates.sync.busy.load();

    // Exceptionally add an extra assert here for people confused about initial Dear ImGui setup
    // Most ImGui functions would normally just crash if the context is missing.
    IM_ASSERT(ImGui::GetCurrentContext() != NULL && "Missing dear imgui context.");

    ImGuiWindowFlags windowFlags =
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoCollapse;

    // We specify a default position/size in case there's no data in the .ini file.
    // We only do it to make the demo applications a little more welcoming, but typically this isn't required.
    ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(400, 600), ImGuiCond_FirstUseEver);

    // Main body of the Demo window starts here.
    if (!ImGui::Begin("medyan control", nullptr, windowFlags))
    {
        // Early out if the window is collapsed, as an optimization.
        ImGui::End();
        return;
    }

    // Most "big" widgets share a common width settings by default. See 'Demo->Layout->Widgets Width' for details.

    // e.g. Use 2/3 of the space for widgets and 1/3 for labels (default)
    // ImGui::PushItemWidth(ImGui::GetWindowWidth() * 0.65f);

    // e.g. Leave a fixed amount of width for labels (by passing a negative value), the rest goes to widgets.
    // ImGui::PushItemWidth(ImGui::GetFontSize() * -12);

    // Menu Bar
    if (ImGui::BeginMenuBar())
    {
        if (ImGui::BeginMenu("file")) {
            ImGui::MenuItem("(empty menu)", NULL, false, false);
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("window")) {
            ImGui::MenuItem("profile", nullptr, &displaySettings.gui.profileWindow, true);
            ImGui::EndMenu();
        }

        if(ImGui::BeginMenu("help"))
        {
            ImGui::MenuItem("help window", nullptr, &displaySettings.gui.helpWindow, true);
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }

    // Main display settings
    guiAuxEnumComboBox(
        "display mode",
        displaySettings.displayMode,
        [&](DisplayMode modeOld, DisplayMode modeNew) {
            switch(modeNew) {
                case DisplayMode::trajectory:
                    if(modeOld != modeNew && displayStates.trajectoryDataStates.trajectories.empty()) {
                        pushAnAsyncTask(
                            displayStates.sync,
                            [&] {
                                backgroundTaskReadTrajectory(displayStates, DisplayTrajectoryFileSettings {});
                            }
                        );
                    }
                    break;
            }
        },
        busy ? ImGuiSelectableFlags_Disabled : 0
    );

    if(displaySettings.displayMode == DisplayMode::trajectory) {
        guiTrajectoryControlPanel(displaySettings, displayStates);
    }

    ImGui::Spacing();


    if (ImGui::CollapsingHeader("info")) {

        // Function to print view object information
        const auto printView = [](const ObjectViewSettings& viewSettings) {
            ImGui::Text("view");
            ImGui::BulletText(
                "window size: (%d, %d)",
                viewSettings.canvas.width,
                viewSettings.canvas.height
            );
            ImGui::BulletText(
                "camera position: (%.1f, %.1f, %.1f)",
                viewSettings.camera.position[0],
                viewSettings.camera.position[1],
                viewSettings.camera.position[2]
            );
            ImGui::BulletText(
                "camera target: (%.1f, %.1f, %.1f)",
                viewSettings.camera.target[0],
                viewSettings.camera.target[1],
                viewSettings.camera.target[2]
            );
            ImGui::BulletText("projection: %s", text(viewSettings.projection.type));
            if(viewSettings.projection.type == ObjectViewSettings::Projection::Type::perspective) {
                ImGui::BulletText("fov: %.2f", viewSettings.projection.fov);
            } else {
                ImGui::BulletText("scale: %.1f", viewSettings.projection.scale);
            }
            ImGui::BulletText(
                "z range: (%.1f, %.1f)",
                viewSettings.projection.zNear,
                viewSettings.projection.zFar
            );
        };

        // busy information
        if(busy) {
            ImGui::Text("busy...");
            ImGui::Separator();
        }
        // fps information
        ImGui::Text("fps: %.1f", displayStates.timing.fps);
        ImGui::Separator();
        // main view information
        printView(displaySettings.mainView);

    }

    if(ImGui::CollapsingHeader("settings", ImGuiTreeNodeFlags_DefaultOpen)) {

        // main view
        guiViewSettings(displaySettings.mainView);
    }


    // End of main window
    ImGui::End();
}

// Note:
//   - This should only be used in GLFW main loop
inline void imguiLoopRender(
    DisplaySettings& displaySettings,
    DisplayStates  & displayStates
) {
    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    if(displaySettings.gui.enabled) {
        guiMainWindow(displaySettings, displayStates);
        guiProfileWindow(displaySettings, displayStates);
        guiHelpWindow(displaySettings);
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

} // namespace medyan::visual

#endif
