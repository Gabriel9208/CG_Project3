#include "FacePicker.h"
#include "Error.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace CG
{
	FacePicker::FacePicker() : defaultLineColor(1, 1, 1), selectedLineColor(1, 0, 0), canHighlightEdge(false), mesh(nullptr)
	{}

	FacePicker FacePicker::instance;

	FacePicker& FacePicker::getInstance()
	{
		return instance; 
	}

	void FacePicker::registerToMesh(MyMesh* target)
	{
		mesh = target;
	}

	void FacePicker::chooseFace(double _xpos, double _ypos, glm::mat4 view, glm::mat4 proj, GLuint textureID)
	{
		float depthVal = 0;
		double windowX = _xpos;
		double windowY = 720 - _ypos;
		GLCall(glReadPixels(windowX, windowY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthVal));

		GLint _viewPort[4];
		GLCall(glGetIntegerv(GL_VIEWPORT, _viewPort));
		glm::vec4 viewport(_viewPort[0], _viewPort[1], _viewPort[2], _viewPort[3]);
		glm::vec3 windowPos(windowX, windowY, depthVal);
		glm::vec3 wp = glm::unProject(windowPos, view, proj, viewport);

		unsigned int idx;
		GLCall(glGetTextureSubImage(textureID, 0, static_cast<GLint>(windowX), static_cast<GLint>(windowY), 0, 1, 1, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, sizeof(unsigned int), &idx));
		GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));

		if (idx == 0)
		{
			return;
		}

		if (pickedFaces.find(idx - 1) == pickedFaces.end())
		{
			pickedFaces.insert(idx - 1);

			for (auto faceId = pickedFaces.begin(); faceId != pickedFaces.end(); faceId++)
			{
				OpenMesh::FaceHandle fh = mesh->face_handle(*faceId);
				for (MyMesh::FaceEdgeIter fe_itr = mesh->fe_iter(fh); fe_itr.is_valid(); fe_itr++)
				{
					OpenMesh::EdgeHandle eh = *fe_itr;
					mesh->set_color(eh, selectedLineColor);
					std::vector<glm::vec3> v = { f2f(selectedLineColor), f2f(selectedLineColor) };
					mesh->setWVBOcSubData(eh.idx() * 2, 2, &v);
				}
			}
		}
	}


	glm::vec3 FacePicker::d2f(OpenMesh::Vec3d v)
	{
		return glm::vec3(v[0], v[1], v[2]);
	}

	glm::vec3 FacePicker::f2f(OpenMesh::Vec3f v)
	{
		return glm::vec3(v[0], v[1], v[2]);
	}

	void FacePicker::clearPickedFaces()
	{
		for (auto faceId = pickedFaces.begin(); faceId != pickedFaces.end(); faceId++) 
		{
			OpenMesh::FaceHandle fh = mesh->face_handle(*faceId);
			for (MyMesh::FaceEdgeIter fe_itr = mesh->fe_iter(fh); fe_itr.is_valid(); fe_itr++)
			{
				OpenMesh::EdgeHandle eh = *fe_itr;
				mesh->set_color(eh, defaultLineColor);
				std::vector<glm::vec3> v = { f2f(defaultLineColor), f2f(defaultLineColor) };
				mesh->setWVBOcSubData(eh.idx() * 2, 2, &v);
			}
		}
		pickedFaces.clear(); 
		VertexToBeColored.clear();
		canHighlightEdge = false;
	}

	void FacePicker::clearColoredEdge(MyMesh::EdgeHandle eh)
	{
		mesh->set_color(eh, defaultLineColor);
	}

}