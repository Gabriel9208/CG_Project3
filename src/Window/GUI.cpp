#include "GUI.h"

#include "glm/glm.hpp"
#define GLM_ENABLE_EXPERIMENTAL

#include "glm/gtx/string_cast.hpp"
#include "../App/App.h"
#include "../Utilty/Error.h"
#include "../Utilty/FacePicker.h"

#include <glm/gtc/matrix_transform.hpp>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>
#include <fstream>
#include <sstream>

#define HEAD 4

namespace CG {

    GUI::GUI(GLFWwindow* window, MainScene* _scene)
    {
        if (window == nullptr)
        {
            return;
        }

        init(window, _scene);
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
                pickerPanel();
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Texture Factory"))
            {
                // load texture
                // modify texture on the faces (translation, rotation and scaling)
                ImGui::EndTabItem();
            }

            if (ImGui::BeginTabItem("Export Station"))
            {
                // export to file
                // add to gallery
                ImGui::EndTabItem();
            }            
            
            ImGui::EndTabBar();
        }
        
        
        
        ImGui::End();
    }

    void GUI::_render()
    {
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
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
}
