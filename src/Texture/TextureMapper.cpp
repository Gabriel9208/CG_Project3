#include "TextureMapper.h"

#include <iostream>
#include <cmath>
#include <Eigen/SparseCore>
#include <Eigen/Sparse>
#include <map>

#define PI 3.14159265359

namespace CG
{
	TextureMapper TextureMapper::instance;

	TextureMapper::TextureMapper() : referenceMesh(nullptr), uvCenter(0, 0), translateOffset(0, 0), rotateDegree(0), scalingDegree(0)
	{}


	TextureMapper& TextureMapper::getInstance()
	{
		return instance;
	}

	void TextureMapper::Map(
		MyMesh* mesh, 
		std::vector<OpenMesh::HalfedgeHandle>& orderedBoundaryEdges, 
		std::set<OpenMesh::VertexHandle>* _vertices
	)
	{
		// clear state
		orderedBoundaryVertex.clear();
		boundaryUV.clear();
		allUV.clear();
		referenceMesh = mesh;
		vertices = _vertices;
		translateOffset = glm::vec2(0, 0);
		rotateDegree = 0;
		scalingDegree = 0;

		// init boundary
		halfedgeToVertex(orderedBoundaryEdges);
		calculateBoundaryVertexUV();

		// calculate inner points
		calculateInnerPointUV();
	}

	void TextureMapper::translate(double x, double y)
	{
		for (auto itr = allUV.begin(); itr != allUV.end(); itr++)
		{
			itr->second[0] += x;
			itr->second[1] += y;
		}

		calculateUVCenter();

		translateOffset[0] += x;
		translateOffset[1] += y;
	}

	void TextureMapper::rotate(double degree) 
	{
		double radian = glm::radians(degree);
		for (auto itr = allUV.begin(); itr != allUV.end(); itr++)
		{
			glm::dvec2 holder(itr->second[0] - uvCenter[0], itr->second[1] - uvCenter[1]);

			glm::dmat2 rotate(glm::vec2(std::cos(radian), std::sin(radian)), glm::vec2(-std::sin(radian), std::cos(radian)));
			
			holder = rotate * holder;

			holder.x += uvCenter[0];
			holder.y += uvCenter[1];

			itr->second[0] = holder.x;
			itr->second[1] = holder.y;
		}

		rotateDegree += degree;
	}

	void TextureMapper::scaling(double scale)
	{
		for (auto itr = allUV.begin(); itr != allUV.end(); itr++)
		{
			glm::dvec2 direction = d2d(itr->second) - d2d(uvCenter);

			double dirLength = glm::length(direction);
			if (dirLength < 1e-8) {
				std::cout << "Cannot scale down anymore.\n";
				return;  
			}
		}

		for (auto itr = allUV.begin(); itr != allUV.end(); itr++)
		{
			glm::dvec2 direction = d2d(itr->second) - d2d(uvCenter);
			
			// Move proportional to distance from center
			itr->second[0] += scale * direction[0];
			itr->second[1] += scale * direction[1];
		}

		scalingDegree += scale;
	}

	void TextureMapper::halfedgeToVertex(std::vector<OpenMesh::HalfedgeHandle>& _orderedBoundaryEdges)
	{
		orderedBoundaryEdges = _orderedBoundaryEdges;
		for (std::vector<OpenMesh::HalfedgeHandle>::iterator itr = orderedBoundaryEdges.begin();
			itr != orderedBoundaryEdges.end();
			itr++)
		{
			orderedBoundaryVertex.emplace_back(referenceMesh->to_vertex_handle(*itr));
		}
	}

