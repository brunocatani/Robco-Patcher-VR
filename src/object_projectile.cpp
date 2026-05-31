#include "object_projectile.h"

namespace PROJECTILE
{
	namespace
	{
		constexpr const char* CATEGORY = "projectile";

		bool ParseSoundLevel(const std::string& value, RE::SOUND_LEVEL& result)
		{
			const auto lower = PATCH::ToLower(PATCH::Trim(value));
			if (lower == "loud") {
				result = RE::SOUND_LEVEL::kLoud;
			} else if (lower == "normal") {
				result = RE::SOUND_LEVEL::kNormal;
			} else if (lower == "silent") {
				result = RE::SOUND_LEVEL::kSilent;
			} else if (lower == "veryloud" || lower == "very_loud" || lower == "very loud") {
				result = RE::SOUND_LEVEL::kVeryLoud;
			} else if (lower == "quiet") {
				result = RE::SOUND_LEVEL::kQuiet;
			} else {
				return false;
			}
			return true;
		}

		void SetString(line_content& instruction, const std::string& key, const std::string& value)
		{
			const auto lower = PATCH::ToLower(key);
			if (lower == "speed") {
				instruction.speed = value;
			} else if (lower == "range") {
				instruction.range = value;
			} else if (lower == "gravity") {
				instruction.gravity = value;
			} else if (lower == "force") {
				instruction.force = value;
			} else if (lower == "conespread") {
				instruction.coneSpread = value;
			} else if (lower == "collisionradius") {
				instruction.collisionRadius = value;
			} else if (lower == "lifetime") {
				instruction.lifetime = value;
			} else if (lower == "explosiontype") {
				instruction.explosionType = value;
			} else if (lower == "vatsprojectile") {
				instruction.vatsProjectile = value;
			} else if (lower == "soundlevel") {
				instruction.soundLevel = value;
			} else if (lower == "fullname") {
				instruction.fullName = value;
			} else if (lower == "filterbyprojectiles") {
				instruction.objects = PATCH::SplitList(value);
			} else if (lower == "filterbyprojectilesexcluded") {
				instruction.objectExcluded = PATCH::SplitList(value);
			} else if (PATCH::IsStrict()) {
				PATCH::RecordInvalidRule(CATEGORY, "unknown key " + key);
			} else {
				PATCH::RecordWarning(CATEGORY, "unknown key " + key);
			}
		}

		bool IsExcluded(const line_content& line, RE::BGSProjectile* projectile)
		{
			for (const auto& excluded : line.objectExcluded) {
				auto* form = GetFormFromIdentifier(excluded);
				if (form && form->formID == projectile->formID) {
					return true;
				}
			}
			return false;
		}

		void ApplyFloat(const std::string& target, const char* fieldName, float& field, const std::string& value)
		{
			if (value.empty() || PATCH::ToLower(value) == "none") {
				return;
			}

			float parsed = 0.0F;
			if (!PATCH::TryParseFloat(value, parsed)) {
				PATCH::RecordInvalidRule(CATEGORY, std::string("invalid float for ") + fieldName + ": " + value);
				return;
			}

			PATCH::RecordMutation(CATEGORY, target, fieldName, fmt::format(FMT_STRING("{}"), field), fmt::format(FMT_STRING("{}"), parsed));
			if (!PATCH::IsDryRun()) {
				field = parsed;
			}
		}

		void ApplyForm(const std::string& target, const char* fieldName, RE::BGSProjectile*& field, const std::string& value)
		{
			if (value.empty() || PATCH::ToLower(value) == "none") {
				return;
			}

			auto* form = GetFormFromIdentifier(value);
			if (!form) {
				return;
			}
			if (form->formType != RE::ENUM_FORM_ID::kPROJ) {
				PATCH::RecordWrongType(CATEGORY, value, "PROJ");
				return;
			}

			PATCH::RecordMutation(CATEGORY, target, fieldName, FormatFormID(field), FormatFormID(form));
			if (!PATCH::IsDryRun()) {
				field = static_cast<RE::BGSProjectile*>(form);
			}
		}

		void ApplyExplosion(const std::string& target, RE::BGSExplosion*& field, const std::string& value)
		{
			if (value.empty() || PATCH::ToLower(value) == "none") {
				return;
			}

			auto* form = GetFormFromIdentifier(value);
			if (!form) {
				return;
			}
			if (form->formType != RE::ENUM_FORM_ID::kEXPL) {
				PATCH::RecordWrongType(CATEGORY, value, "EXPL");
				return;
			}

			PATCH::RecordMutation(CATEGORY, target, "explosionType", FormatFormID(field), FormatFormID(form));
			if (!PATCH::IsDryRun()) {
				field = static_cast<RE::BGSExplosion*>(form);
			}
		}
	}

	line_content create_patch_instruction(const std::string& line)
	{
		line_content instruction;
		for (const auto& keyValue : PATCH::ParseKeyValues(line)) {
			SetString(instruction, keyValue.key, keyValue.value);
		}
		return instruction;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing projectile patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();

		for (const auto& line : tokens) {
			if (!line.objects.empty()) {
				for (const auto& objectString : line.objects) {
					auto* form = GetFormFromIdentifier(objectString);
					if (form && form->formType == RE::ENUM_FORM_ID::kPROJ && !IsExcluded(line, static_cast<RE::BGSProjectile*>(form))) {
						patch(line, static_cast<RE::BGSProjectile*>(form));
					}
				}
				continue;
			}

			for (const auto& projectile : dataHandler->GetFormArray<RE::BGSProjectile>()) {
				if (projectile && !IsExcluded(line, projectile)) {
					patch(line, projectile);
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

	void* patch(line_content line, RE::BGSProjectile* curobj)
	{
		if (!curobj) {
			return nullptr;
		}

		const auto target = FormatFormID(curobj);
		PATCH::RecordMatch(CATEGORY, target);

		ApplyFloat(target, "speed", curobj->data.speed, line.speed);
		ApplyFloat(target, "range", curobj->data.range, line.range);
		ApplyFloat(target, "gravity", curobj->data.gravity, line.gravity);
		ApplyFloat(target, "force", curobj->data.force, line.force);
		ApplyFloat(target, "coneSpread", curobj->data.coneSpread, line.coneSpread);
		ApplyFloat(target, "collisionRadius", curobj->data.collisionRadius, line.collisionRadius);
		ApplyFloat(target, "lifetime", curobj->data.lifetime, line.lifetime);
		ApplyExplosion(target, curobj->data.explosionType, line.explosionType);
		ApplyForm(target, "vatsProjectile", curobj->data.vatsProjectile, line.vatsProjectile);

		if (!line.soundLevel.empty() && PATCH::ToLower(line.soundLevel) != "none") {
			RE::SOUND_LEVEL soundLevel;
			if (ParseSoundLevel(line.soundLevel, soundLevel)) {
				PATCH::RecordMutation(CATEGORY, target, "soundLevel", "current", line.soundLevel);
				if (!PATCH::IsDryRun()) {
					curobj->soundLevel = soundLevel;
				}
			} else {
				PATCH::RecordInvalidRule(CATEGORY, "invalid soundLevel: " + line.soundLevel);
			}
		}

		if (!line.fullName.empty() && PATCH::ToLower(line.fullName) != "none") {
			PATCH::RecordMutation(CATEGORY, target, "fullName", curobj->fullName.c_str(), line.fullName);
			if (!PATCH::IsDryRun()) {
				curobj->fullName = line.fullName;
			}
		}

		return nullptr;
	}
}
