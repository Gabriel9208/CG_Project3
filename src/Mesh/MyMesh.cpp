#include <OpenMesh/Core/IO/MeshIO.hh>

#include "MyMesh.h"
#include <Utilty/LoadShaders.h>
#include "../Utilty/Error.h"

namespace CG
{
	MyMesh::MyMesh()
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

	bool MyMesh::LoadFromFile(std::string filename)
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

		return isRead;
	}

	void MyMesh::Render(const glm::mat4 proj, const glm::mat4 view)
	{
#pragma region Face ID to frame buffer
		
		GLCall(glUseProgram(programFaceID));
		GLCall(glBindVertexArray(fVao));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

		GLCall(glUniformMatrix4fv(fModelID, 1, GL_FALSE, &model[0][0]));

		// update data to UBO for MVP
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, fUbo));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &proj));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));


		GLuint clear_color_uint[4] = {0, 0, 0, 0}; // ID 0 代表背景
		GLCall(glClearBufferuiv(GL_COLOR, 0, clear_color_uint)); // 清除 GL_COLOR_ATTACHMENT0
		GLCall(glClear(GL_DEPTH_BUFFER_BIT)); // 清除深度緩衝區

		GLCall(glDrawArrays(GL_TRIANGLES, 0, face_vertices_for_id_pass.size()));
		
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));
		
#pragma endregion

#pragma region Solid Rendering
		
		GLCall(glUseProgram(programPhong));
		GLCall(glBindVertexArray(sVAO));

		GLCall(glUniformMatrix4fv(pModelID, 1, GL_FALSE, &model[0][0]));
		GLCall(glUniform3fv(pMatKaID, 1, &colorAmbient[0]));
		GLCall(glUniform3fv(pMatKdID, 1, &colorDiffuse[0]));
		GLCall(glUniform3fv(pMatKsID, 1, &colorSpecular[0]));

		// update data to UBO for MVP
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, sUBO));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &proj));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

		// Draw solid mesh
		GLCall(glDrawArrays(GL_TRIANGLES, 0, this->n_faces() * 3));
#pragma endregion
		
#pragma region Wireframe Rendering
		GLCall(glUseProgram(programLine));
		GLCall(glBindVertexArray(wVAO));

		GLCall(glUniformMatrix4fv(lModelID, 1, GL_FALSE, &model[0][0]));
		GLCall(glUniform3fv(lMatKdID, 1, &colorLine[0]));

		// update data to UBO for MVP
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, wUBO));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, 0, sizeof(glm::mat4), &view));
		GLCall(glBufferSubData(GL_UNIFORM_BUFFER, sizeof(glm::mat4), sizeof(glm::mat4), &proj));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));

		GLCall(glLineWidth(1.5f));
		// Draw wireframe mesh
		GLCall(glDrawArrays(GL_LINES, 0, this->n_edges() * 2));
