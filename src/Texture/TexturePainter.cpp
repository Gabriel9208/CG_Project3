#include "TexturePainter.h"
#include "FacePicker.h"
#include "TextureMapper.h"
#include "../Utilty/Error.h"
#include "../Graphic/Material/TextureManager.h"

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
		model(glm::mat4(1.0)),
		colorAmbient(glm::vec3(1, 1, 1)),
		colorDiffuse(glm::vec3(1.0, 1.0, 1.0)),
		colorSpecular(glm::vec3(1.0, 1.0, 1.0)),
		colorLine(glm::vec3(0.8, 0.8, 0.8)),
		drawCount(0)
	{
	}

	void TexturePainter::init(int display_w, int display_h, MyMesh* mesh)
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


		ShaderInfo shadersTexture[] = {
				{ GL_VERTEX_SHADER, "../../res/shaders/texture.vp" }, //vertex shader
				{ GL_FRAGMENT_SHADER, "../../res/shaders/texture.fp" }, //fragment shader
				{ GL_NONE, NULL } };

		program->load(shadersTexture);
		program->use();

		tVAO->bind();

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

		referenceMesh = mesh;
	}

	TexturePainter& TexturePainter::getInstance()
	{
		return instance;
	}

	void TexturePainter::update(Style* style)
	{
		TextureMapper& tm = TextureMapper::getInstance();
		FacePicker& fp = FacePicker::getInstance();

		currentTextureData.clear();
		//uvCoords.clear();
		//heIdx.clear();
		//drawCount.clear();
		//textureName.clear();

		saveTextureDatas = style->saveTextureDatas;
	}

	void TexturePainter::update(std::string _textureName, MyMesh* mesh)
	{
		referenceMesh = mesh;
		TextureMapper& tm = TextureMapper::getInstance();
		FacePicker& fp = FacePicker::getInstance();
		auto pickedFaces = fp.getFacesPicked();

		currentTextureData.clear();
		//uvCoords.clear();
		heIdx.clear();
		drawCount.clear();
		textureName.clear();
		
		currentTextureData.textureName = _textureName;
		textureName.push_back(_textureName);
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
				glm::vec3 p[3];

				for (int i = 0; i < 3; ++i)
				{
					auto vh = vhandles[i];
					auto pos = referenceMesh->point(vh);
					p[i] = glm::vec3(pos[0], pos[1], pos[2]);

					currentTextureData.positions.push_back(p[i]);

					auto it = uvMap.find(vh);
					if (it != uvMap.end())
						currentTextureData.uvCoords.emplace_back(it->second[0], it->second[1]);
					else
						currentTextureData.uvCoords.emplace_back(0.0f, 0.0f);
				}

				glm::vec3 edge1 = p[1] - p[0];
				glm::vec3 edge2 = p[2] - p[0];
				glm::vec3 normal = glm::normalize(glm::cross(edge1, edge2));

				currentTextureData.faceNormals.push_back(normal);
				currentTextureData.faceNormals.push_back(normal);
				currentTextureData.faceNormals.push_back(normal);
			}
		}

		if (currentTextureData.positions.size() > 0)
		{
			tVAO->bind();
			tVBOp->bind();
			tVBOp->setData(currentTextureData.positions, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(0));
			tVBOp->unbind();

			tVBOn->bind();
			tVBOn->setData(currentTextureData.faceNormals, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(1));
			tVBOn->unbind();

			tVBOu->bind();
			tVBOu->setData(currentTextureData.uvCoords, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(2));
			tVBOu->unbind();
			tVAO->unbind();

			//glBindFramebuffer(GL_FRAMEBUFFER, decalFBO);
			
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}

		drawCount.emplace_back(currentTextureData.positions.size());
	}

	void TexturePainter::save()
	{
		/*bool isFound = false;
		for (int i = 0; i < saveTextureDatas.size(); i++)
		{
			if (currentTextureData.textureName == saveTextureDatas[i].textureName)
			{
				saveTextureDatas[i].clear();
				saveTextureDatas[i] = currentTextureData;
				isFound = true;
				break;
			}
		}

		if (!isFound)
		{
			saveTextureDatas.push_back(currentTextureData);
		}*/

		saveTextureDatas.push_back(currentTextureData);
	}

	// only update uv
	void TexturePainter::updateUV()
	{
		TextureMapper& tm = TextureMapper::getInstance();
		FacePicker& fp = FacePicker::getInstance();
		auto pickedFaces = fp.getFacesPicked();

		currentTextureData.uvCoords.clear();
		//uvCoords.clear();
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
						currentTextureData.uvCoords.emplace_back(it->second[0], it->second[1]);
					else
						currentTextureData.uvCoords.emplace_back(0.0f, 0.0f);
				}
			}
		}

		if (currentTextureData.uvCoords.size() > 0)
		{
			tVAO->bind();

			tVBOu->bind();
			tVBOu->setData(currentTextureData.uvCoords, GL_DYNAMIC_DRAW);
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

		glDisable(GL_DEPTH_TEST);
		glEnable(GL_POLYGON_OFFSET_FILL);
		glPolygonOffset(-1.0f, -1.0f);

		TextureManager& tmg = TextureManager::getInstance();
		

		for (int i = 0; i < saveTextureDatas.size(); i++)
		{
			tVBOp->bind();
			tVBOp->setData(saveTextureDatas[i].positions, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(0));
			tVBOp->unbind();

			tVBOn->bind();
			tVBOn->setData(saveTextureDatas[i].faceNormals, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(1));
			tVBOn->unbind();

			tVBOu->bind();
			tVBOu->setData(saveTextureDatas[i].uvCoords, GL_DYNAMIC_DRAW);
			GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));
			GLCall(glEnableVertexAttribArray(2));
			tVBOu->unbind();
		
			tmg.use(program->getId(), "Texture", saveTextureDatas[i].textureName);
			GLCall(glDrawArrays(GL_TRIANGLES, 0, saveTextureDatas[i].positions.size()));
		}	

		tVBOp->bind();
		tVBOp->setData(currentTextureData.positions, GL_DYNAMIC_DRAW);
		GLCall(glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(0));
		tVBOp->unbind();

		tVBOn->bind();
		tVBOn->setData(currentTextureData.faceNormals, GL_DYNAMIC_DRAW);
		GLCall(glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(1));
		tVBOn->unbind();

		tVBOu->bind();
		tVBOu->setData(currentTextureData.uvCoords, GL_DYNAMIC_DRAW);
		GLCall(glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0));
		GLCall(glEnableVertexAttribArray(2));
		tVBOu->unbind();

		for (int i = 0; i < drawCount.size(); i++)
		{
			tmg.use(program->getId(), "Texture", textureName[i]);
			GLCall(glDrawArrays(GL_TRIANGLES, 0, drawCount[i]));
		}
		
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	void TexturePainter::clearSaveTextureDatas()
	{
		saveTextureDatas.clear();
	}
}