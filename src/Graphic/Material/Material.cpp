#include "Material.h"
#include "../../Utilty/Error.h"

Material::Material()
{
	key.Ka = glm::vec3(0, 0, 0);
	key.Kd = glm::vec3(0, 0, 0);
	key.Ks = glm::vec3(0, 0, 0);
	key.map_Kd = "";
	key.map_Ns = "";
	key.map_refl = "";
	key.map_Ke = "";
	key.map_d = "";
	key.map_Bump = "";

	strKey = "";
}

Material::Material(const MaterialKey& _key, const std::string _strKey) : baseColor()
{
	key.Ka = _key.Ka;
	key.Kd = _key.Kd;
	key.Ks = _key.Ks;
	key.map_Kd = _key.map_Kd;
	key.map_Ns = _key.map_Ns;
	key.map_refl = _key.map_refl;
	key.map_Ke = _key.map_Ke;
	key.map_d = _key.map_d;
	key.map_Bump = _key.map_Bump;

	strKey = _strKey;

	baseColor.LoadTexture(key.map_Kd);
}

Material::Material(Material&& other) noexcept:
	key(other.key),
	strKey(other.strKey),
	baseColor(std::move(other.baseColor))
{}

Material& Material::operator=(Material&& other) noexcept
{
	if (this != &other)
	{
		key = other.key;
		strKey = other.strKey;
		baseColor = std::move(other.baseColor);
	}

	return *this;
}

void Material::use(GLuint program)
{
	baseColor.bind(0);
	GLuint TextureID = glGetUniformLocation(program, "u_Texture");
	GLCall(glUniform1i(TextureID, 0));

	GLCall(GLuint M_KaID = glGetUniformLocation(program, "u_Material.Ka"));
	GLCall(GLuint M_KdID = glGetUniformLocation(program, "u_Material.Kd"));
	GLCall(GLuint M_KsID = glGetUniformLocation(program, "u_Material.Ks"));
	GLCall(glUniform3fv(M_KaID, 1, &key.Ka[0]));
	GLCall(glUniform3fv(M_KdID, 1, &key.Kd[0]));
	GLCall(glUniform3fv(M_KsID, 1, &key.Ks[0]));
}

