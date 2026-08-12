#include "object_races.h"

namespace RACES
{

	struct patch_instruction create_patch_instruction(const std::string& line)
	{
		patch_instruction l;


		extractForms(line, "filterByRaces\\s*=([^:]+)", l.object);

		extractForms(line, "filterByRacesExcluded\\s*=([^:]+)", l.objectExcluded);

		extractForms(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

		extractForms(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

		extractForms(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

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

				try {
					const float minValue = std::stof((*avifs_iterator)[2]);
					const float maxValue = (*avifs_iterator)[3] != "" ? std::stof((*avifs_iterator)[3]) : minValue;
					avifs_before_eq.push_back(avif);
					avifs_min_values.push_back(minValue);
					avifs_max_values.push_back(maxValue);
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("Invalid changeAVIFS value for '{}': {}"), avif, e.what());
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

		extractForms(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		extractForms(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

		extractForms(line, "spellsToAdd\\s*=([^:]+)", l.spellsToAdd);

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

		logger::debug(FMT_STRING("races: {}  keywords: {}  avifs: {} keywordsToAdd: {} spellsToAdd {}"), l.object.size(), l.keywords.size(), l.avifs.size(), l.keywordsToAdd.size(), l.spellsToAdd.size());
		//logger::info("returning patch instructions");
		return l;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
	const auto& RaceArray = dataHandler->GetFormArray<RE::TESRace>();

		for (const auto& line : tokens) {
			//logger::info("processing config line");
			for (const auto& curobj : RaceArray) {
				if (!curobj) {
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

				if (!line.object.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.object) {
						RE::TESForm* currentform = nullptr;
						RE::TESRace* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kRACE) {
							npc = (RE::TESRace*)currentform;

							if (curobj->formID == npc->formID) {
								found = true;
								//logger::info("NPC found.");
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
								//logger::debug(FMT_STRING("KeywordAnd race does not have all keywords"));
								break;
							}
							//logger::debug(FMT_STRING("KeywordAnd race true"));
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
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X}"), curobj->formID);
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
					//logger::debug(FMT_STRING("Found a matching race by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && line.object.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
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
						RE::TESRace* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kRACE) {
							npc = (RE::TESRace*)currentform;

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
						logger::warn(FMT_STRING("Race {:08X}: invalid chance value '{}': {}"), curobj->formID, line.chanceRobCo, e.what());
					}
				}

				if (found && ShouldSkipPatch("race", curobj)) {
					continue;
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
							float randomValue = 0;
							if (i < line.values1.size() && i < line.values2.size()) {
								randomValue = getRandomFloat(line.values1[i], line.values2[i]);
							} else {
								logger::warn(FMT_STRING("Race {:08X}: missing changeAVIFS value range for entry {}"), curobj->formID, i);
								continue;
							}
							changeAVIF_Race(curobj, (RE::ActorValueInfo*)currentform, randomValue);
							logger::debug(FMT_STRING("race formid: {:08X} {} changed {:08X} {} {}"), curobj->formID, curobj->formEditorID, ((RE::ActorValueInfo*)currentform)->formID, ((RE::ActorValueInfo*)currentform)->fullName, randomValue);
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
							logger::debug(FMT_STRING("race formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->formEditorID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
							logger::debug(FMT_STRING("race formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
						}
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
							logger::debug(FMT_STRING("race formid: {:08X} {} added spell {:08X} {} "), curobj->formID, curobj->formEditorID, ((RE::SpellItem*)currentform)->formID, ((RE::SpellItem*)currentform)->fullName);
						}
					}
				}
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
								PATCH::RecordFile("race", fullPath);
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

									PATCH::RecordRule("race");
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
