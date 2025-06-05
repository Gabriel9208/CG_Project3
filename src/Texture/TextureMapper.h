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
		std::vector<OpenMesh::HalfedgeHandle> orderedBoundaryEdges;

		std::map<OpenMesh::VertexHandle, OpenMesh::Vec2d> boundaryUV;
		std::map<OpenMesh::VertexHandle, OpenMesh::Vec2d> allUV;

		OpenMesh::Vec2d uvCenter;

		glm::vec2 translateOffset;
		double rotateDegree;
		double scalingDegree;

		TextureMapper();
		~TextureMapper() {}

		void halfedgeToVertex(std::vector<OpenMesh::HalfedgeHandle>& _orderedBoundaryEdges);
		void calculateBoundaryVertexUV();
		void calculateInnerPointUV();
		double calculateWeight(OpenMesh::VertexHandle center, OpenMesh::VertexHandle last, OpenMesh::VertexHandle curr, OpenMesh::VertexHandle next);
		void calculateUVCenter();

		glm::dvec2 d2d(OpenMesh::Vec2d v);

	public:
		static TextureMapper& getInstance(); 

		void Map(
			MyMesh* mesh, 
			std::vector<OpenMesh::HalfedgeHandle>& orderedBoundaryEdges, 
			std::set<OpenMesh::VertexHandle>* _vertices
		);

		void translate(double x, double y);
		void rotate(double radian);
		void scaling(double scale);

		inline std::map<OpenMesh::VertexHandle, OpenMesh::Vec2d> getAllUVMap() { return allUV; }
		inline glm::vec2 getTranslateOffset() const { return translateOffset; };
		inline double getRotateDegree() const { return rotateDegree; };
		inline double getScalingDegree() const { return scalingDegree; };
	};

}