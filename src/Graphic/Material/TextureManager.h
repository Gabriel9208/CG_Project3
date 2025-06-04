#pragma once
#include "Texture.h"

#include <map>
#include <vector>
#include <glm/glm.hpp>

class TextureManager
{
private:
	static TextureManager instance;

	std::vector<std::string> nameList;
	std::map<std::string, Texture> textures;

	std::vector<std::string> unitOccupied;
	std::map<std::string, int> textureUnit;

	int MAXUNIT; 


	TextureManager();
	~TextureManager() {}


public:
	static TextureManager& getInstance();

	void init();
	void registerTexture(const char* path, std::string texName);
	void use(GLuint programID, std::string samplerName, std::string name);
	
	inline std::vector<std::string> getTextureList() { return nameList; }
};

