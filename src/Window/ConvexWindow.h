#pragma once
#include "../Graphic/ShaderProgram/GraphicShader.h"
#include "../Graphic/UBO.h"
#include "../Mesh/MyMesh.h"
#include "../Graphic/Material/ColorTexture.h"

namespace CG
{
	class ConvexWindow
	{
	public:
		ConvexWindow(MyMesh* mesh, int mainWindowDisplay_w, int mainWindowDisplay_h);
		~ConvexWindow() {}

		bool Initialize(int mainWindowDisplay_w, int mainWindowDisplay_h);
		void Resize(int mainWindowDisplay_w, int mainWindowDisplay_h);
		void Render(int display_w, int display_h);
		void updateGraph();

	private:
		GraphicShader drawGraphProgram;
		GraphicShader drawWindowProgram;
		VAO wVAO;
		VBO<float> wVBO;

		VAO fVAO;
		FBO fFBO;
		VBO<glm::vec2> fVBOp; // pos
		GLuint ProjID;
		ColorTexture texture;
		unsigned int bufferElementCount;

		MyMesh* referenceMesh;
		
		int display_w, display_h;

		glm::vec2 d2f(OpenMesh::Vec2d v);
		glm::vec2 f2f(OpenMesh::Vec2f v);
	};
}