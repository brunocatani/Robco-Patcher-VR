#ifndef EXPLOSION_H
#define EXPLOSION_H

#include "PCH.h"
#include "dirent.h"
#include "gameforms.h"
#include "utility.h"

namespace EXPLOSION
{
	struct line_content
	{
		std::vector<std::string> objects;
		std::vector<std::string> objectExcluded;
		std::string damage{ "none" };
		std::string force{ "none" };
		std::string innerRadius{ "none" };
		std::string outerRadius{ "none" };
		std::string imageSpaceRadius{ "none" };
		std::string projectileSpread{ "none" };
		std::string projectileCount{ "none" };
		std::string spawnProjectile{ "none" };
		std::string soundLevel{ "none" };
		std::string staggerMagnitude{ "none" };
		std::string fullName{ "none" };
	};

	line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void* readConfig(const std::string& folder);
	void* patch(line_content line, RE::BGSExplosion* curobj);
}

#endif
