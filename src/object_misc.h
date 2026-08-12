#ifndef MISC_H
#define MISC_H

using namespace std;

#	include "PCH.h"
#	include "dirent.h"
#	include "gameforms.h"
#	include "utility.h"
#	include <fstream>
#	include <iostream>
#	include <regex>
#	include <string>
#	include <vector>

namespace MISC
{
	struct line_content
	{
		std::vector<std::string> objects;

		std::vector<std::string> keywords;
		std::vector<std::string> keywordsOr;
		std::vector<std::string> keywordsExcluded;
		std::string weight;
		std::string weightMultiply;
		std::string capsvalue;
		std::string valueMult;
		std::string fullName;
		std::vector<std::string> keywordsToAdd;
		std::vector<std::string> keywordsToRemove;
		std::string filterByHasComponent;
		std::string filterByHasNoComponent;
		std::vector<std::string> modNames;

	};

	struct line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void readConfig(const std::string& folder);
	void patch(const MISC::line_content& line, RE::TESObjectMISC* curobj);
}

#endif
