#pragma once
#include "../Mesh/MyMesh.h"

#include <vector>
#include <set>
#include <map>

namespace CG
{
	class TextureMapper
	{
	private:
		static TextureMapper instance;

		MyMesh* referenceMesh;
		std::set<OpenMesh::VertexHandle>* vertices;

		std::vector<OpenMesh::VertexHandle> orderedBoundaryVertex;

		std::map<OpenMesh::VertexHandle, OpenMesh::Vec2d> boundaryUV;

		std::map<OpenMesh::VertexHandle, OpenMesh::Vec2f> allUV;

		TextureMapper();
		~TextureMapper() {}

		void halfedgeToVertex(std::vector<OpenMesh::HalfedgeHandle>& orderedBoundaryEdges);
		void calculateBoundaryVertexUV();
		bool calculateInnerPointUV();
		double calculateWeight(OpenMesh::VertexHandle center, OpenMesh::VertexHandle last, OpenMesh::VertexHandle curr, OpenMesh::VertexHandle next);
			
	public:
		static TextureMapper& getInstance(); 

		void Map(
			MyMesh* mesh, 
			std::vector<OpenMesh::HalfedgeHandle>& orderedBoundaryEdges, 
			std::set<OpenMesh::VertexHandle>* _vertices
		);

		inline std::map<OpenMesh::VertexHandle, OpenMesh::Vec2f> getAllUVMap() { return allUV; }
	};

}