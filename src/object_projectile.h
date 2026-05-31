#ifndef PROJECTILE_H
#define PROJECTILE_H

#include "PCH.h"
#include "dirent.h"
#include "gameforms.h"
#include "utility.h"

namespace PROJECTILE
{
	struct line_content
	{
		std::vector<std::string> objects;
		std::vector<std::string> objectExcluded;
		std::string speed{ "none" };
		std::string range{ "none" };
		std::string gravity{ "none" };
		std::string force{ "none" };
		std::string coneSpread{ "none" };
		std::string collisionRadius{ "none" };
		std::string lifetime{ "none" };
		std::string explosionType{ "none" };
		std::string vatsProjectile{ "none" };
		std::string soundLevel{ "none" };
		std::string fullName{ "none" };
	};

	line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void* readConfig(const std::string& folder);
	void* patch(line_content line, RE::BGSProjectile* curobj);
}

#endif
