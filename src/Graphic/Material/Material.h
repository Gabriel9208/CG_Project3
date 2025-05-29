#pragma once
#include "Texture.h"

#include <glm/glm.hpp>

struct MaterialKey
{
	glm::vec3 Ka;
	glm::vec3 Kd;
	glm::vec3 Ks;
	std::string map_Kd;
	std::string map_Ns;
	std::string map_refl;
	std::string map_Ke;
	std::string map_d;
	std::string map_Bump;
};

class Material
{
private:
	MaterialKey key;
	std::string strKey;

	Texture baseColor;

public:
	
	Material();
	Material(const MaterialKey& _key, const std::string _strKey);

	Material(Material&& other) noexcept;
	Material(const Material& other) = delete;
	~Material() {}

	Material& operator=(Material&& other) noexcept;
	Material& operator=(const Material& other) = delete;

	void use(GLuint program);
	
	inline const MaterialKey& getMaterial() { return key; }
};