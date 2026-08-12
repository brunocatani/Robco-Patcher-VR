#include "object_misc.h"
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_set>
namespace MISC
{
	std::int32_t checkedInt32(double value)
	{
		if (!std::isfinite(value) || value < static_cast<double>((std::numeric_limits<std::int32_t>::min)()) || value > static_cast<double>((std::numeric_limits<std::int32_t>::max)())) {
			throw std::out_of_range("value is outside the int32 range");
		}
		return static_cast<std::int32_t>(value);
	}

struct line_content create_patch_instruction(const std::string& line)
{
	line_content l;


	extractForms(line, "filterByMiscs\\s*=([^:]+)", l.objects);

	extractValueString(line, "filterByHasComponent\\s*=([^:]+)", l.filterByHasComponent);

	extractValueString(line, "filterByHasNoComponent\\s*=([^:]+)", l.filterByHasNoComponent);

	extractForms(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

	extractForms(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

	extractForms(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

	extractValueString(line, "weight\\s*=([^:]+)", l.weight);

	extractValueString(line, "weightMultiply\\s*=([^:]+)", l.weightMultiply);

	extractValueString(line, "value\\s*=([^:]+)", l.capsvalue);
	extractValueString(line, "valueMult\\s*=([^:]+)", l.valueMult);
	extractValueString(line, "fullName\\s*=\\s*~([^~]+?)\\s*~", l.fullName);
	extractForms(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);
	extractForms(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

	extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

	return l;
}

void process_patch_instructions(const std::list<line_content>& tokens)
{
	logger::debug("processing patch instructions");
	const auto dataHandler = RE::TESDataHandler::GetSingleton();
	const auto& objectArray = dataHandler->GetFormArray<RE::TESObjectMISC>();
	for (const auto& line : tokens) {
		std::unordered_set<std::uint32_t> directlyPatched;

		if (!line.objects.empty()) {
			//logger::info("npc not empty");
			for (const auto& objectstring : line.objects) {
				RE::TESForm* currentform = nullptr;
				RE::TESObjectMISC* object = nullptr;

				std::string string_form = objectstring;
				currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::ENUM_FORM_ID::kMISC && FormMatchesModNames(currentform, line.modNames)) {
				object = (RE::TESObjectMISC*)currentform;
				patch(line, object);
				directlyPatched.insert(object->formID);

				}
			}
		}

		if (!line.objects.empty() && line.filterByHasComponent.empty() && line.filterByHasNoComponent.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
			continue;
		}

		for (const auto& curobj : objectArray) {
			if (!curobj) {
				continue;
			}
			if (directlyPatched.contains(curobj->formID)) {
				continue;
			}

			bool found = false;
			bool keywordAnd = false;
			bool keywordOr = false;

			if (curobj->IsDeleted()) {
				continue;
			}

			if (!FormMatchesModNames(curobj, line.modNames)) {
				continue;
			}

			if (!line.filterByHasComponent.empty() && line.filterByHasComponent != "none") {
				if (curobj->componentData && curobj->componentData[0].size() > 0) {
					//logger::debug(FMT_STRING("misc formid: {:08X} {} has component data "), curobj->formID, curobj->fullName);
					found = true;
				}
			}

			if (!line.filterByHasNoComponent.empty() && line.filterByHasNoComponent != "none") {
				if (!curobj->componentData) {
					//logger::debug(FMT_STRING("misc formid: {:08X} {} has no component data "), curobj->formID, curobj->fullName);
					found = true;
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



			if (!found && line.objects.empty() && line.filterByHasComponent.empty() && line.filterByHasNoComponent.empty() && line.keywords.empty() && line.keywordsOr.empty()) {
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


			if (found) {
				patch(line, curobj);
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
								PATCH::RecordFile("misc", fullPath);
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

									PATCH::RecordRule("misc");
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

	void patch(const MISC::line_content& line, RE::TESObjectMISC* curobj)
	{
		if (!curobj || ShouldSkipPatch("misc", curobj)) {
			return;
		}

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("misc formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Misc {:08X}: invalid weight value '{}': {}"), curobj->formID, line.weight, e.what());
			}
		}

		if (!line.weightMultiply.empty() && line.weightMultiply != "none") {
			try {
				curobj->weight = curobj->weight * std::stof(line.weightMultiply);
				logger::debug(FMT_STRING("misc formid: {:08X} {} changed weight by multiplier {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Misc {:08X}: invalid weightMultiply value '{}': {}"), curobj->formID, line.weightMultiply, e.what());
			}
		}

		if (!line.capsvalue.empty() && line.capsvalue != "none") {
			try {
				curobj->value = checkedInt32(std::stod(line.capsvalue));
				logger::debug(FMT_STRING("misc formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Misc {:08X}: invalid value '{}': {}"), curobj->formID, line.capsvalue, e.what());
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = checkedInt32(static_cast<double>(curobj->value) * std::stod(line.valueMult));
				logger::debug(FMT_STRING("misc formid: {:08X} {} multiplied value to {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Misc {:08X}: invalid valueMult value '{}': {}"), curobj->formID, line.valueMult, e.what());
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			curobj->fullName = line.fullName;
		}

		for (const auto& identifier : line.keywordsToAdd) {
			auto* form = GetFormFromIdentifier(identifier);
			if (form && form->formType == RE::ENUM_FORM_ID::kKYWD) {
				auto* keyword = static_cast<RE::BGSKeyword*>(form);
				if (!curobj->HasKeyword(keyword)) {
					curobj->AddKeyword(keyword);
				}
			} else {
				logger::warn(FMT_STRING("Misc {:08X}: invalid keywordToAdd '{}'"), curobj->formID, identifier);
			}
		}

		for (const auto& identifier : line.keywordsToRemove) {
			auto* form = GetFormFromIdentifier(identifier);
			if (form && form->formType == RE::ENUM_FORM_ID::kKYWD) {
				curobj->RemoveKeyword(static_cast<RE::BGSKeyword*>(form));
			} else {
				logger::warn(FMT_STRING("Misc {:08X}: invalid keywordToRemove '{}'"), curobj->formID, identifier);
			}
		}

		return;
	}

}
