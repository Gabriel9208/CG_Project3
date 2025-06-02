#include "Patch.h"

namespace CG
{
	glm::vec3 Patch::f2f(OpenMesh::Vec3f v)
	{
		return glm::vec3(v[0], v[1], v[2]);;
	}

	Patch::Patch(MyMesh* mesh, std::set<unsigned int>& faceId): borderColor(0, 0, 1), defaultLineColor(0, 0, 0)
	{
		init(mesh, faceId);
	}

	Patch::~Patch()
	{
		clear();
	}

	void Patch::init(MyMesh* mesh, std::set<unsigned int>& faceId)
	{
		referenceMesh = mesh;
		loadSets(faceId);
		identifyBoundary();
		generateOrderedBoundary();
	}

	void Patch::loadSets(std::set<unsigned int>& faceId)
	{
		for (auto id = faceId.begin(); id != faceId.end(); id++)
		{
			OpenMesh::FaceHandle fh = referenceMesh->face_handle(*id);
			faces.insert(fh);

			for (MyMesh::FaceEdgeIter fe_itr = referenceMesh->fe_iter(fh); fe_itr.is_valid(); fe_itr++)
			{
				OpenMesh::EdgeHandle eh = *fe_itr;
				edges.insert(eh);

				for (MyMesh::EdgeVertexIter ev_itr = referenceMesh->ev_iter(eh); ev_itr.is_valid(); ev_itr++)
				{
					OpenMesh::VertexHandle vh = *ev_itr;
					vertices.insert(vh);
				}
			}
		}
	}

	void Patch::updateSet(unsigned int faceId)
	{
		OpenMesh::FaceHandle fh = referenceMesh->face_handle(faceId);
		faces.insert(fh);

		for (MyMesh::FaceEdgeIter fe_itr = referenceMesh->fe_iter(fh); fe_itr.is_valid(); fe_itr++)
		{
			OpenMesh::EdgeHandle eh = *fe_itr;
			edges.insert(eh);

			for (MyMesh::EdgeVertexIter ev_itr = referenceMesh->ev_iter(eh); ev_itr.is_valid(); ev_itr++)
			{
				OpenMesh::VertexHandle vh = *ev_itr;
				vertices.insert(vh);
			}
		}
		
	}

	void Patch::identifyBoundary()
	{
		for (auto eh = edges.begin(); eh != edges.end(); eh++)
		{
			OpenMesh::EdgeHandle edge = *eh;

			OpenMesh::HalfedgeHandle heh0 = referenceMesh->halfedge_handle(edge, 0);
			OpenMesh::HalfedgeHandle heh1 = referenceMesh->halfedge_handle(edge, 1);

			OpenMesh::FaceHandle fh0 = referenceMesh->face_handle(heh0);
			OpenMesh::FaceHandle fh1 = referenceMesh->face_handle(heh1);

			if (heh0.is_valid() && heh1.is_valid())
			{
				if (((faces.find(referenceMesh->face_handle(heh0)) != faces.end() &&
					faces.find(referenceMesh->face_handle(heh1)) == faces.end())
					||
					(faces.find(referenceMesh->face_handle(heh1)) != faces.end() &&
						faces.find(referenceMesh->face_handle(heh0)) == faces.end()))
					&&
					!((faces.find(referenceMesh->face_handle(heh0)) != faces.end() &&
						faces.find(referenceMesh->face_handle(heh1)) == faces.end())
						&&
						(faces.find(referenceMesh->face_handle(heh1)) != faces.end() &&
							faces.find(referenceMesh->face_handle(heh0)) == faces.end())))
				{
					boundaryEdges.insert(edge);
					
					referenceMesh->set_color(edge, borderColor);
					std::vector<glm::vec3> v = { f2f(borderColor), f2f(borderColor) };
					referenceMesh->setWVBOcSubData(edge.idx() * 2, 2, &v);
					
				}
			}
		}
	}

	void Patch::generateOrderedBoundary()
	{
		if (boundaryEdges.empty())
		{
			return;
		}
		orderedBoundaryEdges.clear();

		OpenMesh::EdgeHandle beh = *boundaryEdges.begin(); //boundary edge handle
		OpenMesh::HalfedgeHandle bheh0 = referenceMesh->halfedge_handle(beh, 0);
		OpenMesh::HalfedgeHandle bheh1 = referenceMesh->halfedge_handle(beh, 1);

		OpenMesh::HalfedgeHandle firstHE;

		if (bheh0.is_valid() && faces.find(referenceMesh->face_handle(bheh0)) != faces.end())
		{
			firstHE = bheh0;
		}
		else if (bheh1.is_valid() && faces.find(referenceMesh->face_handle(bheh1)) != faces.end())
		{
			firstHE = bheh1;
		}
		else
		{
			std::cout << "Error in generating ordered boundary.\n";
			return;
		}


		OpenMesh::HalfedgeHandle current_heh = firstHE;
		do
		{
			orderedBoundaryEdges.emplace_back(current_heh);

			OpenMesh::HalfedgeHandle nextHeh = referenceMesh->next_halfedge_handle(current_heh);

			while(true)
			{
				OpenMesh::HalfedgeHandle opposite_heh = referenceMesh->opposite_halfedge_handle(nextHeh);

				if (nextHeh.is_valid() &&
					faces.find(referenceMesh->face_handle(nextHeh)) != faces.end() &&
					faces.find(referenceMesh->face_handle(opposite_heh)) == faces.end()
					)
				{
					current_heh = nextHeh;
					break;
				}

				nextHeh = referenceMesh->next_halfedge_handle(opposite_heh);
			};
		} while (current_heh != firstHE);
	}

	void Patch::clear()
	{
		if (boundaryEdges.size() == 0)
		{
			return;
		}
		for (auto edge = boundaryEdges.begin(); edge != boundaryEdges.end(); edge++)
		{
			OpenMesh::EdgeHandle eh = *edge;

			referenceMesh->set_color(eh, defaultLineColor);
			std::vector<glm::vec3> v = { f2f(defaultLineColor), f2f(defaultLineColor) };
			referenceMesh->setWVBOcSubData(eh.idx() * 2, 2, &v);
		}

		boundaryEdges.clear();
		faces.clear();
		edges.clear();
		vertices.clear();

		orderedBoundaryEdges.clear();
	}
}