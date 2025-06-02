#pragma once

#include <glm\glm.hpp>
#include <glm\gtc\matrix_transform.hpp>
#include <glm\gtc\matrix_inverse.hpp>
#include <glm\gtc\type_ptr.hpp>

#include <GL/glew.h>
#include <GLFW/glfw3.h>

#include "../Window/GUI.h"
#include "Scene/MainScene.h"
#include "../Scene/Camera.h"
#include "../Mesh/Patch.h"
#include "../Utilty/TextureMapper.h"
#include "../Window/ConvexWindow.h"

namespace CG
{
    class App
    {
    public:
        App();
        ~App();

        bool initialize();
        void loop();
        void terminate();

        void chooseFace(GLFWwindow* window);
        void resizeWindow(unsigned int w, unsigned int h);

        inline Patch* getPatch() { return patch; }
        inline MainScene* getMainScene() { return mainScene; }
        inline void setWidth(unsigned int w) { width = w; }
        inline void setHeight(unsigned int h) { height = h; }

    private:
        void render();
        void GLInit();

        GLFWwindow* mainWindow;
        ConvexWindow* convexWindow;
        MainScene* mainScene;
        GUI* gui;
        Patch* patch;

        double timeNow = 0;
        double timeLast = 0;
        double timeDelta = 0;

        unsigned int width, height;

        static int mode; // 0: showcase, 1: animation, 2: edit
    };
}