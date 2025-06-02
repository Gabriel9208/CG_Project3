#pragma once
#include "Texture.h"

class ColorTexture : public Texture
{
private:
	unsigned int width;
	unsigned int height;

public:
	ColorTexture() : Texture(), width(0), height(0) {}
	~ColorTexture() {};

	void setup(unsigned int width, unsigned int height);
	void resize(unsigned int width, unsigned int height);
};
