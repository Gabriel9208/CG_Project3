#include "ComputeShader.h"
#include "../../Utilty/Error.h"

const GLchar* ComputeShader::ReadShader(const char* filename)
{
	FILE* in = fopen(filename, "rb");

	if (!in) {
		std::cout << "Shader file: " << filename << " cannot be opened." << std::endl;
		return NULL;
	}

	fseek(in, 0, SEEK_END);
	int length = ftell(in);
	rewind(in);

	GLchar* shaderSource = new GLchar[length + 1];

	fread(shaderSource, 1, length, in);
	fclose(in);

	shaderSource[length] = '\0';

	return shaderSource;
}

ComputeShader::ComputeShader(const char* shaderFile, unsigned int x, unsigned int y, unsigned int z):
	numGroupsX(x),
	numGroupsY(y),
	numGroupsZ(z)
{
	load(shaderFile);
}

void ComputeShader::setGroupAmount(unsigned int x, unsigned int y, unsigned int z)
{
	numGroupsX = x;
	numGroupsY = y;
	numGroupsZ = z;
}

GLuint ComputeShader::load(const char* shaderFile)
{
	if (shaderFile == nullptr)
	{
		return 0;
	}

	GLCall(program = glCreateProgram());

	const GLchar* source = ReadShader(shaderFile);

	if (source == NULL)
	{
		return 0;
	}

	GLCall(GLuint shader = glCreateShader(GL_COMPUTE_SHADER));
	GLCall(glShaderSource(shader, 1, &source, NULL));
	GLCall(glCompileShader(shader));

	int compileResult;
	GLCall(glGetShaderiv(shader, GL_COMPILE_STATUS, &compileResult));
	if (compileResult == GL_FALSE)
	{
		std::cout << "Shader: \"" << shaderFile << "\" compile failed." << std::endl;
	}

	GLCall(glAttachShader(program, shader));

	delete[] source;	

	GLCall(glLinkProgram(program));

	GLint linkResult;
	GLCall(glGetProgramiv(program, GL_LINK_STATUS, &linkResult));
	if (!linkResult)
	{
		if (linkResult == GL_FALSE)
		{
			std::cout << "Shader: \"" << shaderFile << "\" link failed." << std::endl;
		}

		return 0;
	}

	return program;
}

void ComputeShader::use() const
{
	GLCall(glUseProgram(program));
}

void ComputeShader::compute(unsigned int fromIndex /*= 0*/, unsigned int numParticlesToCompute /*= (unsigned int)-1*/)
{	
	if (numParticlesToCompute == (unsigned int)-1)
	{
		GLCall(glDispatchCompute(numGroupsX, numGroupsY, numGroupsZ));
	}
	else
	{
		unsigned int totalThreadsNeeded = fromIndex + numParticlesToCompute;
		unsigned int dispatchX = (unsigned int)std::ceil((float)totalThreadsNeeded / 256.0f);
		if (totalThreadsNeeded > 0 && dispatchX == 0)
		{
			dispatchX = 1;
		}
		GLCall(glDispatchCompute(dispatchX, 1, 1));
	}
}
