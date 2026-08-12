#include "object_alch.h"
#include "EngineAdapters.h"

namespace ALCH
{

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;

		extractForms(line, "filterByAlchs\\s*=([^:]+)", l.objects);
		extractForms(line, "filterByAlchsExcluded\\s*=([^:]+)", l.objectExcluded);
		extractForms(line, "filterByKeywords\\s*=([^:]+)", l.keywords);
		extractForms(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);
		extractForms(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);
		extractForms(line, "filterByMgefs\\s*=([^:]+)", l.mgefs);
		extractForms(line, "filterByMgefsOr\\s*=([^:]+)", l.mgefsOr);
		extractForms(line, "filterByMgefsExcluded\\s*=([^:]+)", l.mgefsExcluded);
		extractForms(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);
		extractForms(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);
		extractToArr2D(line, "mgefsToAdd\\s*=([^:]+)", l.addedObjects);
		extractToArr2D(line, "mgefsToChange\\s*=([^:]+)", l.changedObjects);
		extractForms(line, "mgefsToRemove\\s*=([^:]+)", l.removedObjects);
		extractValueString(line, "filterByType\\s*=([^:]+)", l.filterType);
		extractValueString(line, "weight\\s*=([^:]+)", l.weight);
		extractValueString(line, "value\\s*=([^:]+)", l.capsvalue);
		extractValueString(line, "clear\\s*=([^:]+)", l.clear);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+?)\\s*~", l.fullName);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		const auto& objectArray = dataHandler->GetFormArray<RE::AlchemyItem>();
		for (const auto& line : tokens) {
			if (!line.filterType.empty()) {
				logger::warn(FMT_STRING("Ingestible filterByType '{}' is not mapped to a verified FO4VR field; skipping rule"), line.filterType);
				continue;
			}
			for (const auto& curobj : objectArray) {
				if (!curobj) {
					continue;
				}
				bool found = false;
				bool keywordAnd = false;
				bool keywordOr = false;
				bool mgefAnd = false;
				bool mgefOr = false;

				if (curobj->IsDeleted()) {
					continue;
				}

				if (!FormMatchesModNames(curobj, line.modNames)) {
					continue;
				}

				if (!line.objects.empty()) {
					//logger::info("npc not empty");
					for (const auto& objectstring : line.objects) {
						RE::TESForm* currentform = nullptr;
						RE::AlchemyItem* object = nullptr;

						std::string string_form = objectstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kALCH) {
							object = (RE::AlchemyItem*)currentform;

							if (curobj->formID == object->formID) {
								found = true;
								//logger::debug("Found True");
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

				if (!line.mgefs.empty()) {
					mgefAnd = true;
					for (const auto& mgefstring : line.mgefs) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kMGEF) {
							keyword = (RE::EffectSetting*)currentform;
							bool foundInList = false;
							for (const auto& effect : curobj->listOfEffects) {
								if (effect && effect->effectSetting && effect->effectSetting->formID == keyword->formID) {
									foundInList = true;
									break;
								}
							}
							if (!foundInList) {
								mgefAnd = false;
								break;
							}
						} else {
							logger::warn(FMT_STRING("Ingestible {:08X}: invalid filterByMgefs form '{}'"), curobj->formID, mgefstring);
							mgefAnd = false;
							break;
						}
					}
				} else {
					mgefAnd = true;
				}
				if (!line.mgefsOr.empty()) {
					for (const auto& mgefstring : line.mgefsOr) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kMGEF) {
							keyword = (RE::EffectSetting*)currentform;

							for (const auto& effect : curobj->listOfEffects) {
								if (effect && effect->effectSetting && effect->effectSetting->formID == keyword->formID) {
									mgefOr = true;
									//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
									//logger::info("Keyword found.");
									break;
								}
							}
						}
						if (mgefOr) {
							break;
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					mgefOr = true;
				}

				if ((!line.mgefs.empty() || !line.mgefsOr.empty()) && mgefAnd && mgefOr) {
					//logger::debug(FMT_STRING("Found a matching weapon by mgefs. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (!found && line.objects.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.mgefs.empty() && line.mgefsOr.empty()) {
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

				if (!line.mgefsExcluded.empty()) {
					//logger::info("mgefs not empty");
					for (const auto& mgefstring : line.mgefsExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::EffectSetting* keyword = nullptr;

						std::string string_form = mgefstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kMGEF) {
							keyword = (RE::EffectSetting*)currentform;
						for (const auto& effect : curobj->listOfEffects) {
							if (effect && effect->effectSetting && effect->effectSetting->formID == keyword->formID) {
									found = false;
									//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
									//logger::info("Keyword found.");
									break;
								}
							}
						}
					}
				}

				if (!line.objectExcluded.empty()) {
					//logger::info("npc not empty");
					for (const auto& npcstring : line.objectExcluded) {
						RE::TESForm* currentform = nullptr;
						RE::AlchemyItem* npc = nullptr;

						std::string string_form = npcstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kALCH) {
							npc = (RE::AlchemyItem*)currentform;

							if (curobj->formID == npc->formID) {
								found = false;
								logger::debug(FMT_STRING("ingestible {:08X} {} excluded."), curobj->formID, curobj->fullName);
								break;
							}
						}
					}
				}

				//if (found) {
				//	for (const auto& effect : curobj->listOfEffects) {
				//	
				//		logger::debug(FMT_STRING("effect formID: {:08X} name: {}"), effect->effectSetting->formID, effect->effectSetting->fullName);
				//		logger::debug(FMT_STRING("effect magnitude: {}"), effect->data.magnitude);
				//		logger::debug(FMT_STRING("effect duration: {}"), effect->data.duration);
				//		logger::debug(FMT_STRING("effect area: {}"), effect->data.area);
				//	}

				//}

				if (found && ShouldSkipPatch("ingestible", curobj)) {
					continue;
				}

				if (found && !line.keywordsToAdd.empty()) {
					for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.keywordsToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							curobj->AddKeyword((RE::BGSKeyword*)currentform);
							logger::debug(FMT_STRING("ingestible formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
							logger::debug(FMT_STRING("ingestible formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
						}
					}
				}


				if (found && (toLowerCase(line.clear) == "true" || toLowerCase(line.clear) == "yes")) {
					curobj->listOfEffects.clear();
					logger::debug(FMT_STRING("ingestible {:08X} {} cleared all effects"), curobj->formID, curobj->fullName);
				}

				if (found && !line.removedObjects.empty()) {
					for (const auto& objectToRemove : line.removedObjects) {
						std::string removeFormStr = objectToRemove;
						RE::EffectSetting* removeForm = (RE::EffectSetting*)GetFormFromIdentifier(removeFormStr);
						if (removeForm) {
						curobj->listOfEffects.erase(std::remove_if(curobj->listOfEffects.begin(), curobj->listOfEffects.end(), [&](const RE::EffectItem* x) {
								bool removed = x && x->effectSetting && x->effectSetting->formID == removeForm->formID;
								if (removed) {
									logger::debug(FMT_STRING("ingestible {:08X} {} removed effect {:08X}"), curobj->formID, curobj->fullName, removeForm->formID);
								}
								return removed;
							}),
								curobj->listOfEffects.end());
						}
					}
				}

				if (found && !line.addedObjects.empty()) {
					for (const auto& objectToAdd : line.addedObjects) {
						if (objectToAdd.size() < 4) {
							logger::error("Invalid mgefsToAdd entry: expected form, magnitude, duration and area");
							continue;
						}
						std::string addFormStr = objectToAdd[0];
						RE::EffectSetting* addForm = (RE::EffectSetting*)GetFormFromIdentifier(addFormStr);
						if (!addForm || addForm->formType != RE::ENUM_FORM_ID::kMGEF) {
							logger::error(FMT_STRING("Invalid magic effect form in mgefsToAdd: {}"), addFormStr);
							continue;
						}
						float magnitude = 0.0f;
						int32_t duration = 0;
						int32_t area = 0;
						try {
							magnitude = std::stof(objectToAdd[1]);
							duration = std::stoi(objectToAdd[2]);
							area = std::stoi(objectToAdd[3]);
						} catch (const std::exception&) {
							logger::error(FMT_STRING("Invalid numeric value in mgefsToAdd entry for {}"), addFormStr);
							continue;
						}
						if (auto* effectItem = EngineAdapters::CreateEffectItem(addForm, magnitude, duration, area)) {
							try {
								curobj->listOfEffects.push_back(effectItem);
							} catch (...) {
								EngineAdapters::DestroyEffectItem(effectItem);
								throw;
							}
						} else {
							logger::warn(FMT_STRING("Ingestible {:08X}: failed to create magic effect item for {}"), curobj->formID, addFormStr);
							continue;
						}
						logger::debug(FMT_STRING("ingestible {:08X} {} added effect {:08X} {} magitude {} duration {} area {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, objectToAdd[1], objectToAdd[2], objectToAdd[3]);
					}
				}

				if (found && !line.changedObjects.empty()) {
					for (const auto& objectToAdd : line.changedObjects) {
						if (objectToAdd.size() < 4) {
							logger::warn(FMT_STRING("Ingestible {:08X}: invalid mgefsToChange entry; expected form, magnitude, duration and area"), curobj->formID);
							continue;
						}
						std::string addFormStr = objectToAdd[0];
						RE::EffectSetting* addForm = (RE::EffectSetting*)GetFormFromIdentifier(addFormStr);
						if (addForm && addForm->formType == RE::ENUM_FORM_ID::kMGEF) {  // Only proceed if addForm is a magic effect
							float magnitude = 0.0f;
							int32_t duration = 0;
							int32_t area = 0;
							try {
								if (objectToAdd[1] != "null") {
									magnitude = std::stof(objectToAdd[1]);
								}
								if (objectToAdd[2] != "null") {
									duration = std::stoi(objectToAdd[2]);
								}
								if (objectToAdd[3] != "null") {
									area = std::stoi(objectToAdd[3]);
								}
							} catch (const std::exception& e) {
								logger::warn(FMT_STRING("Ingestible {:08X}: invalid mgefsToChange values for '{}': {}"), curobj->formID, addFormStr, e.what());
								continue;
							}

							for (const auto& effect : curobj->listOfEffects) {
								if (effect && effect->effectSetting && effect->effectSetting->formID == addForm->formID) {
									if (objectToAdd[1] != "null") {
										effect->data.magnitude = magnitude;
										logger::debug(FMT_STRING("ingestible {:08X} {} changed magnitude of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->data.magnitude);
									}
									if (objectToAdd[2] != "null") {
										effect->data.duration = duration;
										logger::debug(FMT_STRING("ingestible {:08X} {} changed duration of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->data.duration);
									}
									if (objectToAdd[3] != "null") {
										effect->data.area = area;
										logger::debug(FMT_STRING("ingestible {:08X} {} changed area of {:08X} {} to {}"), curobj->formID, curobj->fullName, addForm->formID, addForm->fullName, effect->data.area);
									}
								}
							}
						} else if (addForm) {
							logger::warn(FMT_STRING("Ingestible {:08X}: '{}' is not a magic effect"), curobj->formID, addFormStr);
						}
					}
				}


				if (found && !line.weight.empty() && line.weight != "none") {
					try {
						curobj->weight = std::stof(line.weight);
						logger::debug(FMT_STRING("ingestible formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("Ingestible {:08X}: invalid weight '{}': {}"), curobj->formID, line.weight, e.what());
					}
				}
				if (found && !line.capsvalue.empty() && line.capsvalue != "none") {
					try {
						curobj->data.costOverride = std::stoi(line.capsvalue);
						logger::debug(FMT_STRING("ingestible formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->data.costOverride);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("Ingestible {:08X}: invalid caps value '{}': {}"), curobj->formID, line.capsvalue, e.what());
					}
				}

				if (found && !line.fullName.empty() && line.fullName != "none") {
					logger::debug(FMT_STRING("ingestible formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
					curobj->fullName = line.fullName;
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
								PATCH::RecordFile("ingestible", fullPath);
								std::string line;
								std::ifstream infile;
								std::list<line_content> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line.empty()) {
										continue;
									}
									if (line[0] == skipChar) {
										continue;
									}

									PATCH::RecordRule("ingestible");
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
