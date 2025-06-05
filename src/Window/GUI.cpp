#include "GUI.h"

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtx/string_cast.hpp"
#include "../App/App.h"
#include "../Utilty/Error.h"
#include "../Texture/FacePicker.h"
#include "../Texture/TexturePainter.h"
#include "../Texture/Gallery.h"
#include "../Graphic/Material/TextureManager.h"

#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <fstream>
#include <sstream>

#define HEAD 4

namespace CG {

    std::vector<OpenMesh::Vec2d> glmToOpenMeshVec2(std::vector<glm::vec2> _v)
    {
        std::vector<OpenMesh::Vec2d> v;
        for (auto itr = _v.begin(); itr != _v.end(); itr++)
        {
            v.emplace_back(OpenMesh::Vec2d(itr->x, itr->y));
        }
        return v;
    }

    GUI::GUI(GLFWwindow* window, MainScene* _scene)
    {
        if (window == nullptr)
        {
            return;
        }

        init(window, _scene);
        textureSelectedIdx = 0;

        styleSelectedIdx = 0;
    }

    GUI::~GUI()
    {}

    void GUI::init(GLFWwindow* window, MainScene* _scene)
    {
        const char* glsl_version = "#version 460";

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO(); (void)io;
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

        ImGui::StyleColorsDark();

        ImGui_ImplGlfw_InitForOpenGL(window, false); // Disable this because we have manually handle the glfwSetMouseButtonCallback
        ImGui_ImplOpenGL3_Init(glsl_version);

        bindScene(_scene);
    }

    void GUI::bindScene(MainScene* _scene)
    {
        if (_scene != nullptr)
        {
            scene = _scene;
        }
    }

    void GUI::render()
    {
        startFrame();
        mainPanel();
        _render();
    }

