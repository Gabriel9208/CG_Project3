#include "MaterialManager.h"


MaterialManager MaterialManager::instance;

std::string MaterialManager::parseKey(const MaterialKey& key)
{
	std::string strKey = std::to_string(key.Ka[0]) + "|" +
		std::to_string(key.Ka[1]) + "|" +
		std::to_string(key.Ka[2]) + "|" +
		std::to_string(key.Kd[0]) + "|" +
		std::to_string(key.Kd[1]) + "|" +
		std::to_string(key.Kd[2]) + "|" +
		std::to_string(key.Ks[0]) + "|" +
		std::to_string(key.Ks[1]) + "|" +
		std::to_string(key.Ks[2]) + "|" +
		key.map_Kd + "|" +
		key.map_Ns + "|" +
		key.map_refl + "|" +
		key.map_Ke + "|" +
		key.map_d + "|" +
		key.map_Bump;

	return strKey;
}

void MaterialManager::registerMaterial(const char* path, const char* prefix)
{
	std::vector<glm::vec3> Kds;
	std::vector<glm::vec3> Kas;
	std::vector<glm::vec3> Kss;
	std::vector<std::string> Materials; //mtl-name
	std::vector<std::string> map_Kd;
	std::vector<std::string> map_Ns;
	std::vector<std::string> map_refl;
	std::vector<std::string> map_Ke;
	std::vector<std::string> map_d;
	std::vector<std::string> map_Bump;

	LoadMTL(path, Kds, Kas, Kss, Materials, map_Kd, map_Ns, map_refl, map_Ke, map_d, map_Bump);

	for (int i = 0; i < Kds.size(); i++)
	{
		MaterialKey key;
		key.Ka = Kas[i];
		key.Kd = glm::vec3(1, 1, 1); // not in mtl
		key.Ks = Kss[i];
		key.map_Kd = prefix + map_Kd[i];
		key.map_Ns = prefix + map_Ns[i];
		key.map_refl = prefix + map_refl[i];
		key.map_Ke = prefix + map_Ke[i];
		key.map_d = prefix + map_d[i];
		key.map_Bump = prefix + map_Bump[i];

		std::string strKey = parseKey(key);
		if (strKeyToMtl.find(strKey) == strKeyToMtl.end())
		{
			strKeyToMtl[strKey] = Material(key, strKey);
			mtlNameToStrKey[Materials[i]] = strKey;
		}
		else if (mtlNameToStrKey.find(Materials[i]) == mtlNameToStrKey.end())
		{
			mtlNameToStrKey[Materials[i]] = strKey;

		}
	}
}

void MaterialManager::use(GLuint program, const std::string& name, const std::vector<int>& units /*= std::vector<int>(0)*/)
{
	if (mtlNameToStrKey.find(name) != mtlNameToStrKey.end())
	{
		if (strKeyToMtl.find(mtlNameToStrKey[name]) != strKeyToMtl.end())
		{
			strKeyToMtl[mtlNameToStrKey[name]].use(program);
		}
	}
}

MaterialManager& MaterialManager::getInstance()
{
	return instance;
}

bool MaterialManager::LoadMTL(
	const char* path,
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
)
{
	FILE* file = fopen(path, "r");
	if (file == NULL)
	{
		printf("Impossible to open the .mtl file ! Are you in the right path ? See Tutorial 1 for details\n");
		return false;
	}

	while (1)
	{
		char lineHeader[128];

		int result = fscanf(file, "%s", lineHeader);
		if (result == EOF)
			break;

		if (strcmp(lineHeader, "newmtl") == 0)
		{
			char material[50];
			fscanf(file, "%s", material);
			out_name.push_back(material);
		}
		else if (strcmp(lineHeader, "Kd") == 0)
		{
			glm::vec3 diffuse;
			fscanf(file, "%f %f %f\n", &diffuse.x, &diffuse.y, &diffuse.z);
			Kd.push_back(diffuse);
		}
		else if (strcmp(lineHeader, "Ka") == 0)
		{
			glm::vec3 ambient;
			fscanf(file, "%f %f %f\n", &ambient.x, &ambient.y, &ambient.z);
			Ka.push_back(ambient);
		}
		else if (strcmp(lineHeader, "Ks") == 0)
		{
			glm::vec3 specular;
			fscanf(file, "%f %f %f\n", &specular.x, &specular.y, &specular.z);
			Ks.push_back(specular);
		}
		else if (strcmp(lineHeader, "map_Kd") == 0)
		{
			char buffer[256];
			fscanf(file, " %[^\n]", buffer);
			map_Kd.push_back(buffer);
			Kd.push_back(glm::vec3(1, 1, 1));
		}
		else if (strcmp(lineHeader, "map_Ns") == 0)
		{
			char buffer[256];
			fscanf(file, " %[^\n]", buffer);
			map_Ns.push_back(buffer);
		}
		else if (strcmp(lineHeader, "map_refl") == 0)
		{
			char buffer[256];
			fscanf(file, " %[^\n]", buffer);
			map_refl.push_back(buffer);
		}
		else if (strcmp(lineHeader, "map_Ke") == 0)
		{
			char buffer[256];
			fscanf(file, " %[^\n]", buffer);
			map_Ke.push_back(buffer);
		}
		else if (strcmp(lineHeader, "map_d") == 0)
		{
			char buffer[256];
			fscanf(file, " %[^\n]", buffer);
			map_d.push_back(buffer);
		}
		else if (strcmp(lineHeader, "map_Bump") == 0)
		{
			char buffer[256];

			fscanf(file, "%s", &buffer);
			fscanf(file, "%s", &buffer);

			fscanf(file, " %[^\n]", buffer);
			map_Bump.push_back(buffer);
		}
		else
		{
			char trash[1000];
			fgets(trash, 1000, file);
		}
	}
	return true;
}