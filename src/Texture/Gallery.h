#pragma once
#include "../Mesh/MyMesh.h"

#include <vector>
#include <string>
#include <map>
#include <set>

namespace CG
{
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
		std::vector<Appearance> appearances;
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
		void addAppearance(std::string styleName, const Appearance& app);
		void enlargeAppearance(std::string styleName, unsigned int app_idx, const Appearance& app);
		void renderStyle(std::string styleName);

		inline std::vector<Style>& getStyles() { return styles; }
		inline bool findStyle(std::string name) { return nameIdxMap.find(name) != nameIdxMap.end(); }
	};
}