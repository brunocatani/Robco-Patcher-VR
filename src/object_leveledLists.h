#ifndef LEVELED_LIST_H
#define LEVELED_LIST_H

using namespace std;

#include "PCH.h"
#include "dirent.h"
#include "gameforms.h"
#include "utility.h"
#include <unordered_set>
#include <fstream>
#include <iostream>
#include <regex>
#include <string>
#include <vector>

namespace LEVELEDLISTS
{

	struct struct_ll
	{
		std::uint32_t size;  // engine: count als u32 am blockanfang
		std::uint32_t pad;   // damit ll bei +8 startet
		RE::LEVELED_OBJECT ll[1];
	};

	struct line_content
	{


		std::string chanceRobCo;
		std::string clear;
		std::string calcForLevel;
		std::string calcEachItem;
		std::string calcForLevelAndEachItem;
		std::string calcUseAll;
		
		std::vector<std::string> objects;
		std::vector<std::string> containerObjects;
		std::vector<std::vector<std::string>> addedObjects;
		std::vector<std::vector<std::string>> addedContainerObjects;
		std::vector<std::string> removedContainerObjects;
		std::vector<std::vector<std::string>> removedObjects;
		std::vector<std::vector<std::string>> templateKeyword;
		std::vector<std::string> removeItemsByKeyword;
		std::string noFilterLL;
		std::vector<std::string> modNames;
	};

	struct line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void readConfig(const std::string& folder);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void patch(const LEVELEDLISTS::line_content& line, RE::TESLevItem* curobj);
	void patchContainer(const LEVELEDLISTS::line_content& line, RE::TESObjectCONT* curobj);
}

#endif
