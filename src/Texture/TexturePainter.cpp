#include "TexturePainter.h"
#include "FacePicker.h"
#include "TextureMapper.h"
#include "../Utilty/Error.h"

namespace CG
{
	TexturePainter TexturePainter::instance;

	TexturePainter::TexturePainter():
		program(nullptr),
		tVAO(nullptr),
		tVBOp(nullptr),
		tVBOn(nullptr),
		tVBOu(nullptr),
		tUBO(nullptr),
		tModelID(-1),
		tMatKaID(-1),
		tMatKdID(-1),
		tMatKsID(-1),
		texture(nullptr),
		model(glm::mat4(1.0)),
		colorAmbient(glm::vec3(1, 1, 1)),
		colorDiffuse(glm::vec3(1.0, 1.0, 1.0)),
		colorSpecular(glm::vec3(1.0, 1.0, 1.0)),
		colorLine(glm::vec3(0.8, 0.8, 0.8)),
		drawCount(0)
	{
	}

	void TexturePainter::init(int display_w, int display_h)
	{
		glGenTextures(1, &baseTexture);
		glBindTexture(GL_TEXTURE_2D, baseTexture);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, display_w, display_h, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		glGenFramebuffers(1, &decalFBO);
		glBindFramebuffer(GL_FRAMEBUFFER, decalFBO);
		glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, baseTexture, 0);


		program = new GraphicShader();
		tVAO = new VAO();
		tVBOp = new VBO<glm::vec3>;
		tVBOn = new VBO<glm::vec3>;
		tVBOu = new VBO<glm::vec2>;
		tUBO = new UBO();
		texture = new Texture();


		ShaderInfo shadersTexture[] = {
				{ GL_VERTEX_SHADER, "../../res/shaders/texture.vp" }, //vertex shader
				{ GL_FRAGMENT_SHADER, "../../res/shaders/texture.fp" }, //fragment shader
				{ GL_NONE, NULL } };

		program->load(shadersTexture);
		program->use();

		tVAO->bind();

		texture->LoadTexture("../../res/texture/test.jpg");

		tModelID = glGetUniformLocation(program->getId(), "Model");
		tMatKaID = glGetUniformLocation(program->getId(), "Material.Ka");
		tMatKdID = glGetUniformLocation(program->getId(), "Material.Kd");
		tMatKsID = glGetUniformLocation(program->getId(), "Material.Ks");

		GLCall(glUniformMatrix4fv(tModelID, 1, GL_FALSE, &model[0][0]));
		GLCall(glUniform3fv(tMatKaID, 1, &colorAmbient[0]));
		GLCall(glUniform3fv(tMatKdID, 1, &colorDiffuse[0]));
		GLCall(glUniform3fv(tMatKsID, 1, &colorSpecular[0]));


		tUBO->initialize(sizeof(glm::mat4) * 2);
		tVBOp->initialize(0, GL_DYNAMIC_DRAW);
		tVBOn->initialize(0, GL_DYNAMIC_DRAW);
		tVBOu->initialize(0, GL_DYNAMIC_DRAW);

		tUBO->associateWithShaderBlock(program->getId(), "MatVP", 0);

