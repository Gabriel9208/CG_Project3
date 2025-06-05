#include <OpenMesh/Core/IO/MeshIO.hh>

#include "MyMesh.h"
#include "../Texture/FacePicker.h"
#include "../Utilty/Error.h"
#include "../Texture/TextureMapper.h"

namespace CG
{
	MyMesh::MyMesh() :
		fVAO(), fFBO(), uintFaceIDTexture(), RBO(0), fVBO(), fVBOp(), fUBO(),
		programFaceID(), fModelID(-1),
		sVAO(), sVBOp(), sVBOn(), sUBO(),
		wVAO(), wVBOp(), wVBOn(), wUBO(),
		programPhong(), pModelID(-1),	pMatKaID(-1), pMatKdID(-1),	pMatKsID(-1),
		programLine(), lModelID(-1)
	{
		model = glm::mat4(1.0);

		colorAmbient = glm::vec3(0.2, 0.2, 0.2);
		colorDiffuse = glm::vec3(1.0, 1.0, 0.2);
		colorSpecular = glm::vec3(1.0, 1.0, 1.0);
		colorLine = glm::vec3(0.8, 0.8, 0.8);
	}

	MyMesh::~MyMesh()
	{

	}

	bool MyMesh::LoadFromFile(std::string filename, int display_w, int display_h)
	{
		OpenMesh::IO::Options opt = OpenMesh::IO::Options::VertexNormal;
		bool isRead = OpenMesh::IO::read_mesh(*this, filename, opt);

		if (isRead)
		{
			// If the file did not provide vertex normals and mesh has vertex normal, then calculate them
			if (!opt.check(OpenMesh::IO::Options::VertexNormal) && this->has_vertex_normals())
			{
				this->update_normals();
			}

			CreateBuffers();
		}

		if (!has_edge_colors()) { 
			request_edge_colors(); 
		}

		return isRead;
	}

	void MyMesh::Render(const glm::mat4 proj, const glm::mat4 view, int mode)
	{
#pragma region Face ID to frame buffer
		
		programFaceID.use();
		fVAO.bind();
		fFBO.bind();
		GLCall(glUniformMatrix4fv(fModelID, 1, GL_FALSE, &model[0][0]));

		// update data to UBO for MVP
		fUBO.bind();
		fUBO.fillInData(0, sizeof(glm::mat4), &view);
		fUBO.fillInData(sizeof(glm::mat4), sizeof(glm::mat4), &proj);
		fUBO.unbind();

		GLuint clear_color_uint[4] = {0, 0, 0, 0}; 
		GLCall(glClearBufferuiv(GL_COLOR, 0, clear_color_uint)); // clear GL_COLOR_ATTACHMENT0
		GLCall(glClear(GL_DEPTH_BUFFER_BIT));

		GLCall(glDrawArrays(GL_TRIANGLES, 0, face_vertices_for_id_pass.size()));
		
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		
#pragma endregion

#pragma region Solid Rendering
		
		programPhong.use();
		sVAO.bind();

		GLCall(glUniformMatrix4fv(pModelID, 1, GL_FALSE, &model[0][0]));
		GLCall(glUniform3fv(pMatKaID, 1, &colorAmbient[0]));
		GLCall(glUniform3fv(pMatKdID, 1, &colorDiffuse[0]));
		GLCall(glUniform3fv(pMatKsID, 1, &colorSpecular[0]));

		// update data to UBO for MVP
		sUBO.bind();
		sUBO.fillInData(0, sizeof(glm::mat4), &view);
		sUBO.fillInData(sizeof(glm::mat4), sizeof(glm::mat4), &proj);

		// Draw solid mesh
		GLCall(glDrawArrays(GL_TRIANGLES, 0, this->n_faces() * 3));
#pragma endregion
		
#pragma region Wireframe Rendering
		if (mode != 0)
		{
			programLine.use();
			wVAO.bind();

			GLCall(glUniformMatrix4fv(lModelID, 1, GL_FALSE, &model[0][0]));

			// update data to UBO for MVP
			wUBO.bind();
			wUBO.fillInData(0, sizeof(glm::mat4), &view);
			wUBO.fillInData(sizeof(glm::mat4), sizeof(glm::mat4), &proj);

			GLCall(glLineWidth(1.5f));
			// Draw wireframe mesh
			GLCall(glDrawArrays(GL_LINES, 0, this->n_edges() * 2));
		}
#pragma endregion

		// Unbind shader and VAO
		GLCall(glBindVertexArray(0));
		GLCall(glUseProgram(0));
	}

