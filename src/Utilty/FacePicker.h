#pragma once
#include "../Mesh/MyMesh.h"
#include "../Mesh/Patch.h"

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <glm/glm.hpp>
#include <gl/glew.h>
#include <set>
#include <vector>

namespace CG
{
	class FacePicker
	{
	private:
		static FacePicker instance;

		MyMesh* mesh;
		Patch* patch;
		std::set<unsigned int> pickedFaces;

		OpenMesh::Vec3f defaultLineColor;
		OpenMesh::Vec3f selectedLineColor;

		bool canHighlightEdge;

		FacePicker();
		~FacePicker() {}

		glm::vec3 d2f(OpenMesh::Vec3d v);
		glm::vec3 f2f(OpenMesh::Vec3f v);


	public:
		static FacePicker& getInstance();

		void registerToMesh(MyMesh* target);
		void chooseFace(unsigned int height, double _xpos, double _ypos, glm::mat4 view, glm::mat4 proj, GLuint textureID);
		void clearPickedFaces();
		void clearColoredEdge(MyMesh::EdgeHandle eh);

		inline std::set<unsigned int>& getFacesPicked() { return pickedFaces; }
		inline OpenMesh::Vec3f getDefaultEdgeColor() const { return defaultLineColor; }
		inline OpenMesh::Vec3f getHightlightEdgeColor() const { return selectedLineColor; }
	};
	
}