#pragma once
#include "../Mesh/MyMesh.h"

#include <vector>
#include <string>
#include <map>
#include <set>

namespace CG
{
	struct TextureData
	{
		std::string textureName;
		std::vector<glm::vec3> positions;
		std::vector<glm::vec3> faceNormals;
		std::vector<glm::vec2> uvCoords;

		void clear()
		{
			textureName = "";
			positions.clear();
			faceNormals.clear();
			uvCoords.clear();
		}
	};

	struct UVSet
	{
		std::vector<unsigned int> heIDs; // half edge
		std::vector<OpenMesh::Vec2d> UVs;
	};

	struct Appearance
	{
		std::string textureName;
		std::vector<std::set<unsigned int>> faceIDs;
		std::vector<UVSet> UVSets;
	};

	struct Style
	{
		std::string name;
		std::vector<TextureData> saveTextureDatas;
	};

	class Gallery
	{
	private:
		static Gallery instance;

		std::vector<Style> styles;
		std::map<std::string, unsigned int> nameIdxMap; // map names to index of styles vector

		Gallery() {}
		~Gallery() {}

	public:
		static Gallery& getInstance();

		void registerStyle(std::string styleName);
		void registerStyle(Style style);
		void updataSaveTextureDatas(std::string styleName, const std::vector<TextureData>& tds);
		//void enlargeAppearance(std::string styleName, unsigned int app_idx, const Appearance& app);
		void renderStyle(std::string styleName);

		void importFromFile(std::string inFileName);
		void exportToFile(std::string styleName, std::string outFileName);

		inline std::vector<Style>& getStyles() { return styles; }
		inline bool findStyle(std::string name) { return nameIdxMap.find(name) != nameIdxMap.end(); }
		std::vector<std::string> getStyleList();
	};
}