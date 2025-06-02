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
		bufferElementCount(0)
	{
		Initialize(mainWindowDisplay_w, mainWindowDisplay_h);
	}

	bool ConvexWindow::Initialize(int mainWindowDisplay_w, int mainWindowDisplay_h)
	{
		display_w = mainWindowDisplay_w * 0.2;
		display_h = mainWindowDisplay_h * 0.2;

		ShaderInfo shadersLine[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/line2D.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/line2D.fp" },//fragment shader
			{ GL_NONE, NULL } };
		program.load(shadersLine);
		program.use();
		ProjID = glGetUniformLocation(program.getId(), "Projection");

		fVAO.bind();
		fFBO.bind();
		texture.setup(display_w, display_h);
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, texture.getId(), 0));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

		fVBOp.initialize(0, GL_STATIC_DRAW);
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));

		fVAO.unbind();

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
		if ((_display_w * 0.2 != display_w) || (_display_h * 0.2 != display_w))
		{
			Resize(_display_w, _display_h);
		}

		program.use();
		fVAO.bind();
		fFBO.bind();

		GLCall(glDisable(GL_DEPTH_TEST));
		GLCall(glClear(GL_COLOR_BUFFER_BIT));		

		glm::mat4 proj = glm::ortho(0, 1, 0, 1, 1, -1);
		GLCall(glUniformMatrix4fv(ProjID, 1, GL_FALSE, &proj[0][0]));

		GLCall(glDrawArrays(GL_LINES, 0, bufferElementCount));
		fFBO.unbind();


		GLCall(glEnable(GL_DEPTH_TEST));
	}

	void ConvexWindow::updateBuffer()
	{
		TextureMapper& tm = TextureMapper::getInstance();
		std::map<OpenMesh::VertexHandle, OpenMesh::Vec2d> vhMapVec2d = tm.getResult();

		std::vector<glm::vec2> points;
		for (auto itr = vhMapVec2d.begin(); itr != vhMapVec2d.end(); itr++)
		{
			for (MyMesh::VertexVertexIter vv_itr = referenceMesh->vv_iter(itr->first);
				vv_itr.is_valid();
				vv_itr++)
			{
				if (vhMapVec2d.find(*vv_itr) != vhMapVec2d.end())
				{
					points.emplace_back(d2f(vhMapVec2d[itr->first]));
					points.emplace_back(d2f(vhMapVec2d[*vv_itr]));
				}
			}
		}
		bufferElementCount = points.size();
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