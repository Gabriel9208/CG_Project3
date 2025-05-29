#include "MainScene.h"
#include "../Utilty/Error.h"

namespace CG
{
	MainScene::MainScene()
	{
		camera = nullptr;
		mesh = nullptr;
	}

	MainScene::~MainScene()
	{
	}

	auto MainScene::Initialize() -> bool
	{
		xpos = 0;
		ypos = 0;
		isClicked = false;
		return LoadScene();
	}

	void MainScene::Update(double dt)
	{

	}

	void MainScene::Render()
	{
		GLCall(glClearColor(0.0, 0.0, 0.0, 1)); //black screen
		GLCall(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

		mesh->Render(camera->GetProjectionMatrix(), camera->GetViewMatrix());
	}

	void MainScene::OnResize(int width, int height)
	{
		std::cout << "MainScene Resize: " << width << " " << height << std::endl;
	}

	void MainScene::OnKeyboard(int key, int action)
	{
		std::cout << "MainScene OnKeyboard: " << key << " " << action << std::endl;		
	}

	void MainScene::OnClick(int button, int action, double _xpos, double _ypos)
	{
		if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
		{
			xpos = _xpos;
			ypos = _ypos;

			float depthVal = 0;
			double windowX = xpos;
			double windowY = 720 - ypos;
			GLCall(glReadPixels(windowX, windowY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthVal));
			std::cout << xpos << " " << windowY << "\n";

			GLint _viewPort[4];
			GLCall(glGetIntegerv(GL_VIEWPORT, _viewPort));
			glm::vec4 viewport(_viewPort[0], _viewPort[1], _viewPort[2], _viewPort[3]);
			glm::vec3 windowPos(windowX, windowY, depthVal);
			glm::vec3 wp = glm::unProject(windowPos, camera->GetViewMatrix(), camera->GetProjectionMatrix(), viewport);
			std::cout << wp[0] << " " << wp[1] << " " << wp[2] << "\n";

			unsigned int idx;
			GLCall(glGetTextureSubImage(mesh->fboColor, 0, static_cast<GLint>(windowX), static_cast<GLint>(windowY), 0, 1, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, sizeof(unsigned int), &idx));
			//idx -= 1;
			GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));

			std::cout << "Picked Face ID: " << idx << std::endl;

		}
	}

	
	auto MainScene::LoadScene() -> bool
	{
		camera = new Camera();
		// Camera matrix
		camera->LookAt(
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