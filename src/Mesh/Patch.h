#pragma once
#include "MyMesh.h"

#include <set>

namespace CG
{
	class Patch
	{
	private:
		MyMesh* referenceMesh;

		std::set<OpenMesh::FaceHandle> faces;
		std::set<OpenMesh::EdgeHandle> edges;
		std::set<OpenMesh::VertexHandle> vertices;

		std::set<OpenMesh::EdgeHandle> boundarys;

	public:
		Patch(MyMesh* mesh, std::set<unsigned int> faceId);
		~Patch() {}

		void identifyBoundary();
	};

}