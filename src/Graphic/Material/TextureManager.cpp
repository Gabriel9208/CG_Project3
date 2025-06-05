#include "TextureManager.h"
#include "../../Utilty/Error.h"

#include <iostream>

TextureManager TextureManager::instance;

void TextureManager::init()
{
	GLCall(glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, &MAXUNIT));
	for (int i = 0; i < MAXUNIT; i++)
	{
		unitOccupied.push_back("None");
	}
}

void TextureManager::registerTexture(const char* path, std::string texName)
{
	if (textures.find(texName) == textures.end())
	{
		nameList.emplace_back(texName);
		textures[texName] = std::move(Texture());
		textures[texName].LoadTexture(path);
	}
}

void TextureManager::use(GLuint programID, std::string samplerName, std::string name)
{
	if (textures.find(name) == textures.end())
	{
		std::cout << "Texture with name \"" << name << "\" do not exist.\n";
		return;
	}

	if (textureUnit.find(name) != textureUnit.end() && unitOccupied[textureUnit[name]] == name)
	{
		textures[name].bind(textureUnit[name]);
		GLCall(glUniform1i(glGetUniformLocation(programID, samplerName.data()), textureUnit[name]));
		return;
	}

	int firstNoneIdx = -1;
	for (int i = 0; i < MAXUNIT; i++)
	{
		if (unitOccupied[i] == name)
		{
			textures[name].bind(i);
			textureUnit[name] = i;
			GLCall(glUniform1i(glGetUniformLocation(programID, samplerName.data()), textureUnit[name]));
			return;
		}

		if (unitOccupied[i] == "None")
		{
			firstNoneIdx = i;
		}
	}

	if (firstNoneIdx != -1)
	{
		textures[name].bind(firstNoneIdx);
		textureUnit[name] = firstNoneIdx;
		unitOccupied[firstNoneIdx] = name;
		GLCall(glUniform1i(glGetUniformLocation(programID, samplerName.data()), textureUnit[name]));
		return;
	}

	unitOccupied[0] = name;
	textures[name].bind(0);
	textureUnit[name] = 0;
	GLCall(glUniform1i(glGetUniformLocation(programID, samplerName.data()), textureUnit[name]));
}

TextureManager::TextureManager()
{
}

TextureManager& TextureManager::getInstance()
{
	return instance;
}
