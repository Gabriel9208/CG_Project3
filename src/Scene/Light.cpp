#include "Light.h"
#include "../Utilty/Error.h"


void Light::initialize()
{
	// Calculate size with proper alignment
	// float (4 bytes) + padding (12 bytes) + vec3 (12 bytes) = 28 bytes
	// Round up to next multiple of 16 (std140 requires 16-byte alignment)
	unsigned int size = 32; // 16 * 2
	
	ubo.initialize(size);
	
	// Fill in data with proper offsets
	GLCall(ubo.fillInData(0, sizeof(float), &shininess));
	// Skip padding (4 bytes after float)
	GLCall(ubo.fillInData(16, sizeof(glm::vec3), &color));
}

void Light::bind(GLuint programId)
{
	GLCall(int lightLoc = glGetUniformLocation(programId, "u_LightPosition"));
	GLCall(glUniform3fv(lightLoc, 1, &position[0]));

	ubo.associateWithShaderBlock(programId, "u_Light", 1);
}

void Light::setPosition(glm::vec3 pos)
{
	position = pos;
}

void Light::setShininess(float shine)
{
	if (shine < 0)
	{
		return;
	}

	shininess = shine;
	GLCall(ubo.fillInData(0, sizeof(float), &shininess));
}

void Light::setColor(glm::vec3 _color)
{
	if (_color[0] < 0 || 
		_color[1] < 0 || 
		_color[2] < 0 ||
		_color[0] > 1 ||
		_color[1] > 1 ||
		_color[2] > 1)
	{
		return;
	}

	color = _color;
	GLCall(ubo.fillInData(16, sizeof(glm::vec3), &color));
}
