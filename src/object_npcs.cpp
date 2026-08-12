#include "object_npcs.h"

#include <cmath>
#include <limits>
#include <stdexcept>

namespace NPCS
{
	template <class T>
	T checkedIntegralValue(double value)
	{
		if (!std::isfinite(value) || std::trunc(value) != value ||
			value < static_cast<double>((std::numeric_limits<T>::lowest)()) ||
			value > static_cast<double>((std::numeric_limits<T>::max)())) {
			throw std::out_of_range("value is outside the target integer field range");
		}
		return static_cast<T>(value);
	}


	struct patch_instruction create_patch_instruction(const std::string& line)
	{
		patch_instruction l;
		// extract npcs
		// extract objects

		extractForms(line, "filterByNpcs\\s*=([^:]+)", l.object);


		extractForms(line, "filterByNpcsExcluded\\s*=([^:]+)", l.objectExcluded);

		extractForms(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		extractForms(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		extractForms(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

		extractForms(line, "filterByRaces\\s*=([^:]+)", l.races);


		extractForms(line, "filterByClass\\s*=([^:]+)", l.filterClass);

		extractForms(line, "filterByFactions\\s*=([^:]+)", l.filterFactions);

		extractForms(line, "filterByFactionsOr\\s*=([^:]+)", l.filterFactionsOr);

		extractForms(line, "filterByFactionsExcluded\\s*=([^:]+)", l.filterFactionsExcluded);

		extractValueString(line, "setAutoCalcStats\\s*=([^:]+)", l.calcStats);

		//// extract pclevelmult
		//std::regex pclevelmultlist_regex("setPcLevelMult\\s*=([^:]+)", regex::icase);
		//std::smatch pclevelmultlistmatch;
		//std::regex_search(line, pclevelmultlistmatch, pclevelmultlist_regex);
		//// extract the value after the equals sign
		//if (pclevelmultlistmatch.empty() || pclevelmultlistmatch[1].str().empty()) {
		//	l.kPCLevelMult = "none";
		//} else {
		//	std::string tempString = pclevelmultlistmatch[1].str();
		//	tempString.erase(tempString.begin(), std::find_if_not(tempString.begin(), tempString.end(), ::isspace));
		//	tempString.erase(std::find_if_not(tempString.rbegin(), tempString.rend(), ::isspace).base(), tempString.end());
		//	l.kPCLevelMult = tempString;
		//}

		//
		// extract pcMultFlag
		std::regex pcMultFlag_regex("setPcLevelMult\\s*=([^:]+)", regex::icase);
		std::smatch pcMultFlag_match;
		regexSearchParameter(line, pcMultFlag_match, pcMultFlag_regex);
		std::vector<std::string> pcMultFlag_before_eq;
		std::vector<int> pcMultFlag_min_values;
		std::vector<int> pcMultFlag_max_values;
		if (pcMultFlag_match.empty() || pcMultFlag_match[1].str().empty()) {
			//empty
		} else {
			std::string pcMultFlag_str = pcMultFlag_match[1];
			std::regex pcMultFlag_list_regex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", regex::icase);
			std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
			std::sregex_iterator pcMultFlag_end;
			while (pcMultFlag_iterator != pcMultFlag_end) {
				std::string avif = (*pcMultFlag_iterator)[1].str();
				avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
				avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

				if (avif == "none") {
					break;
				}

				pcMultFlag_before_eq.push_back(avif);
				pcMultFlag_min_values.push_back(std::stoi((*pcMultFlag_iterator)[2]));
				if ((*pcMultFlag_iterator)[3] != "") {
					pcMultFlag_max_values.push_back(std::stoi((*pcMultFlag_iterator)[3]));
				} else {
					pcMultFlag_max_values.push_back(std::stoi((*pcMultFlag_iterator)[2]));
				}
				std::string val1 = ((*pcMultFlag_iterator)[2]);
				std::string val2 = ((*pcMultFlag_iterator)[3] != "") ? ((*pcMultFlag_iterator)[3]) : ((*pcMultFlag_iterator)[2]);
				//logger::info(FMT_STRING("avif: {}"), avif);
				//logger::info(FMT_STRING("value1: {}"), val1);
				//logger::info(FMT_STRING("value2: {}"), val2);
				++pcMultFlag_iterator;
			}
			l.pcMultFlag = pcMultFlag_before_eq;
			l.PCvalues1 = pcMultFlag_min_values;
			l.PCvalues2 = pcMultFlag_max_values;
		}

		extractValueString(line, "setEssential\\s*=([^:]+)", l.kEssential);

		extractValueString(line, "setProtected\\s*=([^:]+)", l.kProtected);

		// extract getLevelRange
		std::regex levelRange_regex("levelRange\\s*=\\s*(\\d+)\\s*~\\s*(\\d+)", regex::icase);
		std::smatch levelRange;
		regexSearchParameter(line, levelRange, levelRange_regex);
		if (levelRange.empty() || levelRange.size() < 3) {
			l.level_min = "none";
			l.level_max = "none";
		} else {
			l.level_min = levelRange[1].str();
			l.level_max = levelRange[2].str();
		}

		// extract avifs
		std::regex avifs_regex("changeAVIFS\\s*=([^:]+)", regex::icase);
		std::smatch avifs_match;
		regexSearchParameter(line, avifs_match, avifs_regex);
		std::vector<std::string> avifs_before_eq;
		std::vector<float> avifs_min_values;
		std::vector<float> avifs_max_values;
		if (avifs_match.empty() || avifs_match[1].str().empty()) {
			//empty
		} else {
			std::string avifs_str = avifs_match[1];
			std::regex avifs_list_regex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*(-?[\\d.]+)(?:\\s*~\\s*(-?[\\d.]+))?", std::regex::icase);
			std::sregex_iterator avifs_iterator(avifs_str.begin(), avifs_str.end(), avifs_list_regex);
			std::sregex_iterator avifs_end;
			while (avifs_iterator != avifs_end) {
				std::string avif = (*avifs_iterator)[1].str();
				avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
				avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

				if (avif == "none") {
					break;
				}

				avifs_before_eq.push_back(avif);
				avifs_min_values.push_back(std::stof((*avifs_iterator)[2]));
				if ((*avifs_iterator)[3] != "") {
					avifs_max_values.push_back(std::stof((*avifs_iterator)[3]));
				} else {
					avifs_max_values.push_back(std::stof((*avifs_iterator)[2]));
				}
				std::string val1 = ((*avifs_iterator)[2]);
				std::string val2 = ((*avifs_iterator)[3] != "") ? ((*avifs_iterator)[3]) : ((*avifs_iterator)[2]);
				//logger::info(FMT_STRING("avif: {}"), avif);
				//logger::info(FMT_STRING("value1: {}"), val1);
				//logger::info(FMT_STRING("value2: {}"), val2);
				++avifs_iterator;
			}
			l.avifs = avifs_before_eq;
			l.values1 = avifs_min_values;
			l.values2 = avifs_max_values;
		}

		// extract Faction
		std::regex Faction_regex("factionsToAdd\\s*=([^:]+)", regex::icase);
		std::smatch Faction_match;
		regexSearchParameter(line, Faction_match, Faction_regex);
		std::vector<std::string> Faction_before_eq;
		std::vector<float> Faction_min_values;
		std::vector<float> Faction_max_values;
		if (Faction_match.empty() || Faction_match[1].str().empty()) {
			//empty
		} else {
			std::string Faction_str = Faction_match[1];
			std::regex Faction_list_regex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", regex::icase);
			std::sregex_iterator Faction_iterator(Faction_str.begin(), Faction_str.end(), Faction_list_regex);
			std::sregex_iterator Faction_end;
			while (Faction_iterator != Faction_end) {
				std::string avif = (*Faction_iterator)[1].str();
				avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
				avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

				if (avif == "none") {
					break;
				}

				Faction_before_eq.push_back(avif);
				Faction_min_values.push_back(std::stof((*Faction_iterator)[2]));
				if ((*Faction_iterator)[3] != "") {
					Faction_max_values.push_back(std::stof((*Faction_iterator)[3]));
				} else {
					Faction_max_values.push_back(std::stof((*Faction_iterator)[2]));
				}
				std::string val1 = ((*Faction_iterator)[2]);
				std::string val2 = ((*Faction_iterator)[3] != "") ? ((*Faction_iterator)[3]) : ((*Faction_iterator)[2]);
				//logger::info(FMT_STRING("avif: {}"), avif);
				//logger::info(FMT_STRING("value1: {}"), val1);
				//logger::info(FMT_STRING("value2: {}"), val2);
				++Faction_iterator;
			}
			l.factionsToAdd = Faction_before_eq;
			l.factionsToAddRank1 = Faction_min_values;
			l.factionsToAddRank2 = Faction_max_values;
		}


		extractForms(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		extractForms(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

		extractForms(line, "factionsToRemove\\s*=([^:]+)", l.factionsToRemove);

		extractForms(line, "perksToAdd\\s*=([^:]+)", l.perksToAdd);

		std::regex objectsToAdd_regex("objectsToAdd\\s*=([^:]+)", regex::icase);
		std::smatch objectsToAdd_match;
		regexSearchParameter(line, objectsToAdd_match, objectsToAdd_regex);
		std::vector<std::string> objectsToAdd;
		if (objectsToAdd_match.empty() || objectsToAdd_match[1].str().empty()) {
			//empty
		} else {
			std::string objectsToAdd_str = objectsToAdd_match[1];
			std::regex pattern("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([^,]+)", regex::icase);

			auto begin = std::sregex_iterator(objectsToAdd_str.begin(), objectsToAdd_str.end(), pattern);
			auto end = std::sregex_iterator();

			for (std::sregex_iterator i = begin; i != end; ++i) {
				std::smatch match = *i;
				l.objectsToAdd.push_back(match[1]);
				l.objectsToAddValue.push_back(match[2]);
				//logger::debug(FMT_STRING("Match: {} {}"), match[1].str(), match[2].str());
			}
		}

		extractForms(line, "ObjectsToRemove\\s*=([^:]+)", l.objectsToRemove);

		extractValueString(line, "outfitDefault\\s*=([^:]+)", l.outfitDefault);

		extractValueString(line, "outfitSleep\\s*=([^:]+)", l.outfitSleep);

		extractValueString(line, "deathItem\\s*=([^:]+)", l.deathItem);

		extractValueString(line, "skin\\s*=([^:]+)", l.skin);

		extractValueString(line, "race\\s*=([^:]+)", l.race);

		extractValueString(line, "powerArmorStand\\s*=([^:]+)", l.powerArmorStand);

		extractValueString(line, "class\\s*=([^:]+)", l.Class);

		extractValueString(line, "xpValueOffset\\s*=([^:]+)", l.xpValueOffset);

		extractValueString(line, "level\\s*=([^:]+)", l.level);

		extractValueString(line, "calcLevelMin\\s*=([^:]+)", l.calcLevelMin);

		extractValueString(line, "calcLevelMax\\s*=([^:]+)", l.calcLevelMax);

		extractForms(line, "spellsToAdd\\s*=([^:]+)", l.spellsToAdd);

		extractValueString(line, "filterByGender\\s*=([^:]+)", l.isFemale);

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

				// extract chanceRobCo
		std::regex chanceRobCo_regex("chance\\s*=([^:]+)", regex::icase);
		std::smatch match;
		regexSearchParameter(line, match, chanceRobCo_regex);
		// extract the value after the equals sign
		if (match.empty() || match[1].str().empty()) {
			l.chanceRobCo = "none";
		} else {
			std::string value = match[1].str();
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			l.chanceRobCo = value;
		}

		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		logger::debug(FMT_STRING("npcs: {} races: {}  keywords: {}  avifs: {} keywordsToAdd: {} perksToAdd {}"), l.object.size(), l.races.size(), l.keywords.size(), l.avifs.size(), l.keywordsToAdd.size(), l.perksToAdd.size());
		//logger::info("returning patch instructions");
		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
	const auto& NPCArray = dataHandler->GetFormArray<RE::TESNPC>();

		for (const auto& line : tokens) {
			//logger::info("processing config line");
			for (const auto& curobj : NPCArray) {
				if (!curobj) {
					continue;
				}
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool factionAnd = false;
				bool factionOr = false;
				//curobj.

				if (curobj->IsDeleted()) {
					continue;
				}

				if (!FormMatchesModNames(curobj, line.modNames)) {
					continue;
				}

				if (!line.object.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.object) {
						RE::TESForm* currentform = nullptr;
						RE::TESNPC* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kNPC_) {
							npc = (RE::TESNPC*)currentform;

							if (curobj->formID == npc->formID) {
								found = true;
								//logger::info("NPC found.");
								break;
							}
						}
					}
				}

				if (!line.filterClass.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.filterClass) {
						RE::TESForm* currentform = nullptr;
						RE::TESClass* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kCLAS) {
							npc = (RE::TESClass*)currentform;

							if (curobj->cl && curobj->cl->formID == npc->formID) {
								found = true;
								//logger::info("NPC found.");
								break;
							}
						}
					}
				}

				
				if (!found && !line.races.empty() && curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::info("race not empty");
					for (const auto& racestring : line.races) {
						RE::TESForm* currentform = nullptr;
						RE::TESRace* race = nullptr;

						std::string string_form = racestring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kRACE) {
							race = (RE::TESRace*)currentform;

							if (curobj->formRace == race) {
								found = true;
								//logger::debug(FMT_STRING("Found a matching npc by race({:08X} {}). {:08X} {}"), race->formID, race->fullName, curobj->formID, curobj->fullName );
								break;
							}
						}
					}
				}

				if (!line.keywords.empty() && curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywords) {
						RE::TESForm* currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword) || (curobj->baseTemplateForm == nullptr && curobj->formRace && curobj->formRace->HasKeyword(keyword))) {
								keywordAnd = true;
							} else {
								keywordAnd = false;
								//logger::debug(FMT_STRING("KeywordAnd npc does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd npc true"));
						}
					}
				} else if (curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					keywordAnd = true;
				}
				if (!line.keywordsOr.empty() && curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsOr) {
						RE::TESForm* currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword) || (curobj->baseTemplateForm == nullptr && curobj->formRace && curobj->formRace->HasKeyword(keyword))) {
								keywordOr = true;
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {:08X} race {:08X}"), curobj->formID, keyword->formID, curobj->formRace->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				} else if (curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					keywordOr = true;
				}

				if (!line.filterFactions.empty() && curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::info("filterByFaction not empty");
					for (const auto& filterByFactiontring : line.filterFactions) {
						RE::TESForm* currentform = nullptr;
						RE::TESFaction* keyword = nullptr;

						std::string string_form = filterByFactiontring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kFACT) {
							keyword = (RE::TESFaction*)currentform;

							if (VRCompat::hasFaction(curobj, keyword)) {
								factionAnd = true;
							} else {
								factionAnd = false;
								//logger::debug(FMT_STRING("KeywordAnd npc does not have all filterByFaction"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd npc true"));
						}
					}
				} else if (curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					factionAnd = true;
				}

				if (!line.filterFactionsOr.empty() && curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.filterFactionsOr) {
						RE::TESForm* currentform = nullptr;
						RE::TESFaction* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kFACT) {
							keyword = (RE::TESFaction*)currentform;

							if (VRCompat::hasFaction(curobj, keyword)) {
								factionOr = true;
								//logger::debug(FMT_STRING("NPC has faction {:08X}"), curobj->formID, curobj->fullName);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				} else if (curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					factionOr = true;
				}




				if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
					logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if ((!line.filterFactions.empty() || !line.filterFactionsOr.empty()) && factionAnd && factionOr) {
					//logger::debug(FMT_STRING("Found a matching npc by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && line.object.empty() && line.races.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.filterClass.empty() && line.filterFactions.empty() && line.filterFactionsOr.empty() && curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					found = true;
					//logger::debug(FMT_STRING("Patch Everything but skip Player {:08X}"), curobj->formID);
				}

				if (found && !line.isFemale.empty() && line.isFemale != "none") {
					std::string lowercaseIsFemale = line.isFemale;
					std::transform(lowercaseIsFemale.begin(), lowercaseIsFemale.end(), lowercaseIsFemale.begin(), [](unsigned char c) { return std::tolower(c); });

					if ((lowercaseIsFemale == "female")) {
						if (curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kFemale) {	
							found = true;
							//logger::debug(FMT_STRING("is female {:08X} {}"), curobj->formID, curobj->fullName);
						} else {
							found = false;
							//logger::debug(FMT_STRING("is not female {:08X} {}"), curobj->formID, curobj->fullName);
						}
					} else if ((lowercaseIsFemale == "male")) {
						if (!(curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kFemale)) {
							//logger::debug(FMT_STRING("is male {:08X} {}"), curobj->formID, curobj->fullName);
							found = true;
						} else {
							//logger::debug(FMT_STRING("is not male {:08X} {}"), curobj->formID, curobj->fullName);
							found = false;
						}
					}
				}

				if (!line.keywordsExcluded.empty() && curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.keywordsExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->HasKeyword(keyword) || (curobj->baseTemplateForm == nullptr && curobj->formRace && curobj->formRace->HasKeyword(keyword))) {
								found = false;
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"), keyword->formID);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (!line.filterFactionsExcluded.empty() && curobj->formID != 0x000007 && !curobj->HasKeyword(PlayerKeyword)) {
					//logger::info("factions not empty");
					for (const auto& factionstring : line.filterFactionsExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::TESFaction* keyword = nullptr;

						std::string string_form = factionstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kFACT) {
							keyword = (RE::TESFaction*)currentform;

							if (VRCompat::hasFaction(curobj, keyword)) {
								found = false;
								//logger::debug(FMT_STRING("KeywordExcluded has a keyword that is excluded.{:08X}"), keyword->formID);
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
						RE::TESNPC* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kNPC_) {
							npc = (RE::TESNPC*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								//logger::info("NPC found.");
								break;
							}
						}
					}
				}

				if (!line.chanceRobCo.empty() && line.chanceRobCo != "none") {
					int random_number = getRandomNumber();
					try {
						const int chance = std::stoi(line.chanceRobCo);
						if (random_number > chance) {
							logger::debug("Skipped {:08X} by chance {} > {}", curobj->formID, random_number, chance);
							found = false;
						}
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("NPC {:08X}: invalid chance value '{}': {}"), curobj->formID, line.chanceRobCo, e.what());
					}
				}

				if (found && ShouldSkipPatch("npc", curobj)) {
					continue;
				}

				if (found && !line.calcStats.empty() && line.calcStats != "none") {
					if (line.calcStats == "yes" || line.calcStats == "true") {
						curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
						logger::debug(FMT_STRING("Enabled kAutoCalcStats for NPC {:08X} {}"), curobj->formID, curobj->fullName);
					} else if (line.calcStats == "no" || line.calcStats == "false") {
						curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kAutoCalcStats);
						logger::debug(FMT_STRING("Disabled kAutoCalcStats for NPC {:08X} {}"), curobj->formID, curobj->fullName);
					}
				}

				if (found && !line.pcMultFlag.empty() && curobj->formID != 0x000007) {
					if (toLowerCase(line.pcMultFlag[0]) == "yes" || toLowerCase(line.pcMultFlag[0]) == "true") {
						curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
						logger::debug(FMT_STRING("Enabled kPCLevelMult for NPC {:08X} {}"), curobj->formID, curobj->fullName);
					} else if (curobj->actorData.actorBaseFlags & RE::ACTOR_BASE_DATA::Flag::kPCLevelMult && (toLowerCase(line.pcMultFlag[0]) == "no" || toLowerCase(line.pcMultFlag[0]) == "false")) {
						curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kPCLevelMult);
						if (!line.PCvalues1.empty()) {
							try {
								curobj->actorData.level = checkedIntegralValue<decltype(curobj->actorData.level)>(line.PCvalues1[0]);
								logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {} and set level to {}"), curobj->formID, curobj->fullName, line.PCvalues1[0]);
							} catch (const std::exception& e) {
								logger::warn(FMT_STRING("NPC {:08X}: invalid PC level '{}': {}"), curobj->formID, line.PCvalues1[0], e.what());
							}
						} else {
							logger::debug(FMT_STRING("Disabled kPCLevelMult for NPC {:08X} {}"), curobj->formID, curobj->fullName);
						}
					}
				}

				if (found && !line.kEssential.empty() && line.kEssential != "none") {
					if (line.kEssential == "yes" || line.kEssential == "true") {
						curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kEssential);
						logger::debug(FMT_STRING("Enabled kEssential for NPC {:08X} {}"), curobj->formID, curobj->fullName);
					} else if (line.kEssential == "no" || line.kEssential == "false") {
						curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kEssential);
						logger::debug(FMT_STRING("Disabled kEssential for NPC {:08X} {}"), curobj->formID, curobj->fullName);
					}
				}

				if (found && !line.kProtected.empty() && line.kProtected != "none") {
					if (line.kProtected == "yes" || line.kProtected == "true") {
						curobj->actorData.actorBaseFlags.set(RE::ACTOR_BASE_DATA::Flag::kProtected);
						logger::debug(FMT_STRING("Enabled kProtected for NPC {:08X} {}"), curobj->formID, curobj->fullName);
					} else if (line.kProtected == "no" || line.kProtected == "false") {
						curobj->actorData.actorBaseFlags.reset(RE::ACTOR_BASE_DATA::Flag::kProtected);
						logger::debug(FMT_STRING("Disabled kProtected for NPC {:08X} {}"), curobj->formID, curobj->fullName);
					}
				}

				if (found && !line.avifs.empty()) {
					//logger::info("found! patching values");
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.avifs.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.avifs[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kAVIF) {
							//logger::info("avif valid!");
							float finalValue = 0;

							if (line.level_min == "none") {
								if (i < line.values1.size() && i < line.values2.size()) {
									finalValue = getRandomFloat(line.values1[i], line.values2[i]);
								}

							} else if (line.level_min != "none" && line.level_max != "none" && line.level_min != line.level_max && i < line.values1.size() && i < line.values2.size() && line.values1[i] != line.values2[i]) {
								int level_min = 0;  // minimum level
								int level_max = 0;  // maximum level
								try {
									level_min = std::stoi(line.level_min);
									level_max = std::stoi(line.level_max);
								} catch (const std::exception& e) {
									logger::warn(FMT_STRING("NPC {:08X}: invalid level range '{}~{}': {}"), curobj->formID, line.level_min, line.level_max, e.what());
									continue;
								}
								if (level_min < level_max) {
									const float avif_min = line.values1[i];
									const float avif_max = line.values2[i];
									//logger::info(FMT_STRING("lvl min {} level max {} avif min {} avif max {} "), level_min, level_max, line.values1[i], line.values2[i]);
									// Calculate proportional increase
									float health_increase = avif_max - avif_min;
									float level_increase = static_cast<float>(level_max - level_min);
									float proportional_increase = health_increase / level_increase;

									// Calculate health at level 50
									int target_level = curobj->actorData.level;
									finalValue = avif_min;

									//logger::info(FMT_STRING("lvl min {} level max {} avif min {} avif max {} "), level_min, level_max, line.values1[i], line.values2[i]);

									if (target_level >= level_min && target_level <= level_max) {
										float level_difference = static_cast<float>(target_level - level_min);
										finalValue = avif_min + (level_difference * proportional_increase);
									} else if (target_level < level_min) {
										finalValue = avif_min;
									} else {
										finalValue = avif_max;
									}
								} else {
									finalValue = line.values1[i];
									logger::info(FMT_STRING("Error: Level Min > Level Max  using default value for av {}"), finalValue);
								}
								//logger::info(FMT_STRING("lvl min {} level max {} avif min {} avif max {} target_level {} healthinc {} levelinc {} propIncrease {} finalValue {}"), level_min, level_max, line.values1[i], line.values2[i], target_level,health_increase, level_increase, proportional_increase, finalValue);

							} else if (i < line.values1.size()) {
								finalValue = line.values1[i];
							} else {
								logger::warn(FMT_STRING("NPC {:08X}: missing AVIF value for entry {}"), curobj->formID, i);
								continue;
							}
							
							changeAVIF_NPC(curobj, (RE::ActorValueInfo*)currentform, finalValue);
							logger::debug(FMT_STRING("npc formid: {:08X} {} changed {:08X} {} {}"), curobj->formID, curobj->fullName, ((RE::ActorValueInfo*)currentform)->formID, ((RE::ActorValueInfo*)currentform)->fullName, finalValue);
						}
					}
				}
				if (found && !line.keywordsToAdd.empty()) {
					//logger::info("found! patching values");
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.keywordsToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							curobj->AddKeyword((RE::BGSKeyword*)currentform);
							logger::debug(FMT_STRING("npc formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
						}
					}
				}

				if (found && !line.keywordsToRemove.empty()) {
					for (size_t i = 0; i < line.keywordsToRemove.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.keywordsToRemove[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							curobj->RemoveKeyword((RE::BGSKeyword*)currentform);
							logger::debug(FMT_STRING("npc formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
						}
					}
				}

				if (found && !line.factionsToRemove.empty()) {
					for (size_t i = 0; i < line.factionsToRemove.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.factionsToRemove[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kFACT) {
							VRCompat::RemoveFaction(curobj, (RE::TESFaction*)currentform);
							logger::debug(FMT_STRING("npc formid: {:08X} removed faction {:08X} {} "), curobj->formID, ((RE::TESFaction*)currentform)->formID, ((RE::TESFaction*)currentform)->fullName);
						}
					}
				}

				if (found && !line.factionsToAdd.empty()) {
					//logger::info("found! patching values");
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.factionsToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.factionsToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kFACT) {
							//logger::info("avif valid!");
							if (i >= line.factionsToAddRank1.size() || i >= line.factionsToAddRank2.size()) {
								logger::warn(FMT_STRING("NPC {:08X}: missing faction rank range for entry {}"), curobj->formID, i);
								continue;
							}
							try {
								const auto randomRank = std::floor((std::rand() / static_cast<double>(RAND_MAX)) *
									(line.factionsToAddRank2[i] - line.factionsToAddRank1[i] + 1.0) + line.factionsToAddRank1[i]);
								const auto rank = checkedIntegralValue<std::int8_t>(randomRank);
								VRCompat::AddFaction(curobj, static_cast<RE::TESFaction*>(currentform), rank);
								logger::debug(FMT_STRING("npc formid: {:08X} {} added faction {:08X} with rank {}"), curobj->formID, curobj->fullName, static_cast<RE::TESFaction*>(currentform)->formID, rank);
							} catch (const std::exception& e) {
								logger::warn(FMT_STRING("NPC {:08X}: invalid faction rank range for entry {}: {}"), curobj->formID, i, e.what());
							}
						}
					}
				}



				if (found && !line.perksToAdd.empty()) {
					//logger::info("found! patching values");
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.perksToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.perksToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kPERK) {
							curobj->AddPerk((RE::BGSPerk*)currentform, 1);
							logger::debug(FMT_STRING("npc formid: {:08X} added perk {:08X} {} "), curobj->formID, ((RE::BGSPerk*)currentform)->formID, ((RE::BGSPerk*)currentform)->fullName);
						}

					}
				}

				if (found && !line.outfitDefault.empty() && line.outfitDefault != "none") {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.outfitDefault;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kOTFT) {
						curobj->defOutfit = (RE::BGSOutfit*)currentform;
						//curobj->defOutfit->outfitItems.add
						logger::debug(FMT_STRING("npc formid: {:08X} changed outfit default to {:08X} "), curobj->formID, ((RE::BGSOutfit*)currentform)->formID);
					}
				}

				if (found && !line.outfitSleep.empty() && line.outfitSleep != "none") {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.outfitSleep;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kOTFT) {
						curobj->sleepOutfit = (RE::BGSOutfit*)currentform;
						logger::debug(FMT_STRING("npc formid: {:08X} changed outfit sleep to {:08X} "), curobj->formID, ((RE::BGSOutfit*)currentform)->formID);
					}
				}

				if (found && !line.deathItem.empty() && line.deathItem != "none") {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.deathItem;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kLVLI) {
						curobj->deathItem = (RE::TESLevItem*)currentform;
						logger::debug(FMT_STRING("npc formid: {:08X} changed deathItem to {:08X} "), curobj->formID, ((RE::TESLevItem*)currentform)->formID);
					} else if (line.deathItem == "null") {
						curobj->deathItem = nullptr;
						logger::debug(FMT_STRING("npc formid: {:08X} changed deathItem to null (none) "), curobj->formID);
					}
				}

				if (found && !line.skin.empty() && line.skin != "none") {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.skin;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kARMO) {
						curobj->formSkin = (RE::TESObjectARMO*)currentform;
						logger::debug(FMT_STRING("npc formid: {:08X} changed skin to {:08X} "), curobj->formID, ((RE::TESObjectARMO*)currentform)->formID);
					} else if (line.skin == "null") {
						curobj->formSkin = nullptr;
						logger::debug(FMT_STRING("npc formid: {:08X} changed skin to null (none) "), curobj->formID);
					}
				}

				if (found && !line.race.empty() && line.race != "none") {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.race;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kRACE) {
						curobj->formRace = (RE::TESRace*)currentform ;
						logger::debug(FMT_STRING("npc formid: {:08X} changed race to {:08X} "), curobj->formID, ((RE::TESRace*)currentform)->formID);
					}
				}

				if (found && !line.powerArmorStand.empty() && line.powerArmorStand != "none") {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.powerArmorStand;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kFURN) {
						curobj->powerArmorFurn = (RE::TESFurniture*)currentform;
						logger::debug(FMT_STRING("npc formid: {:08X} changed power armor stand to {:08X} "), curobj->formID, ((RE::TESFurniture*)currentform)->formID);
					}
					if (line.powerArmorStand == "null") {
						curobj->powerArmorFurn = nullptr;
						logger::debug(FMT_STRING("npc formid: {:08X} changed power armor stand to null (none)"), curobj->formID);
					}
				}

				if (found && !line.Class.empty() && line.Class != "none") {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.Class;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kCLAS) {
						curobj->cl = (RE::TESClass*)currentform;
						logger::debug(FMT_STRING("npc formid: {:08X} changed class to {:08X} "), curobj->formID, ((RE::TESClass*)currentform)->formID);
					}
				}
				if (found && !line.xpValueOffset.empty() && line.xpValueOffset != "none") {
					try {
						curobj->actorData.xpValueOffset = checkedIntegralValue<decltype(curobj->actorData.xpValueOffset)>(std::stod(line.xpValueOffset));
						logger::debug(FMT_STRING("npc formid: {:08X} {} changed xpValueOffset to {}"), curobj->formID, curobj->fullName, curobj->actorData.xpValueOffset);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("NPC {:08X}: invalid xpValueOffset '{}': {}"), curobj->formID, line.xpValueOffset, e.what());
					}
				}
				if (found && !line.level.empty() && line.level != "none") {
					try {
						curobj->actorData.level = checkedIntegralValue<decltype(curobj->actorData.level)>(std::stod(line.level));
						logger::debug(FMT_STRING("npc formid: {:08X} {} changed level to {}"), curobj->formID, curobj->fullName, curobj->actorData.level);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("NPC {:08X}: invalid level '{}': {}"), curobj->formID, line.level, e.what());
					}
				}
				if (found && !line.calcLevelMin.empty() && line.calcLevelMin != "none") {
					try {
						curobj->actorData.calcLevelMin = checkedIntegralValue<decltype(curobj->actorData.calcLevelMin)>(std::stod(line.calcLevelMin));
						logger::debug(FMT_STRING("npc formid: {:08X} {} changed calcLevelMin to {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMin);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("NPC {:08X}: invalid calcLevelMin '{}': {}"), curobj->formID, line.calcLevelMin, e.what());
					}
				}
				if (found && !line.calcLevelMax.empty() && line.calcLevelMax != "none") {
					try {
						curobj->actorData.calcLevelMax = checkedIntegralValue<decltype(curobj->actorData.calcLevelMax)>(std::stod(line.calcLevelMax));
						logger::debug(FMT_STRING("npc formid: {:08X} {} changed calcLevelMax to {}"), curobj->formID, curobj->fullName, curobj->actorData.calcLevelMax);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("NPC {:08X}: invalid calcLevelMax '{}': {}"), curobj->formID, line.calcLevelMax, e.what());
					}
				}

				if (found && !line.spellsToAdd.empty()) {
					//logger::info(FMT_STRING("Processing Spell list size {}"), line.spellsToAdd.size());
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.spellsToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.spellsToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kSPEL) {
							curobj->AddSpell(currentform);
							logger::debug(FMT_STRING("npc formid: {:08X} {} added spell {:08X} {} "), curobj->formID, curobj->fullName, ((RE::SpellItem*)currentform)->formID, ((RE::SpellItem*)currentform)->fullName);
						}
					}
				}

				if (found && !line.fullName.empty() && line.fullName != "none") {
					logger::debug(FMT_STRING("npc formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
					curobj->fullName = line.fullName;
				}

				if (found && !line.objectsToAdd.empty()) {
					//logger::info("found! patching values");
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.objectsToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.objectsToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							auto* bo = currentform->As<RE::TESBoundObject>();
							if (!bo || i >= line.objectsToAddValue.size()) {
								logger::warn(FMT_STRING("NPC {:08X}: invalid objectsToAdd entry '{}'"), curobj->formID, string_form);
								continue;
							}
							try {
								const int count = std::stoi(line.objectsToAddValue[i]);
								curobj->AddObject(bo, count, nullptr);
								logger::debug(FMT_STRING("npc formid: {:08X} added object {:08X} {} "), curobj->formID, bo->formID, line.objectsToAddValue[i]);
							} catch (const std::exception& e) {
								logger::warn(FMT_STRING("NPC {:08X}: invalid object count '{}' for '{}': {}"), curobj->formID, line.objectsToAddValue[i], string_form, e.what());
							}
						}
					}
				}

				if (found && !line.objectsToRemove.empty()) {
					//logger::info("found! patching values");
					//for (const auto& avifstring : line.avifs)
					for (size_t i = 0; i < line.objectsToRemove.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.objectsToRemove[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform) {
							auto* bo = currentform->As<RE::TESBoundObject>();
							if (bo) {
								VRCompat::RemoveObject(curobj, bo);
								logger::info(FMT_STRING("npc formid: {:08X} removed object {:08X} "), curobj->formID, bo->formID);
							} else {
								logger::warn(FMT_STRING("NPC {:08X}: objectsToRemove form '{}' is not a bound object"), curobj->formID, string_form);
							}
						}
					}
				}

				//if (found && !line.raceAttack.empty() && line.raceAttack != "none") {
				//	RE::TESForm* currentform = nullptr;
				//	std::string string_form = line.raceAttack;
				//	currentform = GetFormFromIdentifier(string_form);
				//	if (currentform && currentform->formType == RE::ENUM_FORM_ID::kLVLI) {
				//		curobj->race = (RE::TESObjectARMO*)currentform;
				//		logger::debug(FMT_STRING("npc formid: {:08X} changed attack race to {:08X} "), curobj->formID, ((RE::TESObjectARMO*)currentform)->formID);
				//	} else {
				//		curobj->formSkin = nullptr;
				//		logger::debug(FMT_STRING("npc formid: {:08X} changed attack race to null (none) "), curobj->formID);
				//	}
				//}
				//
				


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
								PATCH::RecordFile("npc", fullPath);
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

									PATCH::RecordRule("npc");
									tokens.push_back(create_patch_instruction(line));
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

}
