#pragma once
#include "Camera.h"
#include "../Graphic/ShaderProgram/GraphicShader.h"
#include "../Graphic/UBO.h"
#include <array>
#include <string>
#include <map>
#include <vector>

#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <Mesh/MyMesh.h>

constexpr auto PARTSNUM = 18;

namespace CG
{
	class MainScene
	{
	public:
		MainScene();
		~MainScene();

		bool Initialize(int display_w, int display_h);
		void Render(double timeNow, double timeDelta, int display_w, int display_h);
		void Resize(int display_w, int display_h);
		void SetObjectsVisibility(std::vector<bool> isDisplays);
		void Update(double dt);

		void OnClick(int button, int action, double _xpos, double _ypos);
		void chooseFace(double _xpos, double _ypos);
		inline Camera& getCamera() { return camera; }


	private:
		bool loadScene(int display_w, int display_h);
		void loadModel();
		void loadAnimation();

		GraphicShader program;
		UBO matVPUbo;

		Camera camera;
		MyMesh* mesh;

		double xpos;
		double ypos;

		bool isClicked;
	};
}

