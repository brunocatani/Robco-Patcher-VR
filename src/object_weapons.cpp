#include "object_weapons.h"
#include <unordered_set>

namespace WEAPONS
{

	struct patch_instruction create_patch_instructions(const std::string& line)
	{
		patch_instruction l;

		extractForms(line, "filterByWeapons\\s*=([^:]+)", l.object);

		// extract skipFlags
		std::regex skipFlagsExcluded_regex("filterByFlagsExcluded\\s*=([^:]+)", regex::icase);
		std::smatch skipFlagsExcluded_match;
		regexSearchParameter(line, skipFlagsExcluded_match, skipFlagsExcluded_regex);
		std::vector<std::string> skipFlagsExcluded;
		if (skipFlagsExcluded_match.empty() || skipFlagsExcluded_match[1].str().empty()) {
			//empty
		} else {
			std::string skipFlagsExcluded_str = skipFlagsExcluded_match[1];
			std::regex skipFlagsExcluded_list_regex("[^,]+(\\w+)", regex::icase);
			std::sregex_iterator skipFlagsExcluded_iterator(skipFlagsExcluded_str.begin(), skipFlagsExcluded_str.end(), skipFlagsExcluded_list_regex);
			std::sregex_iterator skipFlagsExcluded_end;
			while (skipFlagsExcluded_iterator != skipFlagsExcluded_end) {
				std::string keyword = (*skipFlagsExcluded_iterator)[0].str();
				keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
				keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
				if (keyword != "none") {
					skipFlagsExcluded.push_back(keyword);
				}
				++skipFlagsExcluded_iterator;
			}
			l.filterByFlagsExclude = skipFlagsExcluded;
		}

		extractForms(line, "filterByWeaponsExcluded\\s*=([^:]+)", l.objectExcluded);

		extractForms(line, "filterByAmmos\\s*=([^:]+)", l.filterAmmos);

