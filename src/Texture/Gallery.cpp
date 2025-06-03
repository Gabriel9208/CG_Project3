#include "Gallery.h"
#include "TexturePainter.h"

#include <iostream>

namespace CG
{
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

}