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

	void Gallery::updataSaveTextureDatas(std::string styleName, const std::vector<TextureData>& tds)
	{
		if (nameIdxMap.find(styleName) == nameIdxMap.end())
		{
			return;
		}

		styles[nameIdxMap[styleName]].saveTextureDatas = tds;
	}

	/*void Gallery::enlargeAppearance(std::string styleName, unsigned int app_idx, const Appearance& app)
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
	}*/

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

				if (line.find("TextureData") == std::string::npos)
				{
					std::cout << "Reading style file error: " << "Wrong format, missing keyword \"TextureData\".\n";
					return;
				}
				style.saveTextureDatas.emplace_back();

				int currentAppearanceIdx = style.saveTextureDatas.size() - 1;

				std::getline(inFile, line);
				if (line.find("Texture") == std::string::npos)
				{
					std::cout << "Reading style file error: " << "Wrong format, missing keyword \"Texture\".\n";
					return;
				}
				std::getline(inFile, line);
				style.saveTextureDatas[currentAppearanceIdx].textureName = strip(line);

				std::getline(inFile, line);
				if (line.find("-Texture") == std::string::npos)
				{
					std::cout << "Reading style file error: " << "Wrong format, missing keyword \"-Texture\".\n";
					return;
				}

				while (std::getline(inFile, line))
				{
					if (line.find("-TextureData") != std::string::npos)
					{
						break;
					}

					if (line.find("Position") == std::string::npos)
					{
						std::cout << "Reading style file error: " << "Wrong format, missing keyword \"Position\".\n";
						return;
					}

					while (std::getline(inFile, line))
					{
						if (line.find("-Position") != std::string::npos)
						{
							break;
						}

						std::stringstream ss(strip(line));
						float x, y, z;
						ss >> x >> y >> z;
						style.saveTextureDatas[currentAppearanceIdx].positions.push_back({x, y, z});
					}

					std::getline(inFile, line);
					if (line.find("FaceNormals") == std::string::npos)
					{
						std::cout << "Reading style file error: " << "Wrong format, missing keyword \"FaceNormals\".\n";
						return;
					}

					while (std::getline(inFile, line))
					{
						if (line.find("-FaceNormals") != std::string::npos)
						{
							break;
						}

						std::stringstream ss(strip(line));
						float x, y, z;
						ss >> x >> y >> z;
						style.saveTextureDatas[currentAppearanceIdx].faceNormals.push_back({ x, y, z });
					}

					std::getline(inFile, line);
					if (line.find("UVCoords") == std::string::npos)
					{
						std::cout << "Reading style file error: " << "Wrong format, missing keyword \"UVCoords\".\n";
						return;
					}

					while (std::getline(inFile, line))
					{
						if (line.find("-UVCoords") != std::string::npos)
						{
							break;
						}

						std::stringstream ss(strip(line));
						float x, y;
						ss >> x >> y;
						style.saveTextureDatas[currentAppearanceIdx].uvCoords.push_back({ x, y });
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

		for (auto app = style.saveTextureDatas.begin(); app != style.saveTextureDatas.end(); app++)
		{
			outFile << "TextureData\n";
			outFile << "	" << "Texture\n";
			outFile << "	" << "	" << app->textureName << "\n";
			outFile << "	" << "-Texture\n";

			outFile << "	" << "Position\n";
			for (int i = 0; i < app->positions.size(); i++)
			{
				outFile << "	" << "	" << app->positions[i][0] << " " << app->positions[i][1] << " " << app->positions[i][2] << "\n";
			}
			outFile << "	" << "-Position\n";

			outFile << "	" << "FaceNormals\n";
			for (int i = 0; i < app->faceNormals.size(); i++)
			{
				outFile << "	" << "	" << app->faceNormals[i][0] << " " << app->faceNormals[i][1] << " " << app->faceNormals[i][2] << "\n";
			}
			outFile << "	" << "-FaceNormals\n";

			outFile << "	" << "UVCoords\n";
			for (int i = 0; i < app->uvCoords.size(); i++)
			{
				outFile << "	" << "	" << app->uvCoords[i][0] << " " << app->uvCoords[i][1] << "\n";
			}
			outFile << "	" << "-UVCoords\n";

			outFile << "-TextureData\n";

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