#include "Patch.h"

namespace CG
{
	Patch::Patch(MyMesh* mesh, std::set<unsigned int>& faceId)
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

	void Patch::identifyBoundary()
	{
		for (auto eh = edges.begin(); eh != edges.end(); eh++)
		{
			OpenMesh::EdgeHandle edge = *eh;

			OpenMesh::HalfedgeHandle heh0 = referenceMesh->halfedge_handle(edge, 0);
			OpenMesh::HalfedgeHandle heh1 = referenceMesh->halfedge_handle(edge, 1);

			OpenMesh::FaceHandle fh0 = referenceMesh->face_handle(heh0);
			OpenMesh::FaceHandle fh1 = referenceMesh->face_handle(heh1);

			if (!(heh0.is_valid() && heh1.is_valid()) && (heh0.is_valid() || heh1.is_valid()))
			{
				boundaryEdges.insert(edge);
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
}