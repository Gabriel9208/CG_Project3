#include "ConvexWindow.h"
#include "../Utilty/TextureMapper.h"
#include "../Utilty/Error.h"
#include <glm/gtc/matrix_transform.hpp>


namespace CG
{
	ConvexWindow::ConvexWindow(MyMesh* mesh, int mainWindowDisplay_w, int mainWindowDisplay_h): 
		referenceMesh(mesh), 
		fVAO(), 
		fFBO(), 
		fVBOp(),
		wVBO(),
		bufferElementCount(0)
	{
		Initialize(mainWindowDisplay_w, mainWindowDisplay_h);
	}

	bool ConvexWindow::Initialize(int mainWindowDisplay_w, int mainWindowDisplay_h)
	{
		display_w = mainWindowDisplay_w * 0.2;
		display_h = mainWindowDisplay_h * 0.2;

		ShaderInfo shadersWindow[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/drawWindow.vp" },
			{ GL_FRAGMENT_SHADER, "../../res/shaders/drawWindow.fp" },
			{ GL_NONE, NULL } };
		drawWindowProgram.load(shadersWindow);
		drawWindowProgram.use();

		ShaderInfo shadersLine[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/line2D.vp" },
			{ GL_FRAGMENT_SHADER, "../../res/shaders/line2D.fp" },
			{ GL_NONE, NULL } };
		drawGraphProgram.load(shadersLine);
		drawGraphProgram.use();
		ProjID = glGetUniformLocation(drawGraphProgram.getId(), "Projection");

		fVAO.bind();
		fFBO.bind();
		texture.setup(display_w, display_h);
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.getId(), 0));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		fVBOp.initialize(0, GL_STATIC_DRAW);
		GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));

		fVAO.unbind();
		fFBO.unbind();
		fVBOp.unbind();

		wVAO.bind();

		// place the window to the upper right corner
		float left = 0.6f;
		float right = 1.0f;
		float bottom = 0.6f;
		float top = 1.0f;

		std::vector<float> quadVertices = {
			left,  bottom,  0.0f, 0.0f,
			right, bottom,  1.0f, 0.0f,
			left,  top,     0.0f, 1.0f,
			right, bottom,  1.0f, 0.0f,
			right, top,     1.0f, 1.0f,
			left,  top,     0.0f, 1.0f
		};

		wVBO.initialize(quadVertices ,GL_STATIC_DRAW);
		GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), 0));
		GLCall(glEnableVertexAttribArray(0)); 
		GLCall(glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float))));
		GLCall(glEnableVertexAttribArray(1));

		wVAO.unbind();
		wVBO.unbind();

		return false;
	}

	void ConvexWindow::Resize(int mainWindowDisplay_w, int mainWindowDisplay_h)
	{
		display_w = mainWindowDisplay_w * 0.2;
		display_h = mainWindowDisplay_h * 0.2;
		texture.resize(display_w, display_h);
	}

	void ConvexWindow::Render(int _display_w, int _display_h)
	{
		if ((_display_w * 0.2 != display_w) || (_display_h * 0.2 != display_h))
		{
			Resize(_display_w, _display_h);
		}

		drawGraphProgram.use();
		fVAO.bind();
		fFBO.bind();

		GLCall(glViewport(0, 0, display_w, display_h));

		GLCall(glClearColor(0, 0, 0, 1));
		GLCall(glDisable(GL_DEPTH_TEST));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));		

		glm::mat4 proj = glm::ortho(0, 1, 0, 1, 1, -1);
		GLCall(glUniformMatrix4fv(ProjID, 1, GL_FALSE, &proj[0][0]));

		GLCall(glDrawArrays(GL_LINES, 0, bufferElementCount));
		fFBO.unbind();

		GLCall(glViewport(0, 0, _display_w, _display_h));

		drawWindowProgram.use();
		wVAO.bind();
		texture.bind(0);
		GLCall(glDrawArrays(GL_TRIANGLES, 0, 6));

		wVAO.unbind();

		GLCall(glEnable(GL_DEPTH_TEST));
	}

	void ConvexWindow::updateGraph()
	{
		TextureMapper& tm = TextureMapper::getInstance();
		std::map<OpenMesh::VertexHandle, OpenMesh::Vec2d> vhMapVec2d = tm.getAllUVMap();

		std::vector<glm::vec2> points;
		std::set<OpenMesh::Vec2d> used;
		for (auto itr = vhMapVec2d.begin(); itr != vhMapVec2d.end(); itr++)
		{
			for (MyMesh::VertexVertexIter vv_itr = referenceMesh->vv_iter(itr->first);
				vv_itr.is_valid();
				vv_itr++)
			{
				if (vhMapVec2d.find(*vv_itr) != vhMapVec2d.end() && used.find(vhMapVec2d[*vv_itr]) == used.end())
				{
					points.emplace_back(d2f(vhMapVec2d[itr->first]));
					points.emplace_back(d2f(vhMapVec2d[*vv_itr]));
				}
			}
			used.insert(itr->second);
		}

		/* debug
		for (int i = 0; i < points.size(); i++)
		{
			std::cout << points[i][0] << " " << points[i][1] << "\n";
		}
		*/

		bufferElementCount = points.size(); // for render use
		fVBOp.setData(points, GL_STATIC_DRAW);
	}


	glm::vec2 ConvexWindow::d2f(OpenMesh::Vec2d v)
	{
		return glm::vec2(v[0], v[1]);
	}

	glm::vec2 ConvexWindow::f2f(OpenMesh::Vec2f v)
	{
		return glm::vec2(v[0], v[1]);
	}

}