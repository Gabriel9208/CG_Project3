#pragma once
#include "../Graphic/VAO.h"
#include "../Graphic/VBO.h"
#include "../Graphic/UBO.h"
#include "../Graphic/ShaderProgram/GraphicShader.h"
#include "../Graphic/FBO.h"
#include "../Graphic/Material/Texture.h"
#include "../Mesh/MyMesh.h"

#include <glm/glm.hpp>

namespace CG
{
	class TexturePainter
	{
	private:
		static TexturePainter instance;

		MyMesh* referenceMesh;

		GraphicShader* program;
		VAO* tVAO;
		VBO<glm::vec3>* tVBOp;
		VBO<glm::vec3>* tVBOn;
		VBO<glm::vec2>* tVBOu;
		UBO* tUBO;

		GLuint tModelID;
		GLuint tMatKaID;
		GLuint tMatKdID;
		GLuint tMatKsID;
		Texture* texture;
		GLuint baseTexture;
		GLuint decalFBO;

		glm::mat4 model;
		glm::vec3 colorAmbient;
		glm::vec3 colorDiffuse;
		glm::vec3 colorSpecular;
		glm::vec3 colorLine;

		unsigned int drawCount;

		TexturePainter();
		~TexturePainter(){}


	public:
		static TexturePainter& getInstance();

		void init(int display_w, int display_h);
		void update(MyMesh* mesh);
		void updateUV();

		void render(const glm::mat4 proj, const glm::mat4 view);

	};
}
