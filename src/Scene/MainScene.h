#pragma once
#include "Camera.h"
#include "../Graphic/ShaderProgram/GraphicShader.h"
#include "../Graphic/UBO.h"
#include "../Mesh/Patch.h"

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

		inline Camera& getCamera() { return camera; }
		inline GLuint getFaceIDTextureID() const { return mesh->getFboColor(); }
		inline MyMesh* getMesh() { return mesh; }
	private:
		bool loadScene(int display_w, int display_h);
		
		GraphicShader program;
		UBO matVPUbo;

		Camera camera;
		MyMesh* mesh;
		Patch* patch;

		double xpos;
		double ypos;
	};
}

