#ifndef OUTFIT_H
#define OUTFIT_H

#include "PCH.h"
#include "dirent.h"
#include "gameforms.h"
#include "utility.h"

namespace OUTFIT
{
	struct line_content
	{
		std::vector<std::string> objects;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> itemsToAdd;
		std::vector<std::string> itemsToRemove;
		std::vector<std::pair<std::string, std::string>> itemsToReplace;
		std::string clear{ "none" };
	};

	line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void* readConfig(const std::string& folder);
	void* patch(line_content line, RE::BGSOutfit* curobj);
}

#endif