		extractForms(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		extractForms(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		extractForms(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

		extractValueString(line, "attackDamage\\s*=([^:]+)", l.attackDamage);
		extractValueString(line, "attackDamageMult\\s*=([^:]+)", l.attackDamageMult);
		extractValueString(line, "attackDamageToAdd\\s*=([^:]+)", l.attackDamageToAdd);


		extractValueString(line, "weight\\s*=([^:]+)", l.weight);

		extractValueString(line, "value\\s*=([^:]+)", l.capsvalue);

		extractValueString(line, "attackActionPointCost\\s*=([^:]+)", l.actionpointcost);

		extractValueString(line, "weaponHitType\\s*=([^:]+)", l.hittype);

		extractValueString(line, "soundLevel\\s*=([^:]+)", l.soundlevel);

		extractValueString(line, "bashDamage\\s*=([^:]+)", l.bashDamage);

		extractValueString(line, "recoilDiminishSpringForce\\s*=([^:]+)", l.springBackMult);

		extractValueString(line, "coneIronSightsMultiplier\\s*=([^:]+)", l.accuracyMult);

		extractValueString(line, "recoilPerShotMin\\s*=([^:]+)", l.recoilPerShotMin);

		extractValueString(line, "recoilPerShotMax\\s*=([^:]+)", l.recoilPerShotMax);

		extractValueString(line, "outOfRangeDamageMult\\s*=([^:]+)", l.outOfRangeDamageMult);

		extractValueString(line, "filterByHasAmmoFromWeaponList\\s*=([^:]+)", l.weaponList);

		extractForms(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		extractForms(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

		extractForms(line, "attachParentSlotKeywordsToAdd\\s*=([^:]+)", l.attachParentSlotKeywordsToAdd);

		extractForms(line, "attachParentSlotKeywordsToRemove\\s*=([^:]+)", l.attachParentSlotKeywordsToRemove);

		extractForms(line, "setNewAmmo\\s*=([^:]+)", l.ammo);

		extractForms(line, "setNewAmmoList\\s*=([^:]+)", l.ammoList);

		extractForms(line, "aimModel\\s*=([^:]+)", l.aimModel);

		extractValueString(line, "overrideProjectile\\s*=([^:]+)", l.overrideProjectile);

		// extract DamageTypes
		std::regex DamageTypes_regex("changeDamageTypes\\s*=([^:]+)", regex::icase);
		std::smatch DamageTypes_match;
		regexSearchParameter(line, DamageTypes_match, DamageTypes_regex);
		std::vector<std::string> DamageTypes_before_eq;
		std::vector<float> DamageTypes_min_values;
		std::vector<float> DamageTypes_max_values;
		if (DamageTypes_match.empty() || DamageTypes_match[1].str().empty()) {
			//empty
		} else {
			std::string DamageTypes_str = DamageTypes_match[1];
			std::regex DamageTypes_list_regex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", regex::icase);
			std::sregex_iterator DamageTypes_iterator(DamageTypes_str.begin(), DamageTypes_str.end(), DamageTypes_list_regex);
			std::sregex_iterator DamageTypes_end;
			while (DamageTypes_iterator != DamageTypes_end) {
				std::string avif = (*DamageTypes_iterator)[1].str();
				avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
				avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

				if (avif == "none") {
					break;
				}

				DamageTypes_before_eq.push_back(avif);
				DamageTypes_min_values.push_back(std::stof((*DamageTypes_iterator)[2]));
				if ((*DamageTypes_iterator)[3] != "") {
					DamageTypes_max_values.push_back(std::stof((*DamageTypes_iterator)[3]));
				} else {
					DamageTypes_max_values.push_back(std::stof((*DamageTypes_iterator)[2]));
				}
				std::string val1 = ((*DamageTypes_iterator)[2]);
				std::string val2 = ((*DamageTypes_iterator)[3] != "") ? ((*DamageTypes_iterator)[3]) : ((*DamageTypes_iterator)[2]);
				//logger::info(FMT_STRING("avif: {}"), avif);
				//logger::info(FMT_STRING("value1: {}"), val1);
				//logger::info(FMT_STRING("value2: {}"), val2);
				++DamageTypes_iterator;
			}
			l.damageTypes = DamageTypes_before_eq;
			l.values1 = DamageTypes_min_values;
			l.values2 = DamageTypes_max_values;
		}
		
		// extract DamageTypesChangeNew
		std::regex DamageTypesChangeNew_regex("damageTypesToChange\\s*=([^:]+)", regex::icase);
		std::smatch DamageTypesChangeNew_match;
		regexSearchParameter(line, DamageTypesChangeNew_match, DamageTypesChangeNew_regex);
		std::vector<std::string> DamageTypesChangeNew_before_eq;
		std::vector<float> DamageTypesNew_min_values;
		std::vector<float> DamageTypesNew_max_values;
		if (DamageTypesChangeNew_match.empty() || DamageTypesChangeNew_match[1].str().empty()) {
			//empty
		} else {
				std::string DamageTypes_str = DamageTypesChangeNew_match[1];
			std::regex DamageTypes_list_regex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", regex::icase);
			std::sregex_iterator DamageTypes_iterator(DamageTypes_str.begin(), DamageTypes_str.end(), DamageTypes_list_regex);
			std::sregex_iterator DamageTypes_end;
			while (DamageTypes_iterator != DamageTypes_end) {
				std::string avif = (*DamageTypes_iterator)[1].str();
				avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
				avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

				if (avif == "none") {
					break;
				}

				DamageTypesChangeNew_before_eq.push_back(avif);
				DamageTypesNew_min_values.push_back(std::stof((*DamageTypes_iterator)[2]));
				if ((*DamageTypes_iterator)[3] != "") {
					DamageTypesNew_max_values.push_back(std::stof((*DamageTypes_iterator)[3]));
				} else {
					DamageTypesNew_max_values.push_back(std::stof((*DamageTypes_iterator)[2]));
				}
				std::string val1 = ((*DamageTypes_iterator)[2]);
				std::string val2 = ((*DamageTypes_iterator)[3] != "") ? ((*DamageTypes_iterator)[3]) : ((*DamageTypes_iterator)[2]);
				//logger::info(FMT_STRING("avif: {}"), avif);
				//logger::info(FMT_STRING("value1: {}"), val1);
				//logger::info(FMT_STRING("value2: {}"), val2);
				++DamageTypes_iterator;
			}
			l.damageTypes = DamageTypesChangeNew_before_eq;
			l.values1 = DamageTypesNew_min_values;
			l.values2 = DamageTypesNew_max_values;
		}

		// extract changeDamageTypesByMult
		std::regex DamageTypesByMult_regex("changeDamageTypesByMult\\s*=([^:]+)", regex::icase);
		std::smatch DamageTypesByMult_match;
		std::regex_search(line, DamageTypesByMult_match, DamageTypesByMult_regex);
		std::vector<std::string> DamageTypesByMult_before_eq;
		std::vector<float> DamageTypesByMult_values;
		if (DamageTypesByMult_match.empty() || DamageTypesByMult_match[1].str().empty()) {
			//empty
		} else {
			std::string DamageTypesByMult_str = DamageTypesByMult_match[1];
			std::regex DamageTypesByMult_list_regex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)", regex::icase);
			std::sregex_iterator DamageTypesByMult_iterator(DamageTypesByMult_str.begin(), DamageTypesByMult_str.end(), DamageTypesByMult_list_regex);
			std::sregex_iterator DamageTypesByMult_end;
			while (DamageTypesByMult_iterator != DamageTypesByMult_end) {
				std::string avif = (*DamageTypesByMult_iterator)[1].str();
				avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
				avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

				if (avif == "none") {
					break;
				}

				DamageTypesByMult_before_eq.push_back(avif);
				DamageTypesByMult_values.push_back(std::stof((*DamageTypesByMult_iterator)[2]));
				++DamageTypesByMult_iterator;
			}
			l.damageTypesToChangeByMult = DamageTypesByMult_before_eq;
			l.damageTypesByMultValues = DamageTypesByMult_values;
		}

		// extract damageTypesToRemove
		std::regex damageTypesToRemove_regex("damageTypesToRemove\\s*=([^:]+)", regex::icase);
		std::smatch damageTypesToRemove_match;
		regexSearchParameter(line, damageTypesToRemove_match, damageTypesToRemove_regex);
		std::vector<std::string> damageTypesToRemove;
		if (damageTypesToRemove_match.empty() || damageTypesToRemove_match[1].str().empty()) {
			//empty
		} else {
			std::string damageTypesToRemove_str = damageTypesToRemove_match[1];
			std::regex damageTypesToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator damageTypesToRemove_iterator(damageTypesToRemove_str.begin(), damageTypesToRemove_str.end(), damageTypesToRemove_list_regex);
			std::sregex_iterator damageTypesToRemove_end;
			while (damageTypesToRemove_iterator != damageTypesToRemove_end) {
				std::string keywordToRemove = (*damageTypesToRemove_iterator)[0].str();
				keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
				keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
				if (keywordToRemove != "none") {
					//logger::info(FMT_STRING("damageTypesToRemove: {}"), keywordToRemove);
					damageTypesToRemove.push_back(keywordToRemove);
				}
				++damageTypesToRemove_iterator;
			}
			l.damageTypesToRemove = damageTypesToRemove;
		}

		// extract fullName
		std::regex fullName_regex("fullName\\s*=\\s*~([^~]+?)\\s*~");
		std::smatch namematch;
		regexSearchParameter(line, namematch, fullName_regex);
		// extract the value after the equals sign
		if (namematch.empty() || namematch[1].str().empty()) {
			l.fullName = "none";
		} else {
			std::string namevalue = namematch[1].str();
			namevalue.erase(namevalue.begin(), std::find_if_not(namevalue.begin(), namevalue.end(), ::isspace));
			namevalue.erase(std::find_if_not(namevalue.rbegin(), namevalue.rend(), ::isspace).base(), namevalue.end());
			l.fullName = namevalue;
		}

		extractValueString(line, "minRange\\s*=([^:]+)", l.minRange);
		extractValueString(line, "maxRange\\s*=([^:]+)", l.maxRange);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		extractValueString(line, "instanceNamingRule\\s*=([^:]+)", l.INRD);

		logger::debug(FMT_STRING("weapon: {}  keywords: {}  ammo: {} keywordsToAdd: {} aimModel {}"), l.object.size(), l.keywords.size(), l.ammo.size(), l.keywordsToAdd.size(), l.aimModel.size());
		//logger::info("returning patch instructions");



		return l;
	}


	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
	const auto& RaceArray = dataHandler->GetFormArray<RE::TESObjectWEAP>();

		for (const auto& line : tokens) {
			std::unordered_set<std::uint32_t> directlyPatched;
			
			if (!line.object.empty()) {
				//logger::info("npc not empty");
				for (const auto& npcstring : line.object) {
					RE::TESForm* currentform = nullptr;
					RE::TESObjectWEAP* curobj = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kWEAP) {
						curobj = (RE::TESObjectWEAP*)currentform;
						if (FormMatchesModNames(curobj, line.modNames)) {
							patch(line, curobj);
							directlyPatched.insert(curobj->formID);
						}
					}
				}
			}

			if (!line.object.empty() && line.filterAmmos.empty() && line.keywords.empty() && line.keywordsOr.empty() && (line.weaponList.empty() || line.weaponList == "none")) {
				//logger::info("continue");
				continue;
			}


			for (const auto& curobj : RaceArray) {
				if (!curobj) {
					continue;
				}
				if (directlyPatched.contains(curobj->formID)) {
					continue;
				}
				//logger::info("processing npc");
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;

				if (curobj->IsDeleted()) {
					continue;
				}

				if (!FormMatchesModNames(curobj, line.modNames)) {
					continue;
				}
				
				if (!line.filterAmmos.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.filterAmmos) {
						RE::TESForm* currentform = nullptr;
						RE::TESAmmo* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kAMMO) {
							npc = (RE::TESAmmo*)currentform;

							if (curobj->weaponData.ammo && curobj->weaponData.ammo->formID == npc->formID) {
								found = true;
								break;
							}
						}
					}
				}

				if (!line.keywords.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywords) {
						RE::TESForm* currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordAnd = true;
							} else {
								keywordAnd = false;
								//logger::debug(FMT_STRING("KeywordAnd Weapon does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd Weapon true"));
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsOr) {
						RE::TESForm* currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								keywordOr = true;
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					//logger::debug(FMT_STRING("Found a matching weapon by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && !line.weaponList.empty() && line.weaponList != "none" && curobj->weaponData.ammo) {
					//logger::info("keywords not empty");

					RE::TESAmmo* curammo = curobj->weaponData.ammo;

					int ammoindex = -1;
					if (line.weaponList == "pistol") {
						//logger::debug(FMT_STRING("Weapon after ammoindex pistol {:08X} {}"), curnpc->formID, curnpc->fullName.c_str());
						if (ammoindex == -1) {
							ammoindex = findPositionInArray(listPistol, curammo);
							//logger::debug(FMT_STRING("Weapon ammoindex pistol {:08X} {}  {}"), curnpc->formID, curnpc->fullName.c_str() , ammoindex);
							if (ammoindex >= 0) {
								found = true;
								logger::debug(FMT_STRING("Weapon {:08X} {} has ammo {:08X} {} from list pistol index: {}"), curobj->formID, curobj->fullName.c_str(), curammo->formID, curammo->fullName.c_str(), ammoindex);
							}
						}
					} else if (line.weaponList == "rifle") {
						if (ammoindex == -1) {
							ammoindex = findPositionInArray(listRifle, curammo);
							if (ammoindex >= 0) {
								found = true;
								logger::debug(FMT_STRING("Weapon {:08X} {} has ammo {:08X} {} from list rifle index: {}"), curobj->formID, curobj->fullName.c_str(), curammo->formID, curammo->fullName.c_str(), ammoindex);
							}
						}
					}
				}

				if (!found && line.object.empty() && line.filterAmmos.empty() && line.keywords.empty() && line.keywordsOr.empty() && !line.weaponList.empty() && line.weaponList == "none") {
					found = true;
				}

				if (!line.keywordsExcluded.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword)) {
								found = false;
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"),keyword->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (!line.objectExcluded.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::TESObjectWEAP* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kWEAP) {
							npc = (RE::TESObjectWEAP*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								logger::info("Weapon Excluded.");
								break;
							}
						}
					}
				}