		tVAO->unbind();
	}

	TexturePainter& TexturePainter::getInstance()
	{
		return instance;
	}

	void TexturePainter::update(Style* style)
	{
		TextureMapper& tm = TextureMapper::getInstance();
		FacePicker& fp = FacePicker::getInstance();

		std::vector<glm::vec3> positions;
		uvCoords.clear();
		heIdx.clear();
		std::vector<glm::vec3> faceNormals;

		for (auto appearance = style->appearances.begin(); appearance != style->appearances.end(); appearance++)
		{

			for (auto idx = 0; idx < appearance->faceIDs.size(); idx++)
			{
				unsigned int uvIdx = 0;

				auto pickedFaces = appearance->faceIDs[idx];

				//============================
				for (auto faceId = pickedFaces.begin(); faceId != pickedFaces.end(); ++faceId)
				{
					OpenMesh::FaceHandle fh = referenceMesh->face_handle(*faceId);

					std::vector<OpenMesh::VertexHandle> vhandles;

					for (MyMesh::FaceVertexIter fv_it = referenceMesh->fv_iter(fh); fv_it.is_valid(); ++fv_it)
					{
						vhandles.push_back(*fv_it);
					}

					if (vhandles.size() == 3)
					{
						glm::vec3 p[3];

						for (int i = 0; i < 3; ++i)
						{
							auto vh = vhandles[i];
							auto pos = referenceMesh->point(vh);
							p[i] = glm::vec3(pos[0], pos[1], pos[2]);

							positions.push_back(p[i]);

							auto it = appearance->UVSets[idx];
							uvCoords.emplace_back(it.UVs[uvIdx][0], it.UVs[uvIdx][1]);
							uvIdx++;
						}

						glm::vec3 edge1 = p[1] - p[0];
						glm::vec3 edge2 = p[2] - p[0];
						glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

						faceNormals.push_back(normal);
						faceNormals.push_back(normal);
						faceNormals.push_back(normal);
					}
				}
				//===================
			}
		}

		if (positions.size() > 0)
		{
			tVAO->bind();
			tVBOp->bind();
			tVBOp->setData(positions, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(0));
			tVBOp->unbind();

			tVBOn->bind();
			tVBOn->setData(faceNormals, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(1));
			tVBOn->unbind();

			tVBOu->bind();
			tVBOu->setData(uvCoords, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(2));
			tVBOu->unbind();

			program->use();
			texture->bind();
			GLCall(glUniform1i(glGetUniformLocation(program->getId(), "Texture"), 0));
			glBindTexture(GL_TEXTURE_2D, texture->getId());
			program->unUse();

			tVAO->unbind();

			//glBindFramebuffer(GL_FRAMEBUFFER, decalFBO);

			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		drawCount = positions.size();
	}

	void TexturePainter::update(MyMesh* mesh)
	{
		referenceMesh = mesh;
		TextureMapper& tm = TextureMapper::getInstance();
		FacePicker& fp = FacePicker::getInstance();
		auto pickedFaces = fp.getFacesPicked();

		std::vector<glm::vec3> positions;
		uvCoords.clear();
		heIdx.clear();
		std::vector<glm::vec3> faceNormals;

		const auto& uvMap = tm.getAllUVMap();

		for (auto faceId = pickedFaces.begin(); faceId != pickedFaces.end(); ++faceId)
		{
			OpenMesh::FaceHandle fh = referenceMesh->face_handle(*faceId);

			if (!mesh->is_valid_handle(fh)) continue;

			std::vector<OpenMesh::VertexHandle> vhandles;

			for (MyMesh::FaceHalfedgeIter fh_it = referenceMesh->fh_iter(fh); fh_it.is_valid(); fh_it++)
			{
				heIdx.emplace_back(fh_it->idx());
			}

			for (MyMesh::FaceVertexIter fv_it = referenceMesh->fv_iter(fh); fv_it.is_valid(); ++fv_it)
			{
				vhandles.push_back(*fv_it);
			}

			if (vhandles.size() == 3)
			{
				glm::vec3 p[3];

				for (int i = 0; i < 3; ++i)
				{
					auto vh = vhandles[i];
					auto pos = referenceMesh->point(vh);
					p[i] = glm::vec3(pos[0], pos[1], pos[2]);

					positions.push_back(p[i]);

					auto it = uvMap.find(vh);
					if (it != uvMap.end())
						uvCoords.emplace_back(it->second[0], it->second[1]);
					else
						uvCoords.emplace_back(0.0f, 0.0f);
				}

				glm::vec3 edge1 = p[1] - p[0];
				glm::vec3 edge2 = p[2] - p[0];
				glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

				faceNormals.push_back(normal);
				faceNormals.push_back(normal);
				faceNormals.push_back(normal);
			}
		}

		if (positions.size() > 0)
		{
			tVAO->bind();
			tVBOp->bind();
			tVBOp->setData(positions, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(0));
			tVBOp->unbind();

			tVBOn->bind();
			tVBOn->setData(faceNormals, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(1));
			tVBOn->unbind();

			tVBOu->bind();
			tVBOu->setData(uvCoords, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(2));
			tVBOu->unbind();

			program->use();
			texture->bind();
			GLCall(glUniform1i(glGetUniformLocation(program->getId(), "Texture"), 0));
			glBindTexture(GL_TEXTURE_2D, texture->getId());
			program->unUse();

			tVAO->unbind();

			//glBindFramebuffer(GL_FRAMEBUFFER, decalFBO);
			
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		drawCount = positions.size();
	}

	// only update uv
	void TexturePainter::updateUV()
	{
		TextureMapper& tm = TextureMapper::getInstance();
		FacePicker& fp = FacePicker::getInstance();
		auto pickedFaces = fp.getFacesPicked();

		uvCoords.clear();
		heIdx.clear();
		const auto& uvMap = tm.getAllUVMap();

		for (auto faceId = pickedFaces.begin(); faceId != pickedFaces.end(); ++faceId)
		{
			OpenMesh::FaceHandle fh = referenceMesh->face_handle(*faceId);

			if (!referenceMesh->is_valid_handle(fh)) continue;

			std::vector<OpenMesh::VertexHandle> vhandles;

			for (MyMesh::FaceHalfedgeIter fh_it = referenceMesh->fh_iter(fh); fh_it.is_valid(); fh_it++)
			{
				heIdx.emplace_back(fh_it->idx());
			}

			for (MyMesh::FaceVertexIter fv_it = referenceMesh->fv_iter(fh); fv_it.is_valid(); ++fv_it)
			{
				vhandles.push_back(*fv_it);
			}

			if (vhandles.size() == 3)
			{

				for (int i = 0; i < 3; ++i)
				{
					auto vh = vhandles[i];
					auto it = uvMap.find(vh);

					if (it != uvMap.end())
						uvCoords.emplace_back(it->second[0], it->second[1]);
					else
						uvCoords.emplace_back(0.0f, 0.0f);
				}
			}
		}

		if (uvCoords.size() > 0)
		{
			tVAO->bind();

			tVBOu->bind();
			tVBOu->setData(uvCoords, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(2));
			tVBOu->unbind();

			tVAO->unbind();

			//glBindFramebuffer(GL_FRAMEBUFFER, decalFBO);

			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	}

	void TexturePainter::render(const glm::mat4 proj, const glm::mat4 view)
	{
		program->use();
		tVAO->bind();

		tUBO->bind();
		tUBO->fillInData(0, sizeof(glm::mat4), &view);
		tUBO->fillInData(sizeof(glm::mat4), sizeof(glm::mat4), &proj);

		texture->bind();

		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.0f, -1.0f);

		// Draw texture mesh
		GLCall(glDrawArrays(GL_TRIANGLES, 0, drawCount));

		glDisable(GL_POLYGON_OFFSET_FILL);
	}
}