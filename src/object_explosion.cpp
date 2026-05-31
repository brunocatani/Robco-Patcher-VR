#include "object_explosion.h"

namespace EXPLOSION
{
	namespace
	{
		constexpr const char* CATEGORY = "explosion";

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

		bool ParseStagger(const std::string& value, RE::STAGGER_MAGNITUDE& result)
		{
			const auto lower = PATCH::ToLower(PATCH::Trim(value));
			if (lower == "none" || lower == "0") {
				result = RE::STAGGER_MAGNITUDE::kNone;
			} else if (lower == "small" || lower == "1") {
				result = RE::STAGGER_MAGNITUDE::kSmall;
			} else if (lower == "medium" || lower == "2") {
				result = RE::STAGGER_MAGNITUDE::kMedium;
			} else if (lower == "large" || lower == "3") {
				result = RE::STAGGER_MAGNITUDE::kLarge;
			} else if (lower == "extralarge" || lower == "extra_large" || lower == "extra large" || lower == "4") {
				result = RE::STAGGER_MAGNITUDE::kExtraLarge;
			} else {
				return false;
			}
			return true;
		}

		void SetString(line_content& instruction, const std::string& key, const std::string& value)
		{
			const auto lower = PATCH::ToLower(key);
			if (lower == "damage") {
				instruction.damage = value;
			} else if (lower == "force") {
				instruction.force = value;
			} else if (lower == "innerradius") {
				instruction.innerRadius = value;
			} else if (lower == "outerradius") {
				instruction.outerRadius = value;
			} else if (lower == "imagespaceradius") {
				instruction.imageSpaceRadius = value;
			} else if (lower == "projectilespread") {
				instruction.projectileSpread = value;
			} else if (lower == "projectilecount") {
				instruction.projectileCount = value;
			} else if (lower == "spawnprojectile") {
				instruction.spawnProjectile = value;
			} else if (lower == "soundlevel") {
				instruction.soundLevel = value;
			} else if (lower == "staggermagnitude") {
				instruction.staggerMagnitude = value;
			} else if (lower == "fullname") {
				instruction.fullName = value;
			} else if (lower == "filterbyexplosions") {
				instruction.objects = PATCH::SplitList(value);
			} else if (lower == "filterbyexplosionsexcluded") {
				instruction.objectExcluded = PATCH::SplitList(value);
			} else if (PATCH::IsStrict()) {
				PATCH::RecordInvalidRule(CATEGORY, "unknown key " + key);
			} else {
				PATCH::RecordWarning(CATEGORY, "unknown key " + key);
			}
		}

		bool IsExcluded(const line_content& line, RE::BGSExplosion* explosion)
		{
			for (const auto& excluded : line.objectExcluded) {
				auto* form = GetFormFromIdentifier(excluded);
				if (form && form->formID == explosion->formID) {
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

		void ApplyUInt(const std::string& target, const char* fieldName, std::uint32_t& field, const std::string& value)
		{
			if (value.empty() || PATCH::ToLower(value) == "none") {
				return;
			}

			std::uint32_t parsed = 0;
			if (!PATCH::TryParseUInt(value, parsed)) {
				PATCH::RecordInvalidRule(CATEGORY, std::string("invalid unsigned integer for ") + fieldName + ": " + value);
				return;
			}

			PATCH::RecordMutation(CATEGORY, target, fieldName, std::to_string(field), std::to_string(parsed));
			if (!PATCH::IsDryRun()) {
				field = parsed;
			}
		}

		void ApplyProjectile(const std::string& target, RE::BGSProjectile*& field, const std::string& value)
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

			PATCH::RecordMutation(CATEGORY, target, "spawnProjectile", FormatFormID(field), FormatFormID(form));
			if (!PATCH::IsDryRun()) {
				field = static_cast<RE::BGSProjectile*>(form);
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
		logger::debug("processing explosion patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();

		for (const auto& line : tokens) {
			if (!line.objects.empty()) {
				for (const auto& objectString : line.objects) {
					auto* form = GetFormFromIdentifier(objectString);
					if (form && form->formType == RE::ENUM_FORM_ID::kEXPL && !IsExcluded(line, static_cast<RE::BGSExplosion*>(form))) {
						patch(line, static_cast<RE::BGSExplosion*>(form));
					}
				}
				continue;
			}

			for (const auto& explosion : dataHandler->GetFormArray<RE::BGSExplosion>()) {
				if (explosion && !IsExcluded(line, explosion)) {
					patch(line, explosion);
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

	void* patch(line_content line, RE::BGSExplosion* curobj)
	{
		if (!curobj) {
			return nullptr;
		}

		const auto target = FormatFormID(curobj);
		PATCH::RecordMatch(CATEGORY, target);

		ApplyFloat(target, "damage", curobj->data.damage, line.damage);
		ApplyFloat(target, "force", curobj->data.force, line.force);
		ApplyFloat(target, "innerRadius", curobj->data.innerRadius, line.innerRadius);
		ApplyFloat(target, "outerRadius", curobj->data.outerRadius, line.outerRadius);
		ApplyFloat(target, "imageSpaceRadius", curobj->data.imageSpaceRadius, line.imageSpaceRadius);
		ApplyFloat(target, "projectileSpread", curobj->data.projectileSpread, line.projectileSpread);
		ApplyUInt(target, "projectileCount", curobj->data.projectileCount, line.projectileCount);
		ApplyProjectile(target, curobj->data.spawnProjectile, line.spawnProjectile);

		if (!line.soundLevel.empty() && PATCH::ToLower(line.soundLevel) != "none") {
			RE::SOUND_LEVEL soundLevel;
			if (ParseSoundLevel(line.soundLevel, soundLevel)) {
				PATCH::RecordMutation(CATEGORY, target, "soundLevel", "current", line.soundLevel);
				if (!PATCH::IsDryRun()) {
					curobj->data.soundLevel = soundLevel;
				}
			} else {
				PATCH::RecordInvalidRule(CATEGORY, "invalid soundLevel: " + line.soundLevel);
			}
		}

		if (!line.staggerMagnitude.empty() && PATCH::ToLower(line.staggerMagnitude) != "none") {
			RE::STAGGER_MAGNITUDE staggerMagnitude;
			if (ParseStagger(line.staggerMagnitude, staggerMagnitude)) {
				PATCH::RecordMutation(CATEGORY, target, "staggerMagnitude", "current", line.staggerMagnitude);
				if (!PATCH::IsDryRun()) {
					curobj->data.staggerMagnitude = staggerMagnitude;
				}
			} else {
				PATCH::RecordInvalidRule(CATEGORY, "invalid staggerMagnitude: " + line.staggerMagnitude);
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
