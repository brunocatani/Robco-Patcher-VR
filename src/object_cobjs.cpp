#include "object_cobjs.h"
#include <unordered_set>
namespace COBJ
{

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;

		extractForms(line, "filterByCobjs\\s*=([^:]+)", l.objects);

		extractForms(line, "filterByWorkbenchKeywordsOr\\s*=([^:]+)", l.filterWorkbenchKeyword);

		extractForms(line, "filterByCategoryKeywordsOr\\s*=([^:]+)", l.filterCategoryKeyword);

		extractForms(line, "categoryKeywordsToAdd\\s*=([^:]+)", l.categoryKeywordToAdd);

		extractForms(line, "categoryKeywordsToRemove\\s*=([^:]+)", l.categoryKeywordToRemove);

		extractValueString(line, "workbenchKeyword\\s*=([^:]+)", l.workbenchKeyword);

		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
	const auto& objectArray = dataHandler->GetFormArray<RE::BGSConstructibleObject>();
		for (const auto& line : tokens) {
			std::unordered_set<std::uint32_t> directlyPatched;
			if (!line.objects.empty()) {
				for (const auto& objectstring : line.objects) {
					RE::TESForm* currentform = nullptr;
					RE::BGSConstructibleObject* object = nullptr;

					std::string string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kCOBJ) {
						object = (RE::BGSConstructibleObject*)currentform;
						if (FormMatchesModNames(object, line.modNames)) {
							patch(line, object);
							directlyPatched.insert(object->formID);
						}
					}
				}
			}

			if (!line.objects.empty() && line.filterWorkbenchKeyword.empty() && line.filterCategoryKeyword.empty()) {
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

				if (curobj->IsDeleted()) {
					continue;
				}

				if (!FormMatchesModNames(curobj, line.modNames)) {
					continue;
				}

				if (!line.filterWorkbenchKeyword.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.filterWorkbenchKeyword) {
						RE::TESForm* currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->benchKeyword && curobj->benchKeyword->formID == keyword->formID) {
								found = true;
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (!line.filterCategoryKeyword.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.filterCategoryKeyword) {
						RE::TESForm* currentform = nullptr;
						RE::BGSKeyword* keyword = nullptr;

						std::string string_form = keywordstring;
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							keyword = (RE::BGSKeyword*)currentform;

							if (curobj->filterKeywords.HasKeyword(keyword)) {
								found = true;
								//logger::debug(FMT_STRING("KeywordOr has at least one keyword true {:08X} {}"), curobj->formID, curobj->fullName);
								//logger::info("Keyword found.");
								break;
							}
						}
					}
				}

				if (line.objects.empty() && line.filterWorkbenchKeyword.empty() && line.filterCategoryKeyword.empty()) {
					found = true;
				}

				/*if (found && !line.workbenchKeyword.empty() && line.workbenchKeyword != "none") {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.workbenchKeyword;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
						curobj->benchKeyword = (RE::BGSKeyword*)currentform;
						logger::debug(FMT_STRING("cobj formid: {:08X} changed workbench keyword to {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					} else if (line.workbenchKeyword == "null") {
						curobj->benchKeyword = nullptr;
						logger::debug(FMT_STRING("cobj formid: {:08X} changed workbench keyword to null (none)"), curobj->formID);
					}
				}

				if (found && !line.categoryKeywordToAdd.empty()) {
					for (size_t i = 0; i < line.categoryKeywordToAdd.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.categoryKeywordToAdd[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							if (!curobj->filterKeywords.HasKeyword((RE::BGSKeyword*)currentform)) {
								VRCompat::AddKeywordRecipe(curobj->filterKeywords, (RE::BGSKeyword*)currentform);
								logger::debug(FMT_STRING("cobj formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
							}
							if (curobj->filterKeywords.HasKeyword((RE::BGSKeyword*)currentform)) {
								logger::debug(FMT_STRING("cobj formid: {:08X} has added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
							}
						}
					}
				}

				if (found && !line.categoryKeywordToRemove.empty()) {
					for (size_t i = 0; i < line.categoryKeywordToRemove.size(); i++) {
						RE::TESForm* currentform = nullptr;
						std::string string_form = line.categoryKeywordToRemove[i];
						currentform = GetFormFromIdentifier(string_form);
						if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
							VRCompat::RemoveKeywordRecipe(curobj->filterKeywords, (RE::BGSKeyword*)currentform);
							logger::debug(FMT_STRING("cobj formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
						}
					}
				}*/

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
								PATCH::RecordFile("constructibleobject", fullPath);
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