	void MyMesh::setWVBOcSubData(unsigned int offset, unsigned int count, std::vector<glm::vec3>* data)
	{
		wVBOc.setSubData(offset, count, *data);
	}

	void MyMesh::resizeTextureRBO(unsigned int w, unsigned int h)
	{
		// update texture and rbo size
		uintFaceIDTexture.resize(w, h);

		fFBO.bind();
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, RBO));
		GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, w, h));
		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO));
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, uintFaceIDTexture.getId(), 0));
		fFBO.unbind();
	}

	void MyMesh::CreateBuffers()
	{
#pragma region Phong Shader
		ShaderInfo shadersPhong[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/DSPhong_Material.vp" }, //vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/DSPhong_Material.fp" }, //fragment shader
			{ GL_NONE, NULL } };

		programPhong.load(shadersPhong);//Ūshader

		programPhong.use();

		pModelID = glGetUniformLocation(programPhong.getId(), "Model");
		pMatKaID = glGetUniformLocation(programPhong.getId(), "Material.Ka");
		pMatKdID = glGetUniformLocation(programPhong.getId(), "Material.Kd");
		pMatKsID = glGetUniformLocation(programPhong.getId(), "Material.Ks");
#pragma endregion

#pragma region Line Shader
		ShaderInfo shadersLine[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/line.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/line.fp" },//fragment shader
			{ GL_NONE, NULL } };
		programLine.load(shadersLine);

		programLine.use();

		lModelID = glGetUniformLocation(programLine.getId(), "Model");
#pragma endregion

#pragma region Texture Shader
		
#pragma endregion

#pragma region faceID Shader
		ShaderInfo shadersFace[] = {
			{ GL_VERTEX_SHADER, "../../res/shaders/faceid.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "../../res/shaders/faceid.fp" },//fragment shader
			{ GL_NONE, NULL } };
		programFaceID.load(shadersFace);

		programFaceID.use();

		fModelID = glGetUniformLocation(programFaceID.getId(), "Model");
#pragma endregion

#pragma region Solid Rendering
		sVAO.bind();

		// UBO
		sUBO.bind();
		sUBO.initialize(sizeof(glm::mat4) * 2);

		// bind UBO to its idx
		sUBO.associateWithShaderBlock(programPhong.getId(), "MatVP", 0);

		// triangle vertex index
		for (FaceHandle f : this->faces())
		{
			// this is basically a triangle fan for any face valence
			MyMesh::ConstFaceVertexCCWIter it = this->cfv_ccwbegin(f);
			VertexHandle first = *it;
			++it;
			uint face_triangles = this->valence(f) - 2;
			for (uint j = 0; j < face_triangles; ++j)
			{
				face_vertices.push_back(d2f(point(first)));
				face_normals.push_back(d2f(normal(first)));

				face_vertices.push_back(d2f(point(*it)));
				face_normals.push_back(d2f(normal(*it)));
				++it;
				face_vertices.push_back(d2f(point(*it)));
				face_normals.push_back(d2f(normal(*it)));
			}
		}

		sVBOp.bind();
		sVBOp.initialize(face_vertices, GL_STATIC_DRAW);
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));
		sVBOp.unbind();

		sVBOn.bind();
		sVBOn.initialize(face_normals, GL_STATIC_DRAW);
		GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(1));
		sVBOn.unbind();

		sVAO.unbind();
#pragma endregion

