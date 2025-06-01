#include "Patch.h"

namespace CG
{
	Patch::Patch(MyMesh* mesh, std::set<unsigned int> faceId)
	{
		referenceMesh = mesh;
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
				boundarys.insert(edge);
			}
		}
	}
}