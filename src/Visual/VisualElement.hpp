#ifndef MEDYAN_Visual_VisualElement_Hpp
#define MEDYAN_Visual_VisualElement_Hpp

#include <cstdint>
#include <memory> // shared_ptr
#include <mutex>
#include <optional>
#include <variant>
#include <vector>

#include "utility.h" // Overloaded
#include "Visual/Common.hpp"
#include "Visual/MeshData.hpp"
#include "Visual/Shader.hpp"

namespace medyan::visual {

//-------------------------------------
// Element selectors and profiles
//-------------------------------------

struct MembraneSelector {
};
struct MembraneProfile {
    // selects all membranes
    MembraneSelector selector;

    // apply this setting
    MembraneDisplaySettings displaySettings;
};
inline auto select(
    const std::vector< MembraneFrame >& membranes,
    const MembraneSelector& selector
) {
    std::vector< const MembraneFrame* > res;
    for(auto& membrane : membranes) {
        res.push_back(&membrane);
    }
    return res;
}

struct FilamentSelector {
};
struct FilamentProfile {
    // selects all filaments
    FilamentSelector selector;

    // apply this setting
    FilamentDisplaySettings displaySettings;

};
inline auto select(
    const std::vector< FilamentFrame >& filaments,
    const FilamentSelector& selector
) {
    std::vector< const FilamentFrame* > res;
    for(auto& filament : filaments) {
        res.push_back(&filament);
    }
    return res;
}


struct LinkerSelector {
    std::optional< std::string > name;
};
struct LinkerProfile {
    // selector
    LinkerSelector selector;