									PATCH::RecordRule("constructibleobject");
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

	void patch(const COBJ::line_content& line, RE::BGSConstructibleObject* curobj)
	{
		if (!curobj || ShouldSkipPatch("constructibleobject", curobj)) {
			return;
		}

		if (!line.workbenchKeyword.empty() && line.workbenchKeyword != "none") {
			RE::TESForm* currentform = nullptr;
			std::string string_form = line.workbenchKeyword;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
				curobj->benchKeyword = (RE::BGSKeyword*)currentform;
				logger::debug(FMT_STRING("cobj formid: {:08X} changed workbench keyword to {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
			} else if (line.workbenchKeyword == "null") {
				curobj->benchKeyword = nullptr;
				logger::debug(FMT_STRING("cobj formid: {:08X} changed workbench keyword to null (none)"), curobj->formID);
			}
		}

		if (!line.categoryKeywordToRemove.empty()) {
			for (size_t i = 0; i < line.categoryKeywordToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.categoryKeywordToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					VRCompat::RemoveKeywordRecipe(curobj->filterKeywords, (RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("cobj formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.categoryKeywordToAdd.empty()) {
			for (size_t i = 0; i < line.categoryKeywordToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.categoryKeywordToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					if (!curobj->filterKeywords.HasKeyword((RE::BGSKeyword*)currentform)) {
						VRCompat::AddKeywordRecipe(curobj->filterKeywords, (RE::BGSKeyword*)currentform);
						logger::debug(FMT_STRING("cobj formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
					if (curobj->filterKeywords.HasKeyword((RE::BGSKeyword*)currentform)) {
						logger::debug(FMT_STRING("cobj formid: {:08X} has added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}
		}

		return;
	}

	//void readConfig()
	//{
	//	logger::debug("Reading config and create forms...");
	//
	//	char skipChar = '/';
	//	std::string extension = ".ini";
	//	DIR* dir;
	//	struct dirent* ent;
	//	std::string folder = ".\\Data\\F4se\\Plugins\\RobCo_Patcher\\constructibleObject\\";
	//
	//	if ((dir = opendir(folder.c_str())) != NULL) {
	//		while ((ent = readdir(dir)) != NULL) {
	//			if (strcmp(ent->d_name, ".") != 0 && strcmp(ent->d_name, "..") != 0) {
	//				logger::info("*********************************");
	//				logger::info(FMT_STRING("Config file {} found."), ent->d_name);
	//				logger::info("*********************************");
	//			}
	//			std::string fileName = ent->d_name;
	//			size_t pos = fileName.find(extension);
	//			if (pos != std::string::npos) {
	//				fileName = fileName.substr(0, pos);
	//				const char* modname = fileName.c_str();
	//
	//				if ((strstr(modname, ".esp") != nullptr || strstr(modname, ".esl") != nullptr || strstr(modname, ".esm") != nullptr)) {
	//					if (!IsPluginInstalled(modname)) {
	//						logger::info(FMT_STRING("{} not found or is not a valid plugin file, skipping config file."), modname);
	//						continue;
	//					}
	//				}
	//
	//				std::string line;
	//				std::ifstream infile;
	//				std::list<line_content> tokens;
	//				infile.open(folder + ent->d_name);
	//				while (std::getline(infile, line)) {
	//					if (line[0] == skipChar) {
	//						continue;
	//					}
	//
	//					if (line.empty()) {
	//						continue;
	//					}
	//
	//					tokens.push_back(create_patch_instruction(line));
	//				}
	//				infile.close();
	//				process_patch_instructions(tokens);
	//			}
	//		}
	//		closedir(dir);
	//	} else {
	//		logger::info("Couldn't find cobj dir.");
	//	}
	//
	//	return 0;
	//}

}
