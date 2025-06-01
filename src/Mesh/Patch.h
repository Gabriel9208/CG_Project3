#pragma once
#include "MyMesh.h"

#include <vector>
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

		std::set<OpenMesh::EdgeHandle> boundaryEdges;
		std::vector<OpenMesh::HalfedgeHandle> orderedBoundaryEdges;

		OpenMesh::Vec3f borderColor;
		OpenMesh::Vec3f defaultLineColor;

		glm::vec3 f2f(OpenMesh::Vec3f v);

	public:
		Patch(MyMesh* mesh, std::set<unsigned int>& faceId);
		~Patch();

		void loadSets(std::set<unsigned int>& faceId);
		void identifyBoundary();
		void generateOrderedBoundary();
		void clear();
	};

}