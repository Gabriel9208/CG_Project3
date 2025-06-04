#pragma once
#include "../Graphic/VAO.h"
#include "../Graphic/VBO.h"
#include "../Graphic/UBO.h"
#include "../Graphic/ShaderProgram/GraphicShader.h"
#include "../Graphic/FBO.h"
#include "../Mesh/MyMesh.h"
#include "Gallery.h"

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
		GLuint baseTexture;
		GLuint decalFBO;

		glm::mat4 model;
		glm::vec3 colorAmbient;
		glm::vec3 colorDiffuse;
		glm::vec3 colorSpecular;
		glm::vec3 colorLine;

		std::vector<glm::vec2> uvCoords;
		std::vector<unsigned int> heIdx;
		
		// for rendering style, multiple texture
		std::vector<unsigned int> drawCount;
		std::vector<std::string> textureName;

		TexturePainter();
		~TexturePainter(){}


	public:
		static TexturePainter& getInstance();

		void init(int display_w, int display_h);
		void update(std::string _textureName, MyMesh* mesh); // directly take data from texture mapper and face picker
		void update(Style* style);
		void updateUV();

		void render(const glm::mat4 proj, const glm::mat4 view);
		inline std::vector<glm::vec2>& getUVCoords() { return uvCoords; }
		inline std::vector<unsigned int> getHeIdx() { return heIdx; }
	};
}