#pragma region Wireframe Rendering
		wVAO.bind();

		// UBO
		wUBO.bind();
		wUBO.initialize(sizeof(glm::mat4) * 2);

		// get uniform struct size
		sUBO.associateWithShaderBlock(programLine.getId(), "MatVP", 0);

		// triangle vertex index
		FacePicker& fp = FacePicker::getInstance();
		std::vector<glm::vec3> edge_vertices;
		std::vector<glm::vec3> edge_normals;
		std::vector<glm::vec3> edge_colors;
		for (EdgeHandle e : edges())
		{
			set_color(e, fp.getDefaultEdgeColor());
			HalfedgeHandle he = this->halfedge_handle(e, 0);
			edge_vertices.push_back(d2f(point(from_vertex_handle(he))));
			edge_vertices.push_back(d2f(point(to_vertex_handle(he))));
			edge_normals.push_back(d2f(normal(e)));
			edge_normals.push_back(d2f(normal(e)));
			edge_colors.push_back(f2f(color(e)));
			edge_colors.push_back(f2f(color(e)));
		}

		wVBOp.initialize(edge_vertices, GL_STATIC_DRAW);
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));
		wVBOp.unbind();

		wVBOn.initialize(edge_normals, GL_STATIC_DRAW);
		GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(1));
		wVBOn.unbind();

		wVBOc.initialize(edge_colors, GL_STATIC_DRAW) ;
		GLCall(glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(2));
		wVBOc.unbind();

		wVAO.unbind();
#pragma endregion
		
#pragma region FaceID Rendering

		fFBO.bind();

		uintFaceIDTexture.setup(1280, 720);

		GLCall(glGenRenderbuffers(1, &RBO));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, RBO));
		GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720));
		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, RBO));
		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, uintFaceIDTexture.getId(), 0));

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        std::vector<unsigned int> vertex_face_ids;		
		for (FaceHandle fh: this->faces())
		{
			unsigned int current_face_id = fh.idx() + 1;

			MyMesh::ConstFaceVertexCCWIter v_it = this->cfv_ccwbegin(fh);
			VertexHandle first = *v_it;
            
            face_vertices_for_id_pass.push_back(d2f(point(first)));
            vertex_face_ids.push_back(current_face_id);
			
			++v_it;
            face_vertices_for_id_pass.push_back(d2f(point(*v_it)));
            vertex_face_ids.push_back(current_face_id);

            ++v_it; 
            face_vertices_for_id_pass.push_back(d2f(point(*v_it)));
            vertex_face_ids.push_back(current_face_id);
		}
		
		fVAO.bind();
		
		fUBO.initialize(sizeof(glm::mat4) * 2);
		fUBO.associateWithShaderBlock(programFaceID.getId(), "MatVP", 0);

		fVBOp.initialize(face_vertices_for_id_pass, GL_STATIC_DRAW);
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));

		fVBO.initialize(vertex_face_ids, GL_STATIC_DRAW);
		GLCall(glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, 0));
		GLCall(glEnableVertexAttribArray(1));

		fUBO.unbind();
		fVBO.unbind();
		fVAO.unbind();

#pragma endregion

	}

	OpenMesh::Vec3d MyMesh::normal(const HalfedgeHandle he) const
	{
		const FaceHandle f = face_handle(he);
		if (f.is_valid()) 
		{
			return normal(f);
		}
		else
		{
			return OpenMesh::Vec3d(0, 0, 0);
		}
	}

	OpenMesh::Vec3d MyMesh::normal(const EdgeHandle e) const
	{
		const HalfedgeHandle he0 = halfedge_handle(e, 0);
		const HalfedgeHandle he1 = halfedge_handle(e, 1);
		assert(!is_boundary(he0) || !is_boundary(he1)); // free edge, bad
		if (is_boundary(he0))
		{
			return normal(face_handle(he1));
		}
		else if (is_boundary(he1))
		{
			return normal(face_handle(he0));
		}
		else 
		{
			return (normal(face_handle(he0)) + normal(face_handle(he1))).normalized();
		}
	}

	OpenMesh::Vec3d MyMesh::normal(const FaceHandle f) const
	{
		return OpenMesh::TriMesh_ArrayKernelT<MyTraits>::normal(f);
	}

	OpenMesh::Vec3d MyMesh::normal(const VertexHandle v) const
	{
		return OpenMesh::TriMesh_ArrayKernelT<MyTraits>::normal(v);
	}

	glm::vec3 MyMesh::d2f(OpenMesh::Vec3d v)
	{
		return glm::vec3(v[0], v[1], v[2]);
	}
	glm::vec3 MyMesh::f2f(OpenMesh::Vec3f v)
	{
		return glm::vec3(v[0], v[1], v[2]);
	}
}