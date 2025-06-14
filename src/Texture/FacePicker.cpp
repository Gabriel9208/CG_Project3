#include "FacePicker.h"
#include "../Utilty/Error.h"

#include <glm/gtc/matrix_transform.hpp>
#include <iostream>

namespace CG
{
	FacePicker::FacePicker() : defaultLineColor(0, 0, 0), selectedLineColor(1, 0, 0), canHighlightEdge(false), mesh(nullptr)
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

	void FacePicker::setRange(unsigned int _range)
	{
		range = _range;
	}

	void FacePicker::chooseAllFace()
	{
		for (MyMesh::FaceIter f_it = mesh->faces_begin(); f_it != mesh->faces_end(); ++f_it)
		{
			FaceHandle fh = *f_it;

			if (!mesh->is_valid_handle(fh)) continue;

			unsigned int current_face_id = fh.idx() + 1;
			pickedFaces.insert(current_face_id);

			for (MyMesh::FaceEdgeIter fe_itr = mesh->fe_iter(fh); fe_itr.is_valid(); ++fe_itr)
			{
				OpenMesh::EdgeHandle eh = *fe_itr;
				mesh->set_color(eh, selectedLineColor);

				std::vector<glm::vec3> v = { f2f(selectedLineColor), f2f(selectedLineColor) };
				mesh->setWVBOcSubData(eh.idx() * 2, 2, &v);
			}
		}
	}

	void FacePicker::chooseFace(unsigned int height, double _xpos, double _ypos, glm::mat4 view, glm::mat4 proj, GLuint textureID)
	{
		float depthVal = 0;
		double windowX = _xpos;
		double windowY = height - _ypos;
		GLCall(glReadPixels(windowX, windowY, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &depthVal));

		GLint _viewPort[4];
		GLCall(glGetIntegerv(GL_VIEWPORT, _viewPort));
		glm::vec4 viewport(_viewPort[0], _viewPort[1], _viewPort[2], _viewPort[3]);
		glm::vec3 windowPos(windowX, windowY, depthVal);
		glm::vec3 wp = glm::unProject(windowPos, view, proj, viewport);


		range = std::max(1, int(range));
		std::vector<unsigned int> idxs(range * range, 0);
		int startX = std::max(0, int(windowX - range / 2));
		int startY = std::max(0, int(windowY - range / 2));
		int readWidth  = std::min(int(range), _viewPort[2] - startX);
		int readHeight = std::min(int(range), _viewPort[3] - startY);
		
		GLCall(glGetTextureSubImage(textureID, 0, startX, startY, 0, readWidth, readHeight, 1, GL_RED_INTEGER, GL_UNSIGNED_INT, sizeof(unsigned int) * idxs.size(), idxs.data()));
		GLCall(glBindFramebuffer(GL_READ_FRAMEBUFFER, 0));

		for (int y = 0; y < readHeight; ++y)
		{
			for (int x = 0; x < readWidth; ++x)
			{
				unsigned int idx = idxs[y * range + x];

				if (idx == 0)
					continue;

				if (pickedFaces.find(idx - 1) == pickedFaces.end())
				{
					pickedFaces.insert(idx - 1);
				}
			}
		}

		for (auto faceId = pickedFaces.begin(); faceId != pickedFaces.end(); faceId++)
		{
			OpenMesh::FaceHandle fh = mesh->face_handle(*faceId);

			if (!mesh->is_valid_handle(fh)) continue;

			for (MyMesh::FaceEdgeIter fe_itr = mesh->fe_iter(fh); fe_itr.is_valid(); fe_itr++)
			{
				OpenMesh::EdgeHandle eh = *fe_itr;
				mesh->set_color(eh, selectedLineColor);
				std::vector<glm::vec3> v = { f2f(selectedLineColor), f2f(selectedLineColor) };
				mesh->setWVBOcSubData(eh.idx() * 2, 2, &v);
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

			if (!mesh->is_valid_handle(fh)) continue;

			for (MyMesh::FaceEdgeIter fe_itr = mesh->fe_iter(fh); fe_itr.is_valid(); fe_itr++)
			{
				OpenMesh::EdgeHandle eh = *fe_itr;
				mesh->set_color(eh, defaultLineColor);
				std::vector<glm::vec3> v = { f2f(defaultLineColor), f2f(defaultLineColor) };
				mesh->setWVBOcSubData(eh.idx() * 2, 2, &v);
			}
		}
		pickedFaces.clear(); 
		canHighlightEdge = false;
	}

	void FacePicker::clearColoredEdge(MyMesh::EdgeHandle eh)
	{
		mesh->set_color(eh, defaultLineColor);
	}

}