#include "object_outfit.h"

namespace OUTFIT
{
	namespace
	{
		constexpr const char* CATEGORY = "outfit";

		bool IsTrue(const std::string& value)
		{
			const auto lower = PATCH::ToLower(PATCH::Trim(value));
			return lower == "yes" || lower == "true" || lower == "1";
		}

		void SetValue(line_content& instruction, const std::string& key, const std::string& value)
		{
			const auto lower = PATCH::ToLower(key);
			if (lower == "filterbyoutfits") {
				instruction.objects = PATCH::SplitList(value);
			} else if (lower == "filterbyoutfitsexcluded") {
				instruction.objectExcluded = PATCH::SplitList(value);
			} else if (lower == "itemstoadd") {
				instruction.itemsToAdd = PATCH::SplitList(value);
			} else if (lower == "itemstoremove") {
				instruction.itemsToRemove = PATCH::SplitList(value);
			} else if (lower == "clear") {
				instruction.clear = value;
			} else if (lower == "itemstoreplace") {
				for (const auto& replacement : PATCH::SplitList(value)) {
					const auto delimiter = replacement.find('=');
					if (delimiter == std::string::npos) {
						PATCH::RecordInvalidRule(CATEGORY, "itemsToReplace entry must be OldForm=NewForm");
						continue;
					}
					instruction.itemsToReplace.emplace_back(
						PATCH::Trim(replacement.substr(0, delimiter)),
						PATCH::Trim(replacement.substr(delimiter + 1)));
				}
			} else if (PATCH::IsStrict()) {
				PATCH::RecordInvalidRule(CATEGORY, "unknown key " + key);
			} else {
				PATCH::RecordWarning(CATEGORY, "unknown key " + key);
			}
		}

		bool IsExcluded(const line_content& line, RE::BGSOutfit* outfit)
		{
			for (const auto& excluded : line.objectExcluded) {
				auto* form = GetFormFromIdentifier(excluded);
				if (form && form->formID == outfit->formID) {
					return true;
				}
			}
			return false;
		}

		void RecordOrApplyClear(RE::BGSOutfit* outfit, const std::string& target)
		{
			PATCH::RecordMutation(CATEGORY, target, "clear", std::to_string(outfit->outfitItems.size()), "0");
			if (!PATCH::IsDryRun()) {
				outfit->outfitItems.clear();
			}
		}

		void AddItem(RE::BGSOutfit* outfit, const std::string& target, const std::string& identifier)
		{
			auto* form = GetFormFromIdentifier(identifier);
			if (!form) {
				return;
			}

			PATCH::RecordMutation(CATEGORY, target, "itemsToAdd", "none", FormatFormID(form));
			if (!PATCH::IsDryRun()) {
				outfit->outfitItems.push_back(form);
			}
		}

		void RemoveItem(RE::BGSOutfit* outfit, const std::string& target, const std::string& identifier)
		{
			auto* form = GetFormFromIdentifier(identifier);
			if (!form) {
				return;
			}

			PATCH::RecordMutation(CATEGORY, target, "itemsToRemove", FormatFormID(form), "removed");
			if (!PATCH::IsDryRun()) {
				outfit->outfitItems.erase(
					std::remove_if(outfit->outfitItems.begin(), outfit->outfitItems.end(), [&](const RE::TESForm* item) {
						return item && item->formID == form->formID;
					}),
					outfit->outfitItems.end());
			}
		}

		void ReplaceItem(RE::BGSOutfit* outfit, const std::string& target, const std::pair<std::string, std::string>& replacement)
		{
			auto* oldForm = GetFormFromIdentifier(replacement.first);
			auto* newForm = GetFormFromIdentifier(replacement.second);
			if (!oldForm || !newForm) {
				return;
			}

			PATCH::RecordMutation(CATEGORY, target, "itemsToReplace", FormatFormID(oldForm), FormatFormID(newForm));
			if (!PATCH::IsDryRun()) {
				for (auto& item : outfit->outfitItems) {
					if (item && item->formID == oldForm->formID) {
						item = newForm;
					}
				}
			}
		}
	}

	line_content create_patch_instruction(const std::string& line)
	{
		line_content instruction;
		for (const auto& keyValue : PATCH::ParseKeyValues(line)) {
			SetValue(instruction, keyValue.key, keyValue.value);
		}
		return instruction;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing outfit patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();

		for (const auto& line : tokens) {
			if (!line.objects.empty()) {
				for (const auto& outfitString : line.objects) {
					auto* form = GetFormFromIdentifier(outfitString);
					if (form && form->formType == RE::ENUM_FORM_ID::kOTFT && !IsExcluded(line, static_cast<RE::BGSOutfit*>(form))) {
						patch(line, static_cast<RE::BGSOutfit*>(form));
					}
				}
				continue;
			}

			for (const auto& outfit : dataHandler->GetFormArray<RE::BGSOutfit>()) {
				if (outfit && !IsExcluded(line, outfit)) {
					patch(line, outfit);
				}
			}
		}
	}

	void* readConfig(const std::string& folder)
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
							size_t pos = fileName.find(extension);
							if (pos != std::string::npos) {
								fileName = fileName.substr(0, pos);
								const char* modname = fileName.c_str();
								if ((strstr(modname, ".esp") != nullptr || strstr(modname, ".esl") != nullptr || strstr(modname, ".esm") != nullptr) && !IsPluginInstalled(modname)) {
									logger::info(FMT_STRING("{} not found or is not a valid plugin file, skipping config file {}."), modname, fullPath);
									continue;
								}

								logger::info(FMT_STRING("Processing config file {}... "), fullPath.c_str());
								PATCH::RecordFile(CATEGORY, fullPath);

								std::string line;
								std::ifstream infile(fullPath);
								std::list<line_content> tokens;
								std::uint32_t lineNumber = 0;
								while (std::getline(infile, line)) {
									++lineNumber;
									if (line.empty() || line[0] == skipChar) {
										continue;
									}

									PATCH::SetActiveRule(CATEGORY, fullPath, lineNumber, line);
									PATCH::RecordRule(CATEGORY);
									tokens.push_back(create_patch_instruction(line));
									PATCH::ClearActiveRule();
								}
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
		return nullptr;
	}

	void* patch(line_content line, RE::BGSOutfit* curobj)
	{
		if (!curobj) {
			return nullptr;
		}

		const auto target = FormatFormID(curobj);
		PATCH::RecordMatch(CATEGORY, target);

		if (IsTrue(line.clear)) {
			RecordOrApplyClear(curobj, target);
		}
		for (const auto& item : line.itemsToRemove) {
			RemoveItem(curobj, target, item);
		}
		for (const auto& replacement : line.itemsToReplace) {
			ReplaceItem(curobj, target, replacement);
		}
		for (const auto& item : line.itemsToAdd) {
			AddItem(curobj, target, item);
		}

		return nullptr;
	}
}
