#include "MainScene.h"
#include "../Utilty/Error.h"

#include <glm/gtc/type_ptr.hpp>

namespace CG
{
	MainScene::MainScene():
		camera(),
		program()
	{
		mesh = nullptr;
		patch = nullptr;
	}

	MainScene::~MainScene()
	{
	}

	auto MainScene::Initialize(int display_w, int display_h) -> bool
	{
		xpos = 0;
		ypos = 0;
		camera.LookAt(glm::vec3(0, 20, 40), glm::vec3(0, -20, 0), glm::vec3(0, 1, 0));

		return loadScene(display_w, display_h);
	}

	void MainScene::Render(double timeNow, double timeDelta, int display_w, int display_h)
	{
		GLCall(glClearColor(0.0, 0.0, 0.0, 1.0));
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		glm::vec3 cameraPos = camera.getPos();
		glm::vec3 cameraTarget = camera.getTarget();
		glm::vec3 up = glm::vec3(0, 1, 0);

		/*
		matVPUbo.fillInData(0, sizeof(glm::mat4), camera.GetViewMatrix());
		matVPUbo.fillInData(sizeof(glm::mat4), sizeof(glm::mat4), camera.GetProjectionMatrix());
				*/
		glBindFramebuffer(GL_FRAMEBUFFER, 0);

		GLCall(glFlush());

		mesh->Render(*camera.GetProjectionMatrix(), *camera.GetViewMatrix());
	}
	
	auto MainScene::loadScene(int display_w, int display_h) -> bool
	{
		camera.LookAt(
			glm::vec3(0, 0, 1),   // Camera position in World Space
			glm::vec3(0, 0, 0),   // and looks at the origin
			glm::vec3(0, 1, 0)    // Head is up (set to 0,1,0 to look upside-down)
		);
		
		GLCall(glEnable(GL_DEPTH_TEST));
		GLCall(glEnable(GL_BLEND));
		GLCall(glEnable(GL_MULTISAMPLE));
		GLCall(glDepthMask(GL_TRUE));
		GLCall(glBlendEquationSeparate(GL_FUNC_ADD, GL_FUNC_ADD));
		GLCall(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO));


		mesh = new MyMesh();
		mesh->LoadFromFile("../../res/models/Armadillo.obj");

		return true;
	}
}