	void TextureMapper::calculateBoundaryVertexUV()
	{
		if (orderedBoundaryVertex.empty())
		{
			std::cout << "Cannot calculate boundary vertex uv coord due to empty vertex.\n";
			return;
		}

		double totalDistance = 0;
		std::vector<double> verticesDistance;
		for (std::vector<OpenMesh::VertexHandle>::iterator it = orderedBoundaryVertex.begin() + 1;
			it != orderedBoundaryVertex.end();
			it++)
		{
			OpenMesh::VertexHandle lastVertex = *(it - 1);
			OpenMesh::VertexHandle currentVertex = *it;

			MyTraits::Point posCurr = referenceMesh->point(currentVertex);
			MyTraits::Point posLast = referenceMesh->point(lastVertex);

			verticesDistance.emplace_back((posCurr - posLast).length());
			totalDistance += verticesDistance[verticesDistance.size() - 1];
		}

		verticesDistance.emplace_back((referenceMesh->point(*orderedBoundaryVertex.begin()) - 
			referenceMesh->point(*(orderedBoundaryVertex.end() - 1))).length());
		totalDistance += verticesDistance[verticesDistance.size() - 1];

		double angle = 0;
		int idx = 0;
		boundaryUV[orderedBoundaryVertex[0]] = OpenMesh::Vec2f(1, 0.5);
		for (std::vector<double>::iterator i = verticesDistance.begin();
			i != verticesDistance.end() - 1;
			i++)
		{
			idx++;
			angle += (2 * PI) * (*i / totalDistance);
			double x = 0.5 * std::cos(angle) + 0.5;
			double y = 0.5 * std::sin(angle) + 0.5;

			boundaryUV[orderedBoundaryVertex[idx]] = OpenMesh::Vec2f(x, y);
		}
	}

	void TextureMapper::calculateInnerPointUV()
	{
		// chaeck if all element in orderedBoundaryVertex are unique
		std::set<OpenMesh::VertexHandle> boundaryVertex;
		for (std::vector<OpenMesh::VertexHandle>::iterator it = orderedBoundaryVertex.begin();
			it != orderedBoundaryVertex.end();
			it++)
		{
			boundaryVertex.insert(*it);
		}

		if (boundaryVertex.size() != orderedBoundaryVertex.size())
		{
			std::cout << "Element in orderedBoundaryVertex are not unique.\n";
			return;
		}

		
		const unsigned int DIMENTION = vertices->size() - orderedBoundaryVertex.size(); // mappings of vertex and its index
		
		if (DIMENTION == 0)
		{
			allUV = boundaryUV;
			return;
		}

		Eigen::SparseMatrix<double> matrixA(DIMENTION, DIMENTION); // main matrix to be calculated	
		Eigen::VectorXd vectorU = Eigen::VectorXd::Zero(DIMENTION); // right hand side vector for u	
		Eigen::VectorXd vectorV = Eigen::VectorXd::Zero(DIMENTION); // right hand side vector for v

		std::map<OpenMesh::VertexHandle, unsigned int> overallIndex; // give each vertex a index
		std::map<unsigned int, OpenMesh::VertexHandle> matrixEntryIndexToVertexHandle; // give each vertex a index
		std::map<unsigned int, unsigned int> overallIndexToMatrixEntryIndex; // give each vertex a index

		std::set<OpenMesh::VertexHandle> innerVertex; // a set of inner vertex


		// Assign index to all vertices
		unsigned int overallIdx = 0;
		unsigned int matrixIdx = 0; // only include inner points
		for (std::set<OpenMesh::VertexHandle>::iterator it = vertices->begin();
			it != vertices->end();
			it++)
		{
			overallIndex[*it] = overallIdx;

			// Fill in inner vertex
			if (boundaryVertex.find(*it) == boundaryVertex.end())
			{
				matrixEntryIndexToVertexHandle[matrixIdx] = *it;
				overallIndexToMatrixEntryIndex[overallIdx] = matrixIdx;
				innerVertex.insert(*it);
				matrixIdx++;
			}

			overallIdx++;
		}

		// fill in matrixA and vectorB
		std::vector<Eigen::Triplet<double>> tripletListLeft; // element for filling matrix
		for (std::set<OpenMesh::VertexHandle>::iterator it = innerVertex.begin();
			it != innerVertex.end();
			it++)
		{
			// get all neighbos of a vertex (ordered)
			std::vector<OpenMesh::VertexHandle> orderedNeighbors;
			for (MyMesh::ConstVertexOHalfedgeIter voh_it = referenceMesh->cvoh_iter(*it); voh_it.is_valid(); ++voh_it) {
				orderedNeighbors.push_back(referenceMesh->to_vertex_handle(*voh_it));
			}

			// calculate weight of each neighbor
			double totalWeightOfARow = 0;
			for (int i = 0; i < orderedNeighbors.size(); i++)
			{
				int lastElement = i == 0 ? orderedNeighbors.size() - 1 : i - 1;
				int nextElement = i == orderedNeighbors.size() - 1 ? 0 : i + 1;

				OpenMesh::VertexHandle current = orderedNeighbors[i];

				double weight = calculateWeight(*it, 
					orderedNeighbors[lastElement],
					current,
					orderedNeighbors[nextElement]);

				totalWeightOfARow += weight;

				// not boundary -> put in matrix with negative weight
				if (boundaryVertex.find(current) == boundaryVertex.end())
				{
					tripletListLeft.emplace_back(
						overallIndexToMatrixEntryIndex[overallIndex[*it]], 
						overallIndexToMatrixEntryIndex[overallIndex[orderedNeighbors[i]]], 
						-weight
					);
				}
				else // boundary -> add to right hand side 
				{
					vectorU[overallIndexToMatrixEntryIndex[overallIndex[*it]]] += weight * boundaryUV[current][0];
					vectorV[overallIndexToMatrixEntryIndex[overallIndex[*it]]] += weight * boundaryUV[current][1];
				}
			}

			// W
			tripletListLeft.emplace_back(
				overallIndexToMatrixEntryIndex[overallIndex[*it]], 
				overallIndexToMatrixEntryIndex[overallIndex[*it]], 
				totalWeightOfARow
			);
		}

		matrixA.setFromTriplets(tripletListLeft.begin(), tripletListLeft.end());


		// solve linear system
		Eigen::SparseLU<Eigen::SparseMatrix<double>> solver;

		solver.compute(matrixA); 
		if (solver.info() != Eigen::Success) {
			std::cerr << "Eigen decomposition failed!" << std::endl;
			return;
		}

		Eigen::VectorXd innerU = solver.solve(vectorU);
		if (solver.info() != Eigen::Success) {
			std::cerr << "Eigen solve for innerU failed!" << std::endl;
			return;
		}

		Eigen::VectorXd innerV = solver.solve(vectorV);
		if (solver.info() != Eigen::Success) {
			std::cerr << "Eigen solve for innerY failed!" << std::endl;
			return;
		}

		for (auto& vh : innerVertex) 
		{
			unsigned int ovIdx = overallIndex[vh];
			unsigned int matIdx = overallIndexToMatrixEntryIndex[ovIdx];
			allUV[vh] = OpenMesh::Vec2f(innerU[matIdx], innerV[matIdx]);
		}

		for (auto& pair : boundaryUV) 
		{
			allUV[pair.first] = pair.second;
		}
	}

