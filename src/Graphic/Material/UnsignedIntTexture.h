#pragma once
#include "Texture.h"

class UnsignedIntTexture : public Texture
{
private:
	unsigned int width;
	unsigned int height;

public:
	UnsignedIntTexture() : Texture(), width(0), height(0) {}
	~UnsignedIntTexture() {};
	void setup(unsigned int width, unsigned int height);

};
