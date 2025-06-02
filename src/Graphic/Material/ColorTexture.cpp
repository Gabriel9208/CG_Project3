#include "ColorTexture.h"
#include "../../Utilty/Error.h"

void ColorTexture::setup(unsigned int width, unsigned int height)
{
	GLCall(glGenTextures(1, &id));
	GLCall(glBindTexture(GL_TEXTURE_2D, id));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
	GLCall(glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}

void ColorTexture::resize(unsigned int width, unsigned int height)
{
	GLCall(glBindTexture(GL_TEXTURE_2D, id));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
	GLCall(glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}