	// mean-value
	double TextureMapper::calculateWeight(
		OpenMesh::VertexHandle center,
		OpenMesh::VertexHandle last,
		OpenMesh::VertexHandle curr, 
		OpenMesh::VertexHandle next
	)
	{
		MyMesh::Point centerPos = referenceMesh->point(center);
		MyMesh::Point lastPos = referenceMesh->point(last);
		MyMesh::Point currPos = referenceMesh->point(curr);
		MyMesh::Point nextPos = referenceMesh->point(next);

		double angleLast, angleNext, weight;

		angleLast = OpenMesh::dot((lastPos - centerPos), (currPos - centerPos));
		angleLast /= (lastPos - centerPos).length() * (currPos - centerPos).length();
		angleLast = std::acos(angleLast);

		angleNext = OpenMesh::dot((currPos - centerPos), (nextPos - centerPos));
		angleNext /= (currPos - centerPos).length() * (nextPos - centerPos).length();
		angleNext = std::acos(angleNext);

		weight = (std::tan(angleLast / 2) + std::tan(angleNext / 2)) / (currPos - centerPos).length();

		return weight;
	}

	void TextureMapper::calculateUVCenter()
	{
		if (allUV.size() == 0)
		{
			return;
		}

		double x = 0, y = 0;
		for (auto itr = allUV.begin(); itr != allUV.end(); itr++)
		{
			x += itr->second[0];
			y += itr->second[1];
		}

		uvCenter[0] = x / allUV.size();
		uvCenter[1] = y / allUV.size();
	}
	glm::dvec2 TextureMapper::d2d(OpenMesh::Vec2d v)
	{
		return glm::dvec2(v[0], v[1]);
	}
}