#include "UnsignedIntTexture.h"
#include "../../Utilty/Error.h"

void UnsignedIntTexture::setup(unsigned int width, unsigned int height)
{
	GLCall(glGenTextures(1, &id));
	GLCall(glBindTexture(GL_TEXTURE_2D, id));
	GLCall(glTexImage2D(GL_TEXTURE_2D, 0, GL_R32UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_INT, NULL));
	GLCall(glTextureParameteri(id, GL_TEXTURE_MIN_FILTER, GL_NEAREST));
	GLCall(glTextureParameteri(id, GL_TEXTURE_MAG_FILTER, GL_NEAREST));
}