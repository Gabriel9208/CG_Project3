#pragma once
#include "Material.h"

#include <map>
#include <vector>
#include <glm/glm.hpp>

class MaterialManager
{
private:
	static MaterialManager instance;

	std::map<std::string, std::string> mtlNameToStrKey;
	std::map<std::string, Material> strKeyToMtl;

	MaterialManager() {}
	~MaterialManager() {}

	std::string parseKey(const MaterialKey& key);

public:
	static MaterialManager& getInstance();

	void registerMaterial(const char* path, const char* prefix);
	void use(GLuint program, const std::string& name, const std::vector<int>& units = std::vector<int>(0));
	bool LoadMTL(const char* path,
		std::vector<glm::vec3>& Kd,
		std::vector<glm::vec3>& Ka,
		std::vector<glm::vec3>& Ks,
		std::vector<std::string>& out_name,
		std::vector<std::string>& map_Kd,
		std::vector<std::string>& map_Ns,
		std::vector<std::string>& map_refl,
		std::vector<std::string>& map_Ke,
		std::vector<std::string>& map_d,
		std::vector<std::string>& map_Bump
	);

};