    void GUI::terminate()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
    }

    void GUI::startFrame()
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void GUI::mainPanel()
    {
        ImGui::SetNextWindowPos(ImVec2(0, 0), ImGuiCond_Always);
        ImGui::SetNextWindowSize(ImVec2(ImGui::GetIO().DisplaySize.x * 0.3, ImGui::GetIO().DisplaySize.y), ImGuiCond_Always);
        ImGui::Begin("Configurations", nullptr, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoResize);

        
        if (ImGui::BeginTabBar("Mode"))
        {
            if (ImGui::BeginTabItem("Gallery"))
            { // sets of textured model
                mode = 0;
                galleryPanel();

                importPanel();
                exportPanel();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Face Picker"))
            {
                // picking face enable
                // picjing face related functionalities

                /*
                if (ImGui::CollapsingHeader("Objects Visibility", ImGuiTreeNodeFlags_DefaultOpen))
                {
                    pickerPanel();
                }
                */

                mode = 1;
                pickerPanel();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Texture Edit"))
            {
                // load texture
                // modify texture on the faces (translation, rotation and scaling)

                mode = 2;
                texturePanel();
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        ImGui::End();
    }

    void GUI::exportPanel()
    {
        ImGui::SeparatorText("Export style");
        ImGui::Text("This will overwrite the whole file.");

        ImGui::InputTextWithHint(" ", "ex. export.style", outFileName, IM_ARRAYSIZE(outFileName));
        ImGui::Text("");

        if (ImGui::Button("Export")) {
            Gallery& glry = Gallery::getInstance();
            std::vector<std::string> sList = glry.getStyleList();
            glry.exportToFile(sList[styleSelectedIdx], outFileName);
        }
        
    }

    void GUI::importPanel()
    {
        ImGui::SeparatorText("Import style");

        ImGui::InputTextWithHint("  ", "ex. import.style", inFileName, IM_ARRAYSIZE(inFileName));
        ImGui::Text("");

        if (ImGui::Button("Import")) {
            Gallery& glry = Gallery::getInstance();
            glry.importFromFile(inFileName);
        }
    }

    void GUI::_render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
    }

    void GUI::galleryPanel()
    {
        ImGui::SeparatorText("Choose style to apply.");

        Gallery& glry = Gallery::getInstance();
        std::vector<std::string> sList = glry.getStyleList();

        std::vector<const char*> styleList;

        for (const auto& str : sList) {
            styleList.push_back(str.c_str());
        }

        ImGui::PushItemWidth(-1);  // Make ListBox fill available width

        if (ImGui::ListBox("##StyleList",
            &styleSelectedIdx,
            styleList.data(),
            (int)(styleList.size()),
            (int)(styleList.size())))
        {
            if (styleList.size() > styleSelectedIdx)
            {
                glry.renderStyle(std::string(styleList[styleSelectedIdx]));
            }
        }
        ImGui::PopItemWidth();
        
    }

    void GUI::pickerPanel()
    {
        ImGui::SeparatorText("Usage");
        ImGui::Text("Set the range of pick.\n");

        FacePicker& fp = FacePicker::getInstance();
        int range = fp.getRange();
        if (ImGui::DragInt("    ", &range, 0.5f, 0, 500, "%d"))
        {
            fp.setRange(range);
        }
    }

    void GUI::texturePanel()
    {
        ImGui::SeparatorText("Choose texture to apply.");
        
        TextureManager& tmg = TextureManager::getInstance();
        std::vector<const char*> textureList;
        std::vector<std::string> tList = tmg.getTextureList();

        for (const auto& str : tList) {
            textureList.push_back(str.c_str());
        }

        ImGui::PushItemWidth(-1);  // Make ListBox fill available width

        if (ImGui::ListBox("##TextureList",
            &textureSelectedIdx,
            textureList.data(),
            (int)(textureList.size()),
            (int)(textureList.size())))
        {
            if (textureList.size() > textureSelectedIdx)
            {
                currentTexture = std::string(textureList[textureSelectedIdx]);
            }
        }
        ImGui::PopItemWidth();
        
        ImGui::SeparatorText("Transform");

        FacePicker& fp = FacePicker::getInstance();
        TextureMapper& tm = TextureMapper::getInstance();
        TexturePainter& tp = TexturePainter::getInstance();
        Gallery& glry = Gallery::getInstance();

        glm::vec2 translateOffset = tm.getTranslateOffset();
        float rotateDegree = tm.getRotateDegree();
        float scalingDegree = tm.getScalingDegree();

        const glm::vec2 trans = translateOffset;
        const double rotate = rotateDegree;
        const double scaling = scalingDegree;
        
        ImGui::SeparatorText("Translation");
        if (ImGui::DragFloat("x (Translate)", &translateOffset[0], 0.05f, -FLT_MAX, FLT_MAX, "%.3f"))
        {
            double x = translateOffset[0] - trans[0];
            double y = translateOffset[1] - trans[1];
            tm.translate(x, y);
            tp.updateUV();
        }
        if (ImGui::DragFloat("y (Translate)", &translateOffset[1], 0.05f, -FLT_MAX, FLT_MAX, "%.3f"))
        {
            double x = translateOffset[0] - trans[0];
            double y = translateOffset[1] - trans[1];
            tm.translate(x, y);
            tp.updateUV();

        }

        ImGui::SeparatorText("Rotation");
        if (ImGui::DragFloat("Rotation Degree", &rotateDegree, 0.05f, -360, 360, "%.3f"))
        {
            tm.rotate(rotateDegree - rotate);
            tp.updateUV();

        }

        ImGui::SeparatorText("Scale");
        if (ImGui::DragFloat("Scaling Degree", &scalingDegree, 0.05f, -100, 100, "%.3f"))
        {
            tm.scaling(scalingDegree - scaling);
            tp.updateUV();
        }
        ImGui::TextWrapped("");
        ImGui::InputText("Style Name", styleName, IM_ARRAYSIZE(styleName));
        if (ImGui::Button("Save to temp gallery."))
        {
            if (!glry.findStyle(std::string(styleName)))
            {
                glry.registerStyle(std::string(styleName));
            }

            /*Appearance app;
            UVSet uv;

            uv.UVs = glmToOpenMeshVec2(tp.getUVCoords());
            uv.heIDs = tp.getHeIdx();

            app.textureName = std::string(textureList[textureSelectedIdx]);
            app.faceIDs.emplace_back(fp.getFacesPicked());
            app.UVSets.emplace_back(uv);

            glry.addAppearance(styleName, app);*/
            glry.updataSaveTextureDatas(styleName, tp.getSaveTextureDatas());
        }

    }
}
