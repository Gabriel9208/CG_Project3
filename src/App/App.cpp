#include <iostream>
#include <functional>

#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_opengl3.h>
#include <imgui_impl_glfw.h>

#include "App.h"
#include "../Texture/FacePicker.h"
#include "../Texture/TextureMapper.h"
#include "../Texture/TexturePainter.h"
#include "../Texture/Gallery.h"
#include "../Graphic/Material/TextureManager.h"

namespace CG
{
	double lastCursorX;
	double lastCursorY;

	bool mouseMiddlePressed = false;
	bool mouseRightPressed = false;
	bool mouseLeftPressed = false;


	static void keyPress(GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);

		ImGuiIO& io = ImGui::GetIO();
		if (!io.WantCaptureKeyboard) {
			App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
			Camera* camera = &(app->getMainScene()->getCamera());
			const float rotatSpeed = 2.0f;
			const float transSpeed = 0.5f;

			if (action == GLFW_REPEAT || action == GLFW_PRESS)
			{
				if (key == GLFW_KEY_W) // camera forward
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS) {
						camera->zoom(-transSpeed);
					}
					else
					{
						camera->rotateAround(-rotatSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
					}
				}
				if (key == GLFW_KEY_S) // camera backward
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS) {
						camera->zoom(transSpeed);
					}
					else
					{
						camera->rotateAround(rotatSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
					}
				}
				if (key == GLFW_KEY_D) // camera go right
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) != GLFW_PRESS) {
						camera->flatTranslate(transSpeed, 0);
					}
					else
					{
						camera->rotateAround(rotatSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
					}
				}
				if (key == GLFW_KEY_A) // camera go left
				{
					if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) 
					{	
						camera->rotateAround(-rotatSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
					}
					else if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS)
					{
						FacePicker& fp = FacePicker::getInstance();
						fp.chooseAllFace();

						app->getPatch()->init(app->getMainScene()->getMesh(), fp.getFacesPicked());

						TextureMapper& tm = TextureMapper::getInstance();
						tm.Map(app->getMainScene()->getMesh(), app->getPatch()->getOrderedBoundaryEdges(), &app->getPatch()->getVertices());

						app->getConvexWindow()->updateGraph();

						TexturePainter& tp = TexturePainter::getInstance();
						tp.update(std::string(app->getGUIChoosedTexture()), app->getMainScene()->getMesh());
					}
					else
					{
						camera->flatTranslate(-transSpeed, 0);

					}
				}
				if (key == GLFW_KEY_E) // camera go up
				{
					camera->flatTranslate(0, transSpeed);
				}
				if (key == GLFW_KEY_Q) // camera go down
				{
					camera->flatTranslate(0, -transSpeed);
				}
				if (key == GLFW_KEY_ENTER) 
				{
					TexturePainter& tp = TexturePainter::getInstance();
					tp.save();
				}
				if (key == GLFW_KEY_BACKSPACE)
				{
					TexturePainter& tp = TexturePainter::getInstance();
					tp.clearSaveTextureDatas();
				}
			}
		}
	}

	static void charCallback(GLFWwindow* window, unsigned int c)
	{
		ImGui_ImplGlfw_CharCallback(window, c);
	}

	static void mouseScroll(GLFWwindow* window, double xoffset, double yoffset)
	{
		const float transSpeed = 1.5f;

		App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
		Camera* camera = &(app->getMainScene()->getCamera());
		camera->zoom(-transSpeed * yoffset);

	}

	static void cursorEvent(GLFWwindow* window, double xpos, double ypos)
	{
		App* app = static_cast<App*>(glfwGetWindowUserPointer(window));

		if (mouseMiddlePressed || mouseRightPressed)
		{
			Camera* camera = &(app->getMainScene()->getCamera());

			double x = xpos - lastCursorX;
			double y = ypos - lastCursorY;

			float transSpeed = 0.0007f;
			float rotatSpeed = 0.25f;

			if (mouseRightPressed)
			{
				camera->rotateAround(-y * rotatSpeed, glm::vec3(1.0f, 0.0f, 0.0f));
				camera->rotateAround(-x * rotatSpeed, glm::vec3(0.0f, 1.0f, 0.0f));
			}

			if (mouseMiddlePressed)
			{
				camera->flatTranslate(-x * transSpeed, y * transSpeed);
			}

			lastCursorX = xpos;
			lastCursorY = ypos;
		}
		if (mouseLeftPressed)
		{
			app->chooseFace(window);
		}
	}

	App::App(): width(1280), height(720)
	{
		gui = nullptr;
		mainWindow = nullptr;
		mainScene = nullptr;
		convexWindow = nullptr;
	}

	App::~App()
	{

	}

	auto App::initialize() -> bool
	{
		// Set error callback
		glfwSetErrorCallback([](int error, const char* description)
			{ fprintf(stderr, "GLFW Error %d: %s\n", error, description); });

		// Initialize GLFW
		if (!glfwInit())
			return false;

		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_COMPAT_PROFILE);

		// Create window with graphics context
		unsigned int display_w = width, display_h = height;
		mainWindow = glfwCreateWindow(display_w, display_h, "Group6", nullptr, nullptr);
		if (mainWindow == nullptr)
			return false;
		glfwMakeContextCurrent(mainWindow);
		glfwSwapInterval(1); // Enable vsync

		// Initialize GLEW
		glewExperimental = GL_TRUE;
		GLenum glew_err = glewInit();
		if (glew_err != GLEW_OK)
		{
			throw std::runtime_error(std::string("Error initializing GLEW, error: ") + (const char*)glewGetErrorString(glew_err));
			return false;
		}

		GLInit();
		TextureManager& tmg = TextureManager::getInstance();
		tmg.init();
		tmg.registerTexture("../../res/texture/test.jpg", "Sky");
		tmg.registerTexture("../../res/texture/test2.jpg", "B");
		tmg.registerTexture("../../res/texture/wool.jpg", "Wool");
		tmg.registerTexture("../../res/texture/wool2.jpg", "Wool2");
		tmg.registerTexture("../../res/texture/elf.png", "Elf");
		tmg.registerTexture("../../res/texture/Red.jpg", "Red");
		tmg.registerTexture("../../res/texture/Blue.jpg", "Blue");
		tmg.registerTexture("../../res/texture/Green.jpg", "Green");
		tmg.registerTexture("../../res/texture/Complexion.jpg", "Complexion");

		TexturePainter& tp = TexturePainter::getInstance();

		mainScene = new MainScene();
		mainScene->Initialize(display_w, display_h);
		tp.init(display_w, display_h, mainScene->getMesh());

		patch = new Patch(mainScene->getMesh());
		convexWindow = new ConvexWindow(mainScene->getMesh(), display_w, display_h);

		FacePicker& fp = FacePicker::getInstance();
		fp.registerToMesh(mainScene->getMesh());

		gui = new GUI(mainWindow, mainScene);

		glfwSetWindowUserPointer(mainWindow, this);
		glfwSetFramebufferSizeCallback(mainWindow,
			[](GLFWwindow* window, int w, int h) {
				auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
				auto mainScene = app->getMainScene();
				
				app->resizeWindow(w, h);
			}
		);

		glfwSetMouseButtonCallback(mainWindow,
			[](GLFWwindow* window, int button, int action, int mods)
			{
				double xpos, ypos;
				glfwGetCursorPos(window, &xpos, &ypos);
				auto app = static_cast<App*>(glfwGetWindowUserPointer(window));
				auto mainScene = app->getMainScene();

				ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
				ImGuiIO& io = ImGui::GetIO();

				if (!io.WantCaptureMouse) {				
					if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
					{
						FacePicker& fp = FacePicker::getInstance();

						if (!(mods & GLFW_MOD_SHIFT) && mouseLeftPressed == false)
						{
							fp.clearPickedFaces();
						}

						app->patch->clear();
						app->chooseFace(window);
						mouseLeftPressed = true;

					}
					if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
					{
						FacePicker& fp = FacePicker::getInstance();
						mouseLeftPressed = false;
						app->patch->init(mainScene->getMesh(), fp.getFacesPicked());

						TextureMapper& tm = TextureMapper::getInstance();
						tm.Map(mainScene->getMesh(), app->patch->getOrderedBoundaryEdges(), &app->patch->getVertices());

						app->convexWindow->updateGraph();

						TexturePainter& tp = TexturePainter::getInstance();
						tp.update(std::string(app->getGUIChoosedTexture()), mainScene->getMesh());
					}
					if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS)
					{
						glfwGetCursorPos(window, &lastCursorX, &lastCursorY);
						mouseMiddlePressed = true;
					}
					if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE)
					{
						mouseMiddlePressed = false;
					}
					if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
					{
						glfwGetCursorPos(window, &lastCursorX, &lastCursorY);
						mouseRightPressed = true;
					}
					if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE)
					{
						mouseRightPressed = false;
					}
				}
			});

		glfwSetScrollCallback(mainWindow,
			[](GLFWwindow* window, double xoffset, double yoffset)
			{
				const float transSpeed = 0.025f;

				App* app = static_cast<App*>(glfwGetWindowUserPointer(window));
				Camera* camera = &(app->getMainScene()->getCamera());
				camera->zoom(-transSpeed * yoffset);
			});

		glfwSetKeyCallback(mainWindow, keyPress);

		glfwSetCursorPosCallback(mainWindow, cursorEvent);

		glfwSetCharCallback(mainWindow, charCallback);


		
		return true;
	}

	void App::loop()
	{
		while (!glfwWindowShouldClose(mainWindow))
		{
			glfwPollEvents();

			timeNow = glfwGetTime();
			timeDelta = timeNow - timeLast;
			timeLast = timeNow;

			render();
		
			gui->render();
			
			ImGuiIO& io = ImGui::GetIO();
			(void)io;
			if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
			{
				GLFWwindow* backup_current_context = glfwGetCurrentContext();
				ImGui::UpdatePlatformWindows();
				ImGui::RenderPlatformWindowsDefault();
				glfwMakeContextCurrent(backup_current_context);
			}
			
			glfwSwapBuffers(mainWindow);
		}
	}

	void App::terminate()
	{
		gui->terminate();

		if (mainScene != nullptr) {
			delete mainScene;
			mainScene = nullptr;
		}

		glfwDestroyWindow(mainWindow);
		glfwTerminate();

	}

	void App::render()
	{
		int display_w, display_h;
		glfwGetFramebufferSize(mainWindow, &display_w, &display_h);
		glViewport(0, 0, display_w, display_h);

		mainScene->Render(timeNow, timeDelta, display_w, display_h, gui->getMode());
		convexWindow->Render(display_w, display_h);
	}

	void App::chooseFace(GLFWwindow* window)
	{
		FacePicker& fp = FacePicker::getInstance();

		double x, y;
		glfwGetCursorPos(window, &x, &y);
		if (x < 0 || y < 0)
		{
			return;
		}
		fp.chooseFace(
			height,
			x,
			y,
			*mainScene->getCamera().GetViewMatrix(),
			*mainScene->getCamera().GetProjectionMatrix(),
			mainScene->getFaceIDTextureID()
		);
	}

	void App::resizeWindow(unsigned int w, unsigned int h)
	{
		width = w;
		height = h;
		mainScene->getMesh()->resizeTextureRBO(width, height);
		convexWindow->Resize(width, height);
	}

	void App::GLInit()
	{
		glEnable(GL_DEPTH_TEST);
		glCullFace(GL_BACK);
		glEnable(GL_CULL_FACE);
		glClearColor(0.0, 0.0, 0.0, 1); //black screen
	}
}