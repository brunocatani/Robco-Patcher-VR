#pragma once

#include "PCH.h"
#include "dirent.h"
#include "utility.h"
#include <fstream>
#include <list>
#include <limits>
#include <type_traits>

namespace EXPLOSION
{
	struct line_content
	{
		std::vector<std::string> objects;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> modNames;
		std::string projectileSpread;
		std::string projectileCount;
		std::string force;
		std::string forceMult;
		std::string damage;
		std::string damageToAdd;
		std::string damageMult;
		std::string innerRadius;
		std::string outerRadius;
		std::string imageSpaceRadius;
		std::string verticalOffsetMult;
		std::string placedObjectFadeDelay;
		std::string soundLevel;
		std::string staggerMagnitude;
		std::string projectileVectorX;
		std::string projectileVectorY;
		std::string projectileVectorZ;
		std::string light;
		std::string sound1;
		std::string sound2;
		std::string impactDataSet;
		std::string impactPlacedObject;
		std::string spawnProjectile;
		std::string fullName;
	};

	line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void readConfig(const std::string& folder);
	void patch(const line_content& line, RE::BGSExplosion* object);
}