				if (!line.filterByFlagsExclude.empty()) {
					for (const auto& flag : line.filterByFlagsExclude) {
						//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude"), curobj->formID, curobj->fullName);
						if (toLowerCase(flag) == "notplayable" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kNotPlayable) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude nonplayable set to false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "cantdrop" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kCantDrop) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "embeddedweapon" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kEmbeddedWeapon) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "chargingattack" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kChargingAttack) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "criteffectondeath" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kCritEffectOnDeath) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "boltaction" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kBoltAction) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "automatic" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kAutomatic) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "secondaryweapon" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kSecondaryWeapon) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "chargingreload" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kChargingReload) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						} else if (toLowerCase(flag) == "holdinputtopower" && curobj->weaponData.flags & RE::WEAPON_FLAGS::kHoldInputToPower) {
							//logger::debug(FMT_STRING("weapon formid: {:08X} {} filterByFlagsExclude cantdrop false"), curobj->formID, curobj->fullName);
							found = false;
						}
					}
				
				}

				if (found) {
					patch(line,curobj);
				}

				//if (found && !line.attackDamage.empty() && line.attackDamage != "none") {
				//	try {
				//		curobj->weaponData.attackDamage = std::stof(line.attackDamage);
				//		logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->weaponData.attackDamage);
				//	} catch (const std::invalid_argument& e) {
				//	}
				//}
				//if (found && !line.weight.empty() && line.weight != "none") {
				//	try {
				//		curobj->weaponData.weight = std::stof(line.weight);
				//		logger::debug(FMT_STRING("weapon formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weaponData.weight);
				//	} catch (const std::invalid_argument& e) {
				//	}
				//}
				//if (found && !line.capsvalue.empty() && line.capsvalue != "none") {
				//	try {
				//		curobj->weaponData.value = std::stof(line.capsvalue);
				//		logger::debug(FMT_STRING("weapon formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->weaponData.value);
				//	} catch (const std::invalid_argument& e) {
				//	}
				//}
				//if (found && !line.actionpointcost.empty() && line.actionpointcost != "none") {
				//	try {
				//		curobj->weaponData.attackActionPointCost = std::stof(line.actionpointcost);
				//		logger::debug(FMT_STRING("weapon formid: {:08X} {} changed attack actionpoint cost {}"), curobj->formID, curobj->fullName, curobj->weaponData.attackActionPointCost);
				//	} catch (const std::invalid_argument& e) {
				//	}
				//}

				//if (found && !line.hittype.empty() && line.hittype != "none") {
				//	try {
				//		std::string hittypeLower = line.hittype;
				//		std::transform(hittypeLower.begin(), hittypeLower.end(), hittypeLower.begin(), [](unsigned char c) {
				//			return std::tolower(c);  // convert to lowercase
				//		});

				//		if (hittypeLower == "normal") {
				//			curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kNormal;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to normal"), curobj->formID, curobj->fullName);
				//		} else if (hittypeLower == "dismemberonly" || hittypeLower == "dismember") {
				//			curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kDismemberOnly;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to dismember only"), curobj->formID, curobj->fullName);
				//		} else if (hittypeLower == "explodeOnly" || hittypeLower == "explode") {
				//			curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kExplodeOnly;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to explode only"), curobj->formID, curobj->fullName);
				//		} else if (hittypeLower == "nodismemberorexplode" || hittypeLower == "no") {
				//			curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kNoDismemberOrExplode;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to no dismember or explode"), curobj->formID, curobj->fullName);
				//		}

				//		
				//	} catch (const std::invalid_argument& e) {
				//	}
				//}

				//if (found && !line.soundlevel.empty() && line.soundlevel != "none") {
				//	try {
				//		std::string hittypeLower = line.soundlevel;
				//		std::transform(hittypeLower.begin(), hittypeLower.end(), hittypeLower.begin(), [](unsigned char c) {
				//			return std::tolower(c);  // convert to lowercase
				//		});

				//		if (hittypeLower == "loud") {
				//			curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kLoud;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to loud"), curobj->formID, curobj->fullName);
				//		} else if (hittypeLower == "normal") {
				//			curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kNormal;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to normal"), curobj->formID, curobj->fullName);
				//		} else if (hittypeLower == "silent") {
				//			curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kSilent;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to silent"), curobj->formID, curobj->fullName);
				//		} else if (hittypeLower == "veryload") {
				//			curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kVeryLoud;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to very load"), curobj->formID, curobj->fullName);
				//		} else if (hittypeLower == "quiet") {
				//			curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kQuiet;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to quiet"), curobj->formID, curobj->fullName);
				//		}

				//	} catch (const std::invalid_argument& e) {
				//	}
				//}

				//if (found && !line.bashDamage.empty() && line.bashDamage != "none") {
				//	try {
				//		curobj->weaponData.secondaryDamage = std::stof(line.bashDamage);
				//		logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->weaponData.secondaryDamage);
				//	} catch (const std::invalid_argument& e) {
				//	}
				//}
				//if (found && !line.outOfRangeDamageMult.empty() && line.outOfRangeDamageMult != "none") {
				//	try {
				//		curobj->weaponData.outOfRangeDamageMult = std::stof(line.outOfRangeDamageMult);
				//		logger::debug(FMT_STRING("weapon formid: {:08X} {} changed outOfRangeDamageMult {}"), curobj->formID, curobj->fullName, curobj->weaponData.outOfRangeDamageMult);
				//	} catch (const std::invalid_argument& e) {
				//	}
				//}
				//if (found && !line.attachParentSlotKeywordsToAdd.empty()) {
				//	//logger::info("found! patching values");
				//	//for (const auto& avifstring : line.avifs)
				//	for (size_t i = 0; i < line.attachParentSlotKeywordsToAdd.size(); i++) {
				//		RE::TESForm* currentform = nullptr;
				//		std::string string_form = line.attachParentSlotKeywordsToAdd[i];
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
				//			if (!curobj->attachParents.HasKeyword((RE::BGSKeyword*)currentform)) {
				//				curobj->attachParents.AddKeyword((RE::BGSKeyword*)currentform);
				//				logger::debug(FMT_STRING("weapon formid: {:08X} {} added attach parent slot keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				//			}
				//		}
				//	}
				//}
				//if (found && !line.attachParentSlotKeywordsToRemove.empty()) {
				//	for (size_t i = 0; i < line.attachParentSlotKeywordsToRemove.size(); i++) {
				//		RE::TESForm* currentform = nullptr;
				//		std::string string_form = line.attachParentSlotKeywordsToRemove[i];
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
				//			if (!curobj->attachParents.HasKeyword((RE::BGSKeyword*)currentform)) {
				//				curobj->attachParents.RemoveKeyword((RE::BGSKeyword*)currentform);

				//				logger::debug(FMT_STRING("armor formid: {:08X} removed attach parent slot keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				//			}
				//		}
				//	}
				//}
				//if (found && !line.keywordsToAdd.empty()) {
				//	for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				//		RE::TESForm* currentform = nullptr;
				//		std::string string_form = line.keywordsToAdd[i];
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
				//			curobj->AddKeyword((RE::BGSKeyword*)currentform);
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				//		}
				//	}
				//}

				//if (found && !line.keywordsToRemove.empty()) {
				//	for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				//		RE::TESForm* currentform = nullptr;
				//		std::string string_form = line.keywordsToRemove[i];
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
				//			curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
				//			logger::debug(FMT_STRING("weapon formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				//		}
				//	}
				//}
				//
				//if (found && !line.ammo.empty()) {
				//	//logger::info(FMT_STRING("found in ammo {}"), found);
				//	for (size_t i = 0; i < line.ammo.size(); i++) {
				//		//logger::info(FMT_STRING("found in ammo {} {}"), line.ammo.size(), line.ammo[i]);
				//		RE::TESForm* currentform = nullptr;
				//		std::string string_form = line.ammo[i];
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kAMMO) {
				//			curobj->weaponData.ammo = (RE::TESAmmo*)currentform;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed ammo {:08X} {} "), curobj->formID, curobj->fullName, ((RE::TESAmmo*)currentform)->formID, ((RE::TESAmmo*)currentform)->fullName);
				//		}
				//	}
				//}
				//if (found && !line.ammoList.empty()) {
				//	for (size_t i = 0; i < line.ammo.size(); i++) {
				//		RE::TESForm* currentform = nullptr;
				//		std::string string_form = line.ammoList[i];
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kLVLI) {
				//			curobj->weaponData.npcAddAmmoList = (RE::TESLevItem*)currentform;
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} change npc ammo list {:08X}  "), curobj->formID, curobj->fullName, ((RE::TESLevItem*)currentform)->formID);
				//		}
				//	}
				//}
				//if (found && !line.aimModel.empty()) {
				//	//logger::info(FMT_STRING("Processing Spell list size {}"), line.spellsToAdd.size());
				//	//for (const auto& avifstring : line.avifs)
				//	for (size_t i = 0; i < line.aimModel.size(); i++) {
				//		RE::TESForm* currentform = nullptr;
				//		std::string string_form = line.aimModel[i];
				//		currentform = GetFormFromIdentifier(string_form);
				//		//logger::info(FMT_STRING("weapon formid: {:08X} {} checking aim model...  "), curnpc->formID, curnpc->fullName);
				//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kAMDL) {
				//			// currentform->As<RE::BGSAimModel>() &&
				//			auto cAimModel = fallout_cast<RE::BGSAimModel*>(currentform);
				//			if (cAimModel) {
				//				curobj->weaponData.aimModel = cAimModel;
				//				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed aim model to {:08X}  "), curobj->formID, curobj->fullName, (currentform)->formID);
				//			}
				//		}
				//	}
				//}

				//if (found && !line.damageTypes.empty()) {
				//	//logger::info("found! patching values");
				//	//for (const auto& avifstring : line.avifs)
				//	for (size_t i = 0; i < line.damageTypes.size(); i++) {
				//		RE::TESForm* currentform = nullptr;
				//		std::string string_form = line.damageTypes[i];
				//		currentform = GetFormFromIdentifier(string_form);
				//		if (currentform && currentform->formType == RE::ENUM_FORM_ID::kDMGT) {
				//			//logger::info("avif valid!");
				//			float finalValue = 0;


				//				if (!line.values1.empty() && !line.values2.empty()) {
				//					finalValue = floor((std::rand() / static_cast<float>(RAND_MAX)) * (line.values2[i] - line.values1[i] + 1) + line.values1[i]);
				//				}

				//			changeDamageType_Weapon(curobj, (RE::BGSDamageType*)currentform, finalValue);
				//			logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage type {:08X} to {}"), curobj->formID, curobj->fullName, ((RE::BGSDamageType*)currentform)->formID, finalValue);
				//		}
				//	}
				//}
				//
				//if (found && !line.fullName.empty() && line.fullName != "none") {
				//	try {
				//		logger::debug(FMT_STRING("weapon formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				//		curobj->fullName = line.fullName;
				//	} catch (const std::invalid_argument& e) {
				//	}
				//}
				//if (found) {
				//	const auto dataHandler = RE::TESDataHandler::GetSingleton();
				//	RE::BSTArray<RE::TESObjectREFR*> refs = dataHandler->GetFormArray<RE::TESObjectREFR>();
				//	logger::debug(FMT_STRING("weapon instanceData count {}"), refs.size());
				//	for (const auto& ref : refs) {
				//		logger::debug(FMT_STRING("weapon instanceData curobj {:08X} ref {:08X} base formID {:08X}"), curobj->formID, ref->formID, ref->GetObjectReference()->formID);
				//		if (ref->GetObjectReference()->formID == curobj->formID) {
				//			ref->extraList->RemoveExtra(RE::EXTRA_DATA_TYPE::kInstanceData);
				//			logger::debug(FMT_STRING("weapon instanceData ref {:08X} base formID {:08X}"), ref->formID, ref->GetObjectReference()->formID);
				//		}
				//	}

				//}

				
				
				//const std::string testB = "Fallout4.esm|0004C922";
				//RE::TESForm* test11 = GetFormFromIdentifier(testB);
				//RE::BGSKeyword* testk = (RE::BGSKeyword*)test11;
				//const std::string ref = "CorvalhoWidowShotgun.esp|3E00E603";
				//RE::TESForm* refForm = GetFormFromIdentifier(ref);
				//if (0x3E0044EA == curobj->formID) {
				//	RE::TESObjectREFR* test1 = static_cast<RE::TESObjectREFR*>(refForm);
				//	RE::TESBoundObject* ref = test1->GetObjectReference();
				//	RE::TESObjectWEAP* weap = ref->As<RE::TESObjectWEAP>();
				//	//auto instanceDataCopy = *(weap->GetBaseInstanceData());
				//	//test1->extraList->SetInstanceData(&instanceDataCopy, ref);
				//	test1->extraList->RemoveExtra(RE::EXTRA_DATA_TYPE::kInstanceData);
				//	//RE::BaseExtraList::RemoveExtra //test1->
				//	//RE::TESObjectWEAP::InstanceData::CreateKeywordData
				//	//weap->weaponData.attackDamage = 1;
				//	logger::debug(FMT_STRING(" formID {:08X} {:08X} {:08X} {}"), test1->formID, weap->formID, ref->formID, weap->weaponData.attackDamage);
				//} else {
				//	logger::debug("Failed to get form from identifier");
				//	//aidatauto weap = (RE::TESObjectWEAP*)test1->GetHandle().get().get();
				//	//auto BO = test1->GetObjectReference();
				//	//auto weap = BO->As<RE::TESObjectWEAP>();
				//	//weap->weaponData = ((RE::TESObjectWEAP*)test11)->weaponData;
				//}


			}
		}
	}

	void readConfig(const std::string& folder)
	{
		char skipChar = '/';
		std::string extension = ".ini";

		DIR* dir;
		struct dirent* ent;
		std::list<std::string> directories{ folder };
		std::string currentFolder;

		while (!directories.empty()) {
			currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) != NULL) {
				while ((ent = readdir(dir)) != NULL) {
					if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
						std::string fullPath = currentFolder + "\\" + ent->d_name;
						struct _stat st;
						if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
							directories.push_back(fullPath);
						} else {
							std::string fileName = ent->d_name;
							if (HasIniExtension(fileName)) {
								fileName.resize(fileName.size() - 4);
								const char* modname = fileName.c_str();

								if ((strstr(modname, ".esp") != nullptr || strstr(modname, ".esl") != nullptr || strstr(modname, ".esm") != nullptr)) {
									if (!IsPluginInstalled(modname)) {
										logger::info("************************************************************");
										logger::info(FMT_STRING("{} not found or is not a valid plugin file, skipping config file {}."), modname, fullPath);
										logger::info("************************************************************");
										continue;
									}
								}
								logger::info("************************************************************");
								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								logger::info("************************************************************");
								PATCH::RecordFile("weapon", fullPath);
								std::string line;
								std::ifstream infile;
								std::list<patch_instruction> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line.empty()) {
										continue;
									}
									if (line[0] == skipChar) {
										continue;
									}

									PATCH::RecordRule("weapon");
									tokens.push_back(create_patch_instructions(line));
								}
								infile.close();
								process_patch_instructions(tokens);
							}
						}
					}
				}
				closedir(dir);
			} else {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder.c_str());
			}
		}
		return;
	}

	void patch(const WEAPONS::patch_instruction& line, RE::TESObjectWEAP* curobj)
	{
		if (!curobj || ShouldSkipPatch("weapon", curobj)) {
			return;
		}


		if (!line.minRange.empty() && line.minRange != "none") {
			try {
				curobj->weaponData.minRange = std::stof(line.minRange);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed minRange {}"), curobj->formID, curobj->fullName, curobj->weaponData.minRange);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid minRange value '{}': {}"), curobj->formID, line.minRange, e.what());
			}
		}

		if (!line.maxRange.empty() && line.maxRange != "none") {
			try {
				curobj->weaponData.maxRange = std::stof(line.maxRange);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed maxRange {}"), curobj->formID, curobj->fullName, curobj->weaponData.maxRange);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid maxRange value '{}': {}"), curobj->formID, line.maxRange, e.what());
			}
		}

		if (!line.attackDamage.empty() && line.attackDamage != "none") {
			try {
				const auto value = std::stod(line.attackDamage);
				if (!std::isfinite(value) || value < 0.0 || value > (std::numeric_limits<std::uint16_t>::max)()) {
					throw std::out_of_range("attackDamage must be between 0 and 65535");
				}
				curobj->weaponData.attackDamage = static_cast<std::uint16_t>(value);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->weaponData.attackDamage);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid attackDamage value '{}': {}"), curobj->formID, line.attackDamage, e.what());
			}
		}

		if (!line.attackDamageToAdd.empty() && line.attackDamageToAdd != "none") {
			try {
				const auto value = static_cast<double>(curobj->weaponData.attackDamage) + std::stod(line.attackDamageToAdd);
				if (!std::isfinite(value) || value < 0.0 || value > (std::numeric_limits<std::uint16_t>::max)()) {
					throw std::out_of_range("resulting attackDamage must be between 0 and 65535");
				}
				curobj->weaponData.attackDamage = static_cast<std::uint16_t>(value);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} added damage {}"), curobj->formID, curobj->fullName, line.attackDamageToAdd);
			}
			catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid attackDamageToAdd value '{}': {}"), curobj->formID, line.attackDamageToAdd, e.what());
			}
		}

		if (!line.attackDamageMult.empty() && line.attackDamageMult != "none") {
			try {
				const auto value = static_cast<double>(curobj->weaponData.attackDamage) * std::stod(line.attackDamageMult);
				if (!std::isfinite(value) || value < 0.0 || value > (std::numeric_limits<std::uint16_t>::max)()) {
					throw std::out_of_range("resulting attackDamage must be between 0 and 65535");
				}
				curobj->weaponData.attackDamage = static_cast<std::uint16_t>(value);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} multiplied attackDamage to {}"), curobj->formID, curobj->fullName, curobj->weaponData.attackDamage);
			}
			catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid attackDamageMult value '{}': {}"), curobj->formID, line.attackDamageMult, e.what());
			}
		}

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weaponData.weight = std::stof(line.weight);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weaponData.weight);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid weight value '{}': {}"), curobj->formID, line.weight, e.what());
			}
		}
		if (!line.capsvalue.empty() && line.capsvalue != "none") {
			try {
				const auto value = std::stod(line.capsvalue);
				if (!std::isfinite(value) || value < 0.0 || value > (std::numeric_limits<std::uint32_t>::max)()) {
					throw std::out_of_range("value must be between 0 and 4294967295");
				}
				curobj->weaponData.value = static_cast<std::uint32_t>(value);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->weaponData.value);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid value '{}': {}"), curobj->formID, line.capsvalue, e.what());
			}
		}
		if (!line.actionpointcost.empty() && line.actionpointcost != "none") {
			try {
				curobj->weaponData.attackActionPointCost = std::stof(line.actionpointcost);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed attack actionpoint cost {}"), curobj->formID, curobj->fullName, curobj->weaponData.attackActionPointCost);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid actionPointCost value '{}': {}"), curobj->formID, line.actionpointcost, e.what());
			}
		}

		if (!line.hittype.empty() && line.hittype != "none") {
			try {
				std::string hittypeLower = line.hittype;
				std::transform(hittypeLower.begin(), hittypeLower.end(), hittypeLower.begin(), [](unsigned char c) {
					return std::tolower(c);  // convert to lowercase
				});

				if (hittypeLower == "normal") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kNormal;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to normal"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "dismemberonly" || hittypeLower == "dismember") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kDismemberOnly;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to dismember only"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "explodeonly" || hittypeLower == "explode") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kExplodeOnly;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to explode only"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "nodismemberorexplode" || hittypeLower == "no") {
					curobj->weaponData.hitBehavior = RE::WEAPONHITBEHAVIOR::kNoDismemberOrExplode;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed hit behavior to no dismember or explode"), curobj->formID, curobj->fullName);
				} else {
					logger::warn(FMT_STRING("Weapon {:08X}: unknown hitType '{}'"), curobj->formID, line.hittype);
				}

			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid hitType value '{}': {}"), curobj->formID, line.hittype, e.what());
			}
		}

		if (!line.soundlevel.empty() && line.soundlevel != "none") {
			try {
				std::string hittypeLower = line.soundlevel;
				std::transform(hittypeLower.begin(), hittypeLower.end(), hittypeLower.begin(), [](unsigned char c) {
					return std::tolower(c);  // convert to lowercase
				});

				if (hittypeLower == "loud") {
					curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kLoud;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to loud"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "normal") {
					curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kNormal;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to normal"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "silent") {
					curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kSilent;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to silent"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "veryloud" || hittypeLower == "veryload") {
					curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kVeryLoud;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to very loud"), curobj->formID, curobj->fullName);
				} else if (hittypeLower == "quiet") {
					curobj->weaponData.soundLevel = RE::SOUND_LEVEL::kQuiet;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed sound level to quiet"), curobj->formID, curobj->fullName);
				} else {
					logger::warn(FMT_STRING("Weapon {:08X}: unknown soundLevel '{}'"), curobj->formID, line.soundlevel);
				}

			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid soundLevel value '{}': {}"), curobj->formID, line.soundlevel, e.what());
			}
		}

		if (!line.bashDamage.empty() && line.bashDamage != "none") {
			try {
				curobj->weaponData.secondaryDamage = std::stof(line.bashDamage);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->weaponData.secondaryDamage);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid bashDamage value '{}': {}"), curobj->formID, line.bashDamage, e.what());
			}
		}

		if (!line.accuracyMult.empty() && line.accuracyMult != "none") {
			if (!curobj->weaponData.aimModel) {
				logger::warn(FMT_STRING("Weapon {:08X}: cannot apply accuracyMult because aimModel is null"), curobj->formID);
			} else {
			try {
				curobj->weaponData.aimModel->aimModelData.aimModelConeIronSightsMultiplier = std::stof(line.accuracyMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed aimModelConeIronSightsMultiplier {}"), curobj->formID, curobj->fullName, curobj->weaponData.aimModel->aimModelData.aimModelConeIronSightsMultiplier);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid accuracyMult value '{}': {}"), curobj->formID, line.accuracyMult, e.what());
			}
			}
		}

		if (!line.springBackMult.empty() && line.springBackMult != "none" && curobj->weaponData.aimModel) {
			try {
				curobj->weaponData.aimModel->aimModelData.aimModelRecoilDiminishSpringForce = std::stof(line.springBackMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed aimModelRecoilDiminishSpringForce {}"), curobj->formID, curobj->fullName, curobj->weaponData.aimModel->aimModelData.aimModelRecoilDiminishSpringForce);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid springBackMult value '{}': {}"), curobj->formID, line.springBackMult, e.what());
			}
		}

		if (!line.recoilPerShotMin.empty() && line.recoilPerShotMin != "none" && curobj->weaponData.aimModel) {
			try {
				curobj->weaponData.aimModel->aimModelData.aimModelRecoilMinDegPerShot = std::stof(line.recoilPerShotMin);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed aimModelRecoilMinDegPerShot {}"), curobj->formID, curobj->fullName, curobj->weaponData.aimModel->aimModelData.aimModelRecoilMinDegPerShot);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid recoilPerShotMin value '{}': {}"), curobj->formID, line.recoilPerShotMin, e.what());
			}
		}

		if (!line.recoilPerShotMax.empty() && line.recoilPerShotMax != "none" && curobj->weaponData.aimModel) {
			try {
				curobj->weaponData.aimModel->aimModelData.aimModelRecoilMaxDegPerShot = std::stof(line.recoilPerShotMax);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed aimModelRecoilMaxDegPerShot {}"), curobj->formID, curobj->fullName, curobj->weaponData.aimModel->aimModelData.aimModelRecoilMaxDegPerShot);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid recoilPerShotMax value '{}': {}"), curobj->formID, line.recoilPerShotMax, e.what());
			}
		}

		if (!line.outOfRangeDamageMult.empty() && line.outOfRangeDamageMult != "none") {
			try {
				curobj->weaponData.outOfRangeDamageMult = std::stof(line.outOfRangeDamageMult);
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed outOfRangeDamageMult {}"), curobj->formID, curobj->fullName, curobj->weaponData.outOfRangeDamageMult);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid outOfRangeDamageMult value '{}': {}"), curobj->formID, line.outOfRangeDamageMult, e.what());
			}
		}
		if (!line.attachParentSlotKeywordsToAdd.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (size_t i = 0; i < line.attachParentSlotKeywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.attachParentSlotKeywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					if (!curobj->attachParents.HasKeyword((RE::BGSKeyword*)currentform)) {
						VRCompat::AddKeywordAttachPoint(curobj->attachParents, (RE::BGSKeyword*)currentform);
						logger::debug(FMT_STRING("weapon formid: {:08X} {} added attach parent slot keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}
		}
		if (!line.attachParentSlotKeywordsToRemove.empty()) {
			for (size_t i = 0; i < line.attachParentSlotKeywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.attachParentSlotKeywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					if (curobj->attachParents.HasKeyword((RE::BGSKeyword*)currentform)) {
						VRCompat::RemoveKeyword(curobj->attachParents, (RE::BGSKeyword*)currentform);

						logger::debug(FMT_STRING("weapon formid: {:08X} removed attach parent slot keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}
		}
		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.keywordsToRemove.empty()) {
			for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.keywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("weapon formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.ammo.empty()) {
			//logger::info(FMT_STRING("found in ammo {}"), found);
			for (size_t i = 0; i < line.ammo.size(); i++) {
				//logger::info(FMT_STRING("found in ammo {} {}"), line.ammo.size(), line.ammo[i]);
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.ammo[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kAMMO) {
					curobj->weaponData.ammo = (RE::TESAmmo*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed ammo {:08X} {} "), curobj->formID, curobj->fullName, ((RE::TESAmmo*)currentform)->formID, ((RE::TESAmmo*)currentform)->fullName);
				}
			}
		}
		if (!line.ammoList.empty()) {
			for (size_t i = 0; i < line.ammoList.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.ammoList[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kLVLI) {
					curobj->weaponData.npcAddAmmoList = (RE::TESLevItem*)currentform;
					logger::debug(FMT_STRING("weapon formid: {:08X} {} change npc ammo list {:08X}  "), curobj->formID, curobj->fullName, ((RE::TESLevItem*)currentform)->formID);
				}
			}
		}
		if (!line.aimModel.empty()) {
			//logger::info(FMT_STRING("Processing Spell list size {}"), line.spellsToAdd.size());
			//for (const auto& avifstring : line.avifs)
			for (size_t i = 0; i < line.aimModel.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.aimModel[i];
				currentform = GetFormFromIdentifier(string_form);
				//logger::info(FMT_STRING("weapon formid: {:08X} {} checking aim model...  "), curnpc->formID, curnpc->fullName);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kAMDL) {
					// currentform->As<RE::BGSAimModel>() &&
					auto cAimModel = fallout_cast<RE::BGSAimModel*>(currentform);
					if (cAimModel) {
						curobj->weaponData.aimModel = cAimModel;
						logger::debug(FMT_STRING("weapon formid: {:08X} {} changed aim model to {:08X}  "), curobj->formID, curobj->fullName, (currentform)->formID);
					}
				}
			}
		}

		if (!line.damageTypes.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (size_t i = 0; i < line.damageTypes.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.damageTypes[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kDMGT) {
					//logger::info("avif valid!");
					float finalValue = 0;

					if (i < line.values1.size() && i < line.values2.size()) {
						finalValue = floor((std::rand() / static_cast<float>(RAND_MAX)) * (line.values2[i] - line.values1[i] + 1) + line.values1[i]);
					} else {
						logger::warn(FMT_STRING("Weapon {:08X}: missing damage value for damage type '{}'"), curobj->formID, string_form);
						continue;
					}

					changeDamageType_Weapon(curobj, (RE::BGSDamageType*)currentform, finalValue);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage type {:08X} to {}"), curobj->formID, curobj->fullName, ((RE::BGSDamageType*)currentform)->formID, finalValue);
				}
			}
		}

		if (!line.damageTypesToChangeByMult.empty()) {
			for (size_t i = 0; i < line.damageTypesToChangeByMult.size(); i++) {
				if (i >= line.damageTypesByMultValues.size()) {
					logger::warn(FMT_STRING("Weapon {:08X}: missing multiplier for damage type entry {}"), curobj->formID, i);
					continue;
				}
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.damageTypesToChangeByMult[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kDMGT && curobj->weaponData.damageTypes) {
					auto& damageTypes = curobj->weaponData.damageTypes[0];
					for (RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>::size_type j = 0; j < damageTypes.size(); ++j) {
						if (damageTypes[j].first->formID == currentform->formID) {
							float finalValue = static_cast<float>(damageTypes[j].second.i) * line.damageTypesByMultValues[i];
							changeDamageType_Weapon(curobj, (RE::BGSDamageType*)currentform, finalValue);
							logger::debug(FMT_STRING("weapon formid: {:08X} {} changed damage type {:08X} by mult to {}"), curobj->formID, curobj->fullName, currentform->formID, finalValue);
							break;
						}
					}
				}
			}
		}

		if (!line.damageTypesToRemove.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (size_t i = 0; i < line.damageTypesToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.damageTypesToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kDMGT) {
					//logger::info("avif valid!");
					eraseDamageType_Weapon(curobj, (RE::BGSDamageType*)currentform);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} removed damage types {:08X}"), curobj->formID, curobj->fullName, ((RE::BGSDamageType*)currentform)->formID);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("weapon formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Weapon {:08X}: could not change fullName: {}"), curobj->formID, e.what());
			}
		}

		if (!line.overrideProjectile.empty() && line.overrideProjectile != "none") {
			if (!curobj->weaponData.rangedData) {
				logger::warn(FMT_STRING("Weapon {:08X}: cannot change overrideProjectile because rangedData is null"), curobj->formID);
			} else {
			RE::TESForm* currentform = nullptr;
			std::string string_form = line.overrideProjectile;
			currentform = GetFormFromIdentifier(string_form);
			//logger::debug(FMT_STRING("Projectile -> {}"), string_form);
			if (currentform && currentform->formType == RE::ENUM_FORM_ID::kPROJ && curobj->weaponData.rangedData) {
				curobj->weaponData.rangedData->overrideProjectile = (RE::BGSProjectile*)currentform;
				logger::debug(FMT_STRING("weapon formid: {:08X} changed overrideProjectile to {:08X} "), curobj->formID, ((RE::BGSProjectile*)currentform)->formID);
			} else if (toLowerCase(line.overrideProjectile) == "null") {
				curobj->weaponData.rangedData->overrideProjectile = nullptr;
				logger::debug(FMT_STRING("weapon formid: {:08X} changed overrideProjectile to null (none) "), curobj->formID);
			} else {
				logger::warn(FMT_STRING("Weapon {:08X}: invalid overrideProjectile '{}'"), curobj->formID, line.overrideProjectile);
			}
			}
		}

		if (!line.INRD.empty()) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.INRD;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kINNR) {
					curobj->instanceNamingRules = ((RE::BGSInstanceNamingRules*)currentform);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} changed InstanceNamingRules to {:08X} "), curobj->formID, curobj->fullName, currentform->formID);
				} else if (toLowerCase(line.INRD) == "null") {
					curobj->instanceNamingRules = nullptr;
					logger::debug(FMT_STRING("weapon formid: {:08X} changed InstanceNamingRules to null (none) "), curobj->formID);
				}
		}




		return;
	}

}
