#ifndef WEAPONS_H
#define WEAPONS_H

using namespace std;

#include <regex>
#include <string>
#include <vector>
#include <fstream>
#include <iostream>
#include "PCH.h"
#include "utility.h"
#include "dirent.h"
#include "gameforms.h"

namespace WEAPONS
{

struct patch_instruction
{
	std::vector<std::string> object;
	std::vector<std::string> objectExcluded;
	std::vector<std::string> filterAmmos;
	std::vector<std::string> keywords;
	std::vector<std::string> keywordsOr;
	std::vector<std::string> keywordsExcluded;
	std::vector<std::string> modNames;
	std::vector<std::string> damageTypes;
	std::vector<float> values1;
	std::vector<float> values2;
	std::string attackDamage;
	std::string attackDamageMult;
	std::string attackDamageToAdd;

	std::string weight;
	std::string INRD;
	std::string capsvalue;
	std::string actionpointcost;
	std::string hittype;
	std::string soundlevel;
	std::string bashDamage;
	std::string minRange;
	std::string maxRange;
	std::string outOfRangeDamageMult;
	std::vector<std::string> keywordsToAdd;
	std::vector<std::string> keywordsToRemove;
	std::vector<std::string> attachParentSlotKeywordsToAdd;
	std::vector<std::string> attachParentSlotKeywordsToRemove;
	std::vector<std::string> ammo;
	std::vector<std::string> ammoList;
	std::vector<std::string> aimModel;
	std::string overrideProjectile;
	std::string weaponList;
	std::string fullName;
	std::string springBackMult;
	std::string accuracyMult;
	std::vector<std::string> damageTypesToRemove;
	std::vector<std::string> damageTypesToChangeByMult;
	std::vector<float> damageTypesByMultValues;
	std::string recoilPerShotMin;
	std::string recoilPerShotMax;
	std::vector<std::string> filterByFlagsExclude;
	std::string instanceNamingRule;
	
	
	
	
};


struct patch_instruction create_patch_instructions(const std::string& line);

void process_patch_instructions(const std::list<patch_instruction>& tokens);

void readConfig(const std::string& folder);
void patch(const WEAPONS::patch_instruction& line, RE::TESObjectWEAP* curobj);
}

#endif
