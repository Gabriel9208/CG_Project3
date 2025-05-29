#pragma once
#include "ShaderProgram.h"
#include <string>
#include <GL/glew.h>


class GraphicShader: public ShaderProgram
{
public:
	GraphicShader() : ShaderProgram() {}
	~GraphicShader();

	GLuint load(ShaderInfo* shaders) override;

	void use() const override;
};