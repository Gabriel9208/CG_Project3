#pragma once
#include "../Scene/MainScene.h"
#include "../Graphic/ShaderProgram/GraphicShader.h"
#include "../Graphic/VAO.h"
#include "../Graphic/VBO.h"
#include "../Graphic/EBO.h"

#include <GLFW/glfw3.h>
#include <cstdlib>


namespace CG {

	class GUI
	{
	private:
		MainScene* scene; // Point to the scene that GUI is currently binding with

		char outFileName[128];
		char inFileName[128];
		char animationName[128];



		void startFrame();
		void mainPanel();

		void exportPanel();
		void importPanel();

		void galleryPanel();
		void pickerPanel();
		void texturePanel();

		// gallery panel variables
		const char* styleList[1];
		int styleSelectedIdx;

		// texture panel variables
		char styleName[128] = "Default";
		const char* textureList[1];
		int textureSelectedIdx;

		void _render();

	public:
		GUI(GLFWwindow* window, MainScene* _scene);
		~GUI();

		void init(GLFWwindow* window, MainScene* _scene);
		void bindScene(MainScene* _scene);
		void render();
		void terminate();

		void exportFrame();
		void exportFromEditor();
		void exportFromAnimator();

		void renderEffectIcon(
			const glm::mat4& viewMatrix,
			const glm::mat4& projectionMatrix,
			unsigned int emitter = -1);
	};
}