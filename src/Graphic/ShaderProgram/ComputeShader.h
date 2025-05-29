#pragma once
#include "ShaderProgram.h"
#include <GL/glew.h>

class ComputeShader: public ShaderProgram
{
private:
	unsigned int numGroupsX;
	unsigned int numGroupsY;
	unsigned int numGroupsZ; 

	const GLchar* ReadShader(const char* filename);

public:
	ComputeShader(): ShaderProgram() {}
	ComputeShader(const char* shaderFile, unsigned int x, unsigned int y, unsigned int z);
	~ComputeShader() {}

	void setGroupAmount(unsigned int x, unsigned int y, unsigned int z);
	GLuint load(const char* shaderFile) override;

	void use() const override;
	void compute(unsigned int fromIndex = 0, unsigned int numParticlesToCompute = (unsigned int)-1);
};