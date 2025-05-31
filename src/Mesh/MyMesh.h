#pragma once
#include "../Graphic/VAO.h"
#include "../Graphic/VBO.h"
#include "../Graphic/UBO.h"
#include "../Graphic/ShaderProgram/GraphicShader.h"
#include "../Graphic/FBO.h"
#include "../Graphic/Material/UnsignedIntTexture.h"

#include <OpenMesh/Core/Mesh/TriMesh_ArrayKernelT.hh>
#include <GL/glew.h>
#include <glm/gtc/type_ptr.hpp>

namespace CG
{
	/* DefaultTraits
	struct DefaultTraits
	{
	  typedef Vec3f  Point;
	  typedef Vec3f  Normal;
	  typedef Vec2f  TexCoord;
	  typedef Vec3uc Color;
	  VertexTraits    {};
	  HalfedgeTraits  {};
	  EdgeTraits      {};
	  FaceTraits      {};
	  
	  VertexAttributes(0);
	  HalfedgeAttributes(Attributes::PrevHalfedge);
	  EdgeAttributes(0);
	  FaceAttributes(0);
	};
	*/

	struct MyTraits : OpenMesh::DefaultTraits
	{
		using Point = OpenMesh::Vec3d;
		using Normal = OpenMesh::Vec3d;
		using Color = OpenMesh::Vec3f;

		// Add normal property to vertices and faces
		VertexAttributes(OpenMesh::Attributes::Normal);
		FaceAttributes(OpenMesh::Attributes::Normal);
		EdgeAttributes(OpenMesh::Attributes::Color);
	};

	class MyMesh : public OpenMesh::TriMesh_ArrayKernelT<MyTraits>
	{
	public:
		MyMesh();
		~MyMesh();

		bool LoadFromFile(std::string filename);

		void Render(const glm::mat4 proj, const glm::mat4 view);
		void setWVBOcSubData(unsigned int offset, unsigned int count, std::vector<glm::vec3>* data);
		inline GLuint getFboColor() { return uintFaceIDTexture.getId(); }

	private:
		void CreateBuffers();
		
		// halfedge, face, and vertex normals
		OpenMesh::Vec3d normal(const HalfedgeHandle he) const;
		OpenMesh::Vec3d normal(const EdgeHandle e) const;
		OpenMesh::Vec3d normal(const FaceHandle f) const;
		OpenMesh::Vec3d normal(const VertexHandle v) const;

		glm::vec3 d2f(OpenMesh::Vec3d v);
		glm::vec3 f2f(OpenMesh::Vec3f v);

		
	private:

		std::vector<glm::vec3> face_vertices;
		std::vector<glm::vec3> face_normals;

		/* Buffers for face ID rendering */
		VAO fVAO;
		FBO fFBO;
		UnsignedIntTexture uintFaceIDTexture;
		GLuint RBO;
		VBO<unsigned int> fVBO; // face ID
		VBO<glm::vec3> fVBOp; // pos
		UBO fUBO;

		/* Face ID shader */
		GraphicShader programFaceID;
		GLuint fModelID;

		std::vector<glm::vec3> face_vertices_for_id_pass; // 與 face_vertices 內容和順序一致
		GLenum DrawBuffers[1];

		/* Buffers for solid rendering */
		VAO sVAO;
		VBO<glm::vec3> sVBOp;
		VBO<glm::vec3> sVBOn;
		UBO sUBO;

		/* Buffers for wireframe rendering */
		VAO wVAO;
		VBO<glm::vec3> wVBOp;
		VBO<glm::vec3> wVBOn;
		VBO<glm::vec3> wVBOc; // draw selected edge
		UBO wUBO;

		/* Phong shader */
		GraphicShader programPhong;
		GLuint pModelID;
		GLuint pMatKaID;
		GLuint pMatKdID;
		GLuint pMatKsID;

		/* Line shader */
		GraphicShader programLine;
		GLuint lModelID;
		GLuint lMatKdID;

		/* Model properties */
		glm::mat4 model;
		glm::vec3 colorAmbient;
		glm::vec3 colorDiffuse;
		glm::vec3 colorSpecular;
		glm::vec3 colorLine;
	};

	// ---- data structures ----
	using Point = MyMesh::Point;

	// ---- Handles ----
	using VertexHandle = MyMesh::VertexHandle;
	using HalfedgeHandle = MyMesh::HalfedgeHandle;
	using EdgeHandle = MyMesh::EdgeHandle;
	using FaceHandle = MyMesh::FaceHandle;

	// ---- Iterators ----
	using EdgeIter = MyMesh::EdgeIter;
	using VertexIter = MyMesh::VertexIter;
	using FaceIter = MyMesh::FaceIter;
	using VertexOHalfedgeIter = MyMesh::VertexOHalfedgeIter;
	using VertexVertexIter = MyMesh::VertexVertexIter;
}