    // setting
    LinkerDisplaySettings displaySettings;
};
inline auto select(
    const std::vector< LinkerFrame >& linkers,
    const LinkerSelector& selector,
    const DisplayTypeMap& typeMap
) {
    std::vector< const LinkerFrame* > res;
    for(auto& linker : linkers) {
        if(!selector.name.has_value() || *selector.name == typeMap.linkerTypeName.at(linker.type)) {
            res.push_back(&linker);
        }
    }
    return res;
}


using ElementProfile = std::variant<
    MembraneProfile,
    FilamentProfile,
    LinkerProfile
>;

struct MeshDataFrameInfo {
    int start = 0;
    int size = 0;
};
struct ProfileWithMeshData {
    ElementProfile                                   profile;
    MeshData                                         data;
    std::optional< std::vector< MeshDataFrameInfo >> frameInfo;
};


//-------------------------------------
// Functions for creating mesh data
//-------------------------------------



inline auto makeDefaultElementProfileData() {
    std::vector< ProfileWithMeshData > profiles;

    profiles.push_back(ProfileWithMeshData { MembraneProfile {} });

    profiles.push_back(ProfileWithMeshData { FilamentProfile {} });

    {
        LinkerProfile lp;
        lp.selector.name = "linker";
        profiles.push_back(ProfileWithMeshData { std::move(lp) } );
    }
    {
        LinkerProfile lp;
        lp.selector.name = "motor";
        lp.displaySettings.colorFixed = mathfunc::Vec3f { 0.1f, 0.1f, 0.99f };
        profiles.push_back(ProfileWithMeshData { std::move(lp) } );
    }

    return profiles;
}


inline auto displayGeometryType(const ElementProfile& profile) {
    return std::visit([](const auto& actualProfile) { return displayGeometryType(actualProfile.displaySettings); }, profile);
}

inline auto createMeshData(
    const DisplayFrame& frameData,
    const DisplayTypeMap& typeMap,
    const ElementProfile& profile
) {
    return std::visit(
        Overloaded {
            [&](const MembraneProfile& membraneProfile) {
                return createMembraneMeshData(
                    select(frameData.membranes, membraneProfile.selector),
                    membraneProfile.displaySettings
                );
            },
            [&](const FilamentProfile& filamentProfile) {
                return createFilamentMeshData(
                    select(frameData.filaments, filamentProfile.selector),
                    filamentProfile.displaySettings
                );
            },
            [&](const LinkerProfile& linkerProfile) {
                return createLinkerMeshData(
                    select(frameData.linkers, linkerProfile.selector, typeMap),
                    linkerProfile.displaySettings
                );
            }
        },
        profile
    );
}


// Returns
//   - The new mesh data created, containing all the frames
//   - The frame infomation
inline auto createMeshDataAllFrames(
    const ElementProfile& profile,
    const DisplayData&    displayData
) {
    MeshData meshData;
    std::vector< MeshDataFrameInfo > frameInfo;

    meshData.descriptor = displayGeometryType(profile) == DisplayGeometryType::surface ? 
        meshDataDescriptorSurface :
        meshDataDescriptorLine;

    const int numFrames = displayData.frames.size();
    frameInfo.reserve(numFrames);
    for(int i = 0; i < numFrames; ++i) {
        const int sizeCur = meshData.data.size();
        const int sizeInc = std::visit(
            Overloaded {
                [&](const MembraneProfile& prof) {
                    return appendMembraneMeshData(
                        meshData,
                        select(displayData.frames[i].membranes, prof.selector),
                        prof.displaySettings
                    );
                },
                [&](const FilamentProfile& prof) {
                    return appendFilamentMeshData(
                        meshData,
                        select(displayData.frames[i].filaments, prof.selector),
                        prof.displaySettings
                    );
                },
                [&](const LinkerProfile& prof) {
                    return appendLinkerMeshData(
                        meshData,
                        select(displayData.frames[i].linkers, prof.selector, displayData.displayTypeMap),
                        prof.displaySettings
                    );
                }
            },
            profile
        );

        frameInfo.push_back({ sizeCur, sizeInc });
    }

    return std::tuple { meshData, frameInfo };
}


//-------------------------------------
// Functions for drawing
//-------------------------------------

// Auxiliary function to replace buffer data in OpenGL
template< typename T >
inline void replaceBuffer(GLenum target, const std::vector<T>& source, int start, int size) {
    GLint prevSize;
    glGetBufferParameteriv(target, GL_BUFFER_SIZE, &prevSize);

    const auto newSize = sizeof(T) * size;

    if(newSize > prevSize) {
        glBufferData(target, newSize, source.data() + start, GL_DYNAMIC_DRAW);
    } else {
        glBufferSubData(target, 0, newSize, source.data() + start);
    }
}
template< typename T >
inline void replaceBuffer(GLenum target, const std::vector<T>& source) {
    replaceBuffer(target, source, 0, source.size());
}


// Draw elements using the mesh data.
//
// Notes:
//   - This function must be used in an OpenGL context
inline void draw(
    MeshData& meshData,
    std::optional< MeshDataFrameInfo > frameInfo,
    const GlVertexBufferManager& vertexBufferManager,
    GLenum polygonMode,
    GLenum elementMode
) {
    glBindVertexArray(vertexBufferManager.vao());

    glPolygonMode(GL_FRONT_AND_BACK, polygonMode);

    // Update data
    if(meshData.updated || frameInfo.has_value()) {
        glBindBuffer(GL_ARRAY_BUFFER, vertexBufferManager.vbo());
        if(frameInfo.has_value()) {
            replaceBuffer(GL_ARRAY_BUFFER, meshData.data, frameInfo->start, frameInfo->size);
        } else {
            replaceBuffer(GL_ARRAY_BUFFER, meshData.data);
        }
        meshData.updated = false;
    }

    // Draw
    const int numStrides = (frameInfo.has_value() ? frameInfo->size : meshData.data.size()) / meshData.descriptor.strideSize;
    glDrawArrays(elementMode, 0, numStrides);
    // glDrawElements(ve->state.eleMode, ve->state.vertexIndices.size(), GL_UNSIGNED_INT, (void*)0);

}
template< typename GeometryDisplaySettings >
inline void draw(
    MeshData&                          meshData,
    std::optional< MeshDataFrameInfo > frameInfo,
    const Shader&                      shader,
    const GeometryDisplaySettings&     settings
) {
    if(settings.enabled) {
        // setting uniform
        if constexpr(std::is_same_v< GeometryDisplaySettings, SurfaceDisplaySettings >) {
            shader.setVec3("material.specular", convertToGlm(settings.colorSpecular));
            shader.setFloat("material.shininess", settings.colorShininess);
        }

        draw(meshData, frameInfo, settings.vertexBufferManager, polygonModeGl(settings), elementModeGl(settings));
    }
}

inline void draw(MeshData& meshData, std::optional< MeshDataFrameInfo > frameInfo, const Shader& shader, const MembraneDisplaySettings& settings) {
    draw(meshData, frameInfo, shader, settings.surface);
}
inline void draw(MeshData& meshData, std::optional< MeshDataFrameInfo > frameInfo, const Shader& shader, const FilamentDisplaySettings& settings) {
    if(settings.pathMode == FilamentDisplaySettings::PathMode::line) {
        draw(meshData, frameInfo, shader, settings.line);
    } else {
        draw(meshData, frameInfo, shader, settings.surface);
    }
}
inline void draw(MeshData& meshData, std::optional< MeshDataFrameInfo > frameInfo, const Shader& shader, const LinkerDisplaySettings& settings) {
    if(settings.pathMode == LinkerDisplaySettings::PathMode::line) {
        draw(meshData, frameInfo, shader, settings.line);
    } else {
        draw(meshData, frameInfo, shader, settings.surface);
    }
}


struct Profile {

    using FlagType = std::uint_fast16_t;

    enum class PathMode { Line, Extrude, Bead };
    enum class GridMode { Boundary, Mesh };
    enum class ColorMode { Fixed };

    static constexpr FlagType targetFilament      = 1 << 0;
    static constexpr FlagType targetMembrane      = 1 << 1;
    static constexpr FlagType targetLinker        = 1 << 2;
    static constexpr FlagType targetMotor         = 1 << 3;
    static constexpr FlagType targetBrancher      = 1 << 4;
    static constexpr FlagType displayForce        = 1 << 5;
    static constexpr FlagType forceUseSearchDir   = 1 << 6;
    static constexpr FlagType targetCompartment   = 1 << 7;
    static constexpr FlagType targetConcentration = 1 << 8;

