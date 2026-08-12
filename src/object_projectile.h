#pragma once

#include "PCH.h"
#include "dirent.h"
#include "utility.h"
#include <fstream>
#include <list>
#include <limits>
#include <type_traits>

namespace PROJECTILE
{
	struct line_content
	{
		std::vector<std::string> objects;
		std::vector<std::string> objectExcluded;
		std::vector<std::string> modNames;
		std::string gravity;
		std::string speed;
		std::string speedMult;
		std::string range;
		std::string rangeMult;
		std::string explosionProximity;
		std::string explosionTimer;
		std::string muzzleFlashDuration;
		std::string fadeOutTime;
		std::string force;
		std::string forceMult;
		std::string coneSpread;
		std::string collisionRadius;
		std::string lifetime;
		std::string relaunchInterval;
		std::string tracerFrequency;
		std::string soundLevel;
		std::string explosionType;
		std::string light;
		std::string muzzleFlashLight;
		std::string defaultWeaponSource;
		std::string vatsProjectile;
		std::string collisionLayer;
		std::string decalData;
		std::string fullName;
	};

	line_content create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<line_content>& tokens);
	void readConfig(const std::string& folder);
	void patch(const line_content& line, RE::BGSProjectile* object);
}
