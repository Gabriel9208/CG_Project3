#include "Gallery.h"
#include "TexturePainter.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <algorithm> 


namespace CG
{
	std::string lstrip(std::string s) {
		s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
			return !std::isspace(ch);
			}));
		return s;
	}

	std::string rstrip(std::string s) {
		s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
			return !std::isspace(ch);
			}).base(), s.end());
		return s;
	}

	std::string strip(std::string s) {
		return lstrip(rstrip(std::move(s))); // Or rstrip(lstrip(std::move(s)));
	}

	Gallery Gallery::instance;

	Gallery& Gallery::getInstance()
	{
		return instance;
	}

	void Gallery::registerStyle(std::string styleName)
	{
		if (nameIdxMap.find(styleName) == nameIdxMap.end())
		{
			styles.emplace_back();
			nameIdxMap[styleName] = styles.size() - 1;
			styles[styles.size() - 1].name = styleName;
		}
	}

	void Gallery::registerStyle(Style style)
	{
		if (nameIdxMap.find(style.name) == nameIdxMap.end())
		{
			styles.emplace_back(style);
			nameIdxMap[style.name] = styles.size() - 1;
			styles[styles.size() - 1].name = style.name;
		}
	}

	void Gallery::addAppearance(std::string styleName, const Appearance& app)
	{
		if (nameIdxMap.find(styleName) == nameIdxMap.end())
		{
			return;
		}

		// check if have the same texture name in all the appearance under this style
		for (int i = 0; i < styles[nameIdxMap[styleName]].appearances.size(); i++)
		{
			if (app.textureName == styles[nameIdxMap[styleName]].appearances[i].textureName)
			{
				enlargeAppearance(styleName, i, app);
				return;
			}
		}

		styles[nameIdxMap[styleName]].appearances.emplace_back(app);
	}

	void Gallery::enlargeAppearance(std::string styleName, unsigned int app_idx, const Appearance& app)
	{
		if (app.UVSets.size() != app.faceIDs.size())
		{
			std::cout << "Gallery error: Miss match size of UVSets and faceIDs.\n";
			return;
		}

		for (unsigned int i = 0; i < app.UVSets.size(); i++)
		{
			styles[nameIdxMap[styleName]].appearances[app_idx].UVSets.emplace_back(app.UVSets[i]);
			styles[nameIdxMap[styleName]].appearances[app_idx].faceIDs.emplace_back(app.faceIDs[i]);

		}
	}

	void Gallery::renderStyle(std::string styleName)
	{
		TexturePainter& tp = TexturePainter::getInstance();
		tp.update(&styles[nameIdxMap[styleName]]);
	}

	void Gallery::importFromFile(std::string inFileName)
	{
		std::ifstream inFile((std::string("../../res/styles/") + std::string(inFileName)).c_str());
		Style style;

		std::string line;
		while (inFile)
		{
			std::getline(inFile, line);
			if (line.find("Name") == std::string::npos)
			{
				std::cout << "Reading style file error: " << "No name specified.\n";
				return;
			}
			std::getline(inFile, line);
			style.name = strip(line);
			std::getline(inFile, line);
			if (line.find("-Name") == std::string::npos)
			{
				std::cout << "Reading style file error: " << "Wrong format, missing keyword \"-Name\".\n";
				return;
			}

			while (std::getline(inFile, line))
			{
				if (!inFile)
				{
					break;
				}

				if (line.find("Appearance") == std::string::npos)
				{
					std::cout << "Reading style file error: " << "Wrong format, missing keyword \"Appearance\".\n";
					return;
				}
				style.appearances.emplace_back();

				int currentAppearanceIdx = style.appearances.size() - 1;

				std::getline(inFile, line);
				if (line.find("Texture") == std::string::npos)
				{
					std::cout << "Reading style file error: " << "Wrong format, missing keyword \"Texture\".\n";
					return;
				}
				std::getline(inFile, line);
				style.appearances[currentAppearanceIdx].textureName = strip(line);

				std::getline(inFile, line);
				if (line.find("-Texture") == std::string::npos)
				{
					std::cout << "Reading style file error: " << "Wrong format, missing keyword \"-Texture\".\n";
					return;
				}

				while (std::getline(inFile, line))
				{
					if (line.find("-Appearance") != std::string::npos)
					{
						break;
					}

					if (line.find("FaceID") == std::string::npos)
					{
						std::cout << "Reading style file error: " << "Wrong format, missing keyword \"FaceID\".\n";
						return;
					}

					std::set<unsigned int> faceid;
					while (std::getline(inFile, line))
					{
						if (line.find("-FaceID") != std::string::npos)
						{
							break;
						}

						faceid.insert(std::stoul(strip(line)));
					}
					style.appearances[currentAppearanceIdx].faceIDs.emplace_back(faceid);

					std::getline(inFile, line);
					if (line.find("UVSet") == std::string::npos)
					{
						std::cout << "Reading style file error: " << "Wrong format, missing keyword \"UVSet\".\n";
						return;
					}

					UVSet uvs;
					std::vector<unsigned int> heid;
					std::vector<OpenMesh::Vec2d> uvCoord;

					std::getline(inFile, line);
					if (line.find("HEId") == std::string::npos)
					{
						std::cout << "Reading style file error: " << "Wrong format, missing keyword \"HEId\".\n";
						return;
					}
					while (std::getline(inFile, line))
					{
						if (line.find("-HEId") != std::string::npos)
						{
							break;
						}

						heid.push_back(std::stoul(strip(line)));
					}

					std::getline(inFile, line);
					if (line.find("UV") == std::string::npos)
					{
						std::cout << "Reading style file error: " << "Wrong format, missing keyword \"HEId\".\n";
						return;
					}
					while (std::getline(inFile, line))
					{
						if (line.find("-UV") != std::string::npos)
						{
							break;
						}
						std::stringstream ss(strip(line));
						double x, y;
						ss >> x >> y;
						uvCoord.push_back(OpenMesh::Vec2d(x, y));
					}

					uvs.heIDs = heid;
					uvs.UVs = uvCoord;
					style.appearances[currentAppearanceIdx].UVSets.emplace_back(uvs);

					std::getline(inFile, line);
					if (line.find("-UVSet") == std::string::npos)
					{
						std::cout << "Reading style file error: " << "Wrong format, missing keyword \"-UVSet\".\n";
						return;
					}
				}				

			}
		}
		registerStyle(style);
	}

	void Gallery::exportToFile(std::string styleName, std::string outFileName)
	{
		Style& style = styles[nameIdxMap[styleName]];
		std::ofstream outFile((std::string("../../res/styles/") + std::string(outFileName)).c_str());

		outFile << "Name\n";
		outFile << "	" << style.name << "\n";
		outFile << "-Name\n";

		for (auto app = style.appearances.begin(); app != style.appearances.end(); app++)
		{
			outFile << "Appearance\n";
			outFile << "	" << "Texture\n";
			outFile << "	" << "	" << app->textureName << "\n";
			outFile << "	" << "-Texture\n";

			for (unsigned int setCount = 0; setCount < app->UVSets.size(); setCount++)
			{
				outFile << "	" << "FaceID\n";
				for (auto id = app->faceIDs[setCount].begin(); id != app->faceIDs[setCount].end(); id++)
				{
					outFile << "	" << "	" << *id << "\n";
				}
				outFile << "	" << "-FaceID\n";

				outFile << "	" << "UVSet\n";
				outFile << "	" << "	" << "HEId" << "\n";
				for (auto id = app->UVSets[setCount].heIDs.begin(); id != app->UVSets[setCount].heIDs.end(); id++)
				{
					outFile << "	" << "	" << "	" << *id << "\n";
				}
				outFile << "	" << "	" << "-HEId" << "\n";

				outFile << "	" << "	" << "UV" << "\n";
				for (auto id = app->UVSets[setCount].UVs.begin(); id != app->UVSets[setCount].UVs.end(); id++)
				{
					outFile << "	" << "	" << "	" << *id << "\n";
				}
				outFile << "	" << "	" << "-UV" << "\n";
				outFile << "	" << "-UVSet\n";

			}
			outFile << "-Appearance\n";

		}
		outFile.close();
	}

	std::vector<std::string> Gallery::getStyleList()
	{
		std::vector<std::string> v;
		for (auto itr = nameIdxMap.begin(); itr != nameIdxMap.end(); itr++)
		{
			v.emplace_back(itr->first);
		}

		return v;
	}

}