    // User settings
    //-------------------------------------------------------------------------
    bool enabled = false;

    // flag for choosing data
    FlagType flag = 0;

    // shape settings
    // -------- Pathable objects --------
    PathMode          pathMode = PathMode::Line; // TODO implement this in visual helper
    float             pathExtrudeRadius = 7.5f;
    std::uint_fast8_t pathExtrudeSides = 10;
    // -------- Bead-like objects --------
    float             beadRadius = 12.0f;
    std::uint_fast8_t beadLongitudeSegs = 10;
    std::uint_fast8_t beadLatitudeSegs = 5;
    // -------- Forces --------
    float             forceScale = 5.0f;
    // -------- Grid-like objects --------
    GridMode          gridMode = GridMode::Boundary;

    GLenum polygonMode = GL_LINE;

    // color settings
    glm::vec3 colorAmbient; // Currently not used
    glm::vec3 colorDiffuse;
    glm::vec3 colorSpecular  = { 0.7, 0.7, 0.7 };
    float     colorShininess = 32.0f;
};

struct GlSize {
    unsigned int vaStride      = 9;
    unsigned int vaPosStart    = 0;
    unsigned int vaPosSize     = 3;
    unsigned int vaNormalStart = 3;
    unsigned int vaNormalSize  = 3;
    unsigned int vaColorStart  = 6;
    unsigned int vaColorSize   = 3;
};

// Note:
//   - The object of this type must be created within an OpenGL context.
struct GlState {
    const GlSize size; // This is a redundant GlSize for thread safe access

    // vao, vbo, ebo
    GLuint vao;
    GLuint vbo;
    // GLuint ebo;

    // Element info
    std::vector< float >    vertexAttribs;
    // std::vector< unsigned > vertexIndices;
    bool attribChanged = true;
    // bool indexChanged  = true;

    // Set by init and helper
    GLenum eleMode = GL_TRIANGLES;

    // ctor and dtor
    GlState(GlSize newSize) : size(newSize) {
        glGenBuffers(1, &vbo);
        // glGenBuffers(1, &ebo);
        glGenVertexArrays(1, &vao);

        glBindVertexArray(vao);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);

        // Vertex attribute
        //---------------------------------------------------------------------
        GLuint idx = 0;
        // Position
        if(size.vaPosSize) {
            glVertexAttribPointer(idx, size.vaPosSize,    GL_FLOAT, GL_FALSE, size.vaStride * sizeof(float), static_cast<const char*>(0) + sizeof(float) * size.vaPosStart   );
            glEnableVertexAttribArray(idx);
            ++idx;
        }
        // Normal
        if(size.vaNormalSize) {
            glVertexAttribPointer(idx, size.vaNormalSize, GL_FLOAT, GL_FALSE, size.vaStride * sizeof(float), static_cast<const char*>(0) + sizeof(float) * size.vaNormalStart);
            glEnableVertexAttribArray(idx);
            ++idx;
        }
        // Color
        if(size.vaColorSize) {
            glVertexAttribPointer(idx, size.vaColorSize,  GL_FLOAT, GL_FALSE, size.vaStride * sizeof(float), static_cast<const char*>(0) + sizeof(float) * size.vaColorStart );
            glEnableVertexAttribArray(idx);
            ++idx;
        }

        // temporarily retarget
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glBindVertexArray(0);
    }

    ~GlState() {
        glDeleteVertexArrays(1, &vao);
        glDeleteBuffers(1, &vbo);
        // glDeleteBuffers(1, &ebo);
    }

};

// Each VisualElement consists of the following parts:
//   - A mutex that locks all the members
//   - All the related settings
//   - OpenGL compatible data structure as output
// Note:
//   - Due to constructor of the GlState element, the object must be created
//     within an OpenGL context.
struct VisualElement {
    std::mutex me;

    // settings
    //-------------------------------------------------------------------------
    Profile profile;

    // OpenGL data
    //-------------------------------------------------------------------------
    GlState state;

    VisualElement(GlSize size) : state(size) {}

};

// Preset configurations for shaders and vector structures
//
// Thread safety: Not thread safe. Creation or modification of objects of this
//     type should only happen in the opengl main thread.
// Note:
//   - The creation/modification/destruction of objects of this type must be
//     within an OpenGL context
struct VisualPreset {
    const GlSize size;
    const Shader shader;

    // Read or modification of visualElements vector requires lock.
    std::mutex veMutex;
    std::vector< std::shared_ptr< VisualElement > > visualElements;

    VisualPreset(GlSize size, const char* vertexShaderSrc, const char* fragmentShaderSrc)
        : size(size), shader(vertexShaderSrc, fragmentShaderSrc) { }

    ~VisualPreset() {
        {
            // Delete visual elements
            std::lock_guard< std::mutex > guard(veMutex);
            visualElements.clear();
        }
    } // ~VisualPreset()
}; // VisualPreset

} // namespace medyan::visual

#endif