#pragma endregion

		// Unbind shader and VAO
		GLCall(glBindVertexArray(0));
		GLCall(glUseProgram(0));
	}

	void MyMesh::CreateBuffers()
	{
#pragma region Phong Shader
		ShaderInfo shadersPhong[] = {
			{ GL_VERTEX_SHADER, "./res/shaders/DSPhong_Material.vp" }, //vertex shader
			{ GL_FRAGMENT_SHADER, "./res/shaders/DSPhong_Material.fp" }, //fragment shader
			{ GL_NONE, NULL } };
		programPhong = LoadShaders(shadersPhong);//Ūshader

		GLCall(glUseProgram(programPhong));//uniformѼƼƭȫeuse shader

		pMatVPID = glGetUniformBlockIndex(programPhong, "MatVP");
		pModelID = glGetUniformLocation(programPhong, "Model");
		pMatKaID = glGetUniformLocation(programPhong, "Material.Ka");
		pMatKdID = glGetUniformLocation(programPhong, "Material.Kd");
		pMatKsID = glGetUniformLocation(programPhong, "Material.Ks");
#pragma endregion

#pragma region Line Shader
		ShaderInfo shadersLine[] = {
			{ GL_VERTEX_SHADER, "./res/shaders/line.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "./res/shaders/line.fp" },//fragment shader
			{ GL_NONE, NULL } };
		programLine = LoadShaders(shadersLine);//Ūshader

		GLCall(glUseProgram(programLine));//uniformѼƼƭȫeuse shader

		lMatVPID = glGetUniformBlockIndex(programLine, "MatVP");
		lModelID = glGetUniformLocation(programLine, "Model");
		lMatKdID = glGetUniformLocation(programLine, "Material.Kd");
#pragma endregion

#pragma region faceID Shader
		ShaderInfo shadersFace[] = {
			{ GL_VERTEX_SHADER, "./res/shaders/faceid.vp" },//vertex shader
			{ GL_FRAGMENT_SHADER, "./res/shaders/faceid.fp" },//fragment shader
			{ GL_NONE, NULL } };
		programFaceID = LoadShaders(shadersFace);

		GLCall(glUseProgram(programFaceID));//uniformѼƼƭȫeuse shader

		fMatVPID = glGetUniformBlockIndex(programFaceID, "MatVP");
		fModelID = glGetUniformLocation(programFaceID, "Model");
#pragma endregion


#pragma region Solid Rendering
		GLCall(glGenVertexArrays(1, &sVAO));
		GLCall(glBindVertexArray(sVAO));

		// UBO
		GLCall(glGenBuffers(1, &sUBO));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, sUBO));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_DYNAMIC_DRAW));
		// get uniform struct size
		int sUBOsize = 0;
		GLCall(glGetActiveUniformBlockiv(programPhong, pMatVPID, GL_UNIFORM_BLOCK_DATA_SIZE, &sUBOsize));
		// bind UBO to its idx
		GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 0, sUBO, 0, sUBOsize));
		GLCall(glUniformBlockBinding(programPhong, pMatVPID, 0));

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

		GLCall(glGenBuffers(1, &sVBOp));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, sVBOp));
		GLCall(glBufferData(GL_ARRAY_BUFFER, face_vertices.size() * sizeof(glm::vec3), glm::value_ptr(face_vertices[0]), GL_STATIC_DRAW));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));

		GLCall(glGenBuffers(1, &sVBOn));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, sVBOn));
		GLCall(glBufferData(GL_ARRAY_BUFFER, face_normals.size() * sizeof(glm::vec3), glm::value_ptr(face_normals[0]), GL_STATIC_DRAW));
		GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(1));

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GLCall(glBindVertexArray(0));
#pragma endregion

#pragma region Wireframe Rendering
		GLCall(glGenVertexArrays(1, &wVAO));
		GLCall(glBindVertexArray(wVAO));

		// UBO
		GLCall(glGenBuffers(1, &wUBO));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, wUBO));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_DYNAMIC_DRAW));
		// get uniform struct size
		int wUBOsize = 0;
		GLCall(glGetActiveUniformBlockiv(programLine, lMatVPID, GL_UNIFORM_BLOCK_DATA_SIZE, &wUBOsize));
		// bind UBO to its idx
		GLCall(glBindBufferRange(GL_UNIFORM_BUFFER, 0, wUBO, 0, wUBOsize));
		GLCall(glUniformBlockBinding(programLine, lMatVPID, 0));

		// triangle vertex index
		std::vector<glm::vec3> edge_vertices;
		std::vector<glm::vec3> edge_normals;
		for (EdgeHandle e : this->edges())
		{
			HalfedgeHandle he = this->halfedge_handle(e, 0);
			edge_vertices.push_back(d2f(point(from_vertex_handle(he))));
			edge_vertices.push_back(d2f(point(to_vertex_handle(he))));
			edge_normals.push_back(d2f(normal(e)));
			edge_normals.push_back(d2f(normal(e)));
		}

		GLCall(glGenBuffers(1, &wVBOp));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, wVBOp));
		GLCall(glBufferData(GL_ARRAY_BUFFER, edge_vertices.size() * sizeof(glm::vec3), glm::value_ptr(edge_vertices[0]), GL_STATIC_DRAW));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));

		GLCall(glGenBuffers(1, &wVBOn));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, wVBOn));
		GLCall(glBufferData(GL_ARRAY_BUFFER, edge_normals.size() * sizeof(glm::vec3), glm::value_ptr(edge_normals[0]), GL_STATIC_DRAW));
		GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(1));

		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GLCall(glBindVertexArray(0));
