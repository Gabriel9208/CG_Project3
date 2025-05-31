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

        inline MainScene* getMainScene() { return mainScene; }

    private:
        void render();
        void GLInit();

        GLFWwindow* mainWindow;
        MainScene* mainScene;
        GUI* gui;

        double timeNow = 0;
        double timeLast = 0;
        double timeDelta = 0;

        static int mode; // 0: showcase, 1: animation, 2: edit
    };
}