#pragma endregion
		
		// create framebuffer
		GLCall(glGenFramebuffers(1, &fbo));
		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, fbo));

		GLCall(glGenTextures(1, &fboColor));
		GLCall(glBindTexture(GL_TEXTURE_2D, fboColor));
		GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, 1280, 720, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL));
		GLCall(glTextureParameteri(fboColor, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
		GLCall(glTextureParameteri(fboColor, GL_TEXTURE_MAG_FILTER, GL_NEAREST));

		GLCall(glGenRenderbuffers(1, &rbo));
		GLCall(glBindRenderbuffer(GL_RENDERBUFFER, rbo));
		GLCall(glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, 1280, 720));
		GLCall(glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, rbo));

		GLCall(glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, fboColor, 0));

		GLCall(glBindFramebuffer(GL_FRAMEBUFFER, 0));

        std::vector<unsigned int> vertex_face_ids;		
		for (FaceHandle fh: this->faces())
		{
			unsigned int current_face_id = fh.idx() + 1;

			MyMesh::ConstFaceVertexCCWIter v_it = this->cfv_ccwbegin(fh);
			VertexHandle first = *v_it;
            ++v_it;
            uint face_triangles = this->valence(fh) - 2;
            for (uint j = 0; j < face_triangles; ++j)
            {
                // 第一個頂點
                face_vertices_for_id_pass.push_back(d2f(point(first)));
                vertex_face_ids.push_back(current_face_id);

                // 第二個頂點
                face_vertices_for_id_pass.push_back(d2f(point(*v_it)));
                vertex_face_ids.push_back(current_face_id);

                // 第三個頂點 (如果不是第一個三角形，則 it 已經被 ++ 過)
                if (j == 0) ++v_it; // 只有第一個三角形需要移動it到第三個頂點
                face_vertices_for_id_pass.push_back(d2f(point(*v_it)));
                vertex_face_ids.push_back(current_face_id);

                if (j > 0) ++v_it; // 後續三角形的第三個頂點需要移動it
            }
		}

		GLCall(glGenVertexArrays(1, &fVao));
		GLCall(glBindVertexArray(fVao));
		
		GLCall(glGenBuffers(1, &fUbo));
		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, fUbo));
		GLCall(glBufferData(GL_UNIFORM_BUFFER, sizeof(glm::mat4) * 2, NULL, GL_DYNAMIC_DRAW));
		
		// get uniform struct size
		int fUbosize = 0;
		GLCall(glGetActiveUniformBlockiv(programFaceID, fMatVPID, GL_UNIFORM_BLOCK_DATA_SIZE, &fUbosize));
		// bind UBO to its idx
		GLCall(glBindBufferBase(GL_UNIFORM_BUFFER, 0, fUbo));
		GLCall(glUniformBlockBinding(programFaceID, fMatVPID, 0));


		GLCall(glGenBuffers(1, &fpvbo));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, fpvbo));
		GLCall(glBufferData(GL_ARRAY_BUFFER, face_vertices_for_id_pass.size() * sizeof(glm::vec3), glm::value_ptr(face_vertices_for_id_pass[0]), GL_STATIC_DRAW));
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));

		GLCall(glGenBuffers(1, &fvbo));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, fvbo));
		GLCall(glBufferData(GL_ARRAY_BUFFER, vertex_face_ids.size() * sizeof(unsigned int), &vertex_face_ids[0], GL_STATIC_DRAW));
		GLCall(glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, 0, 0));
		GLCall(glEnableVertexAttribArray(1));

		GLCall(glBindBuffer(GL_UNIFORM_BUFFER, 0));
		GLCall(glBindBuffer(GL_ARRAY_BUFFER, 0));
		GLCall(glBindVertexArray(0));
		
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
}