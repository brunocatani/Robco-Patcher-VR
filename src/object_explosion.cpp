#include "object_explosion.h"

#include <unordered_set>

namespace EXPLOSION
{
	namespace
	{
		template <class T, class Operation>
		void apply_number(RE::BGSExplosion* object, std::string_view field, const std::string& value, Operation&& operation)
		{
			if (value.empty() || value == "none") return;
			try {
				if constexpr (std::is_integral_v<T>) {
					size_t parsedLength = 0;
					const auto parsed = std::stoll(value, &parsedLength);
					if (parsedLength != value.size()) {
						throw std::invalid_argument("trailing characters");
					}
					if (parsed < static_cast<long long>((std::numeric_limits<T>::min)()) || parsed > static_cast<long long>((std::numeric_limits<T>::max)())) {
						throw std::out_of_range("value does not fit the target field");
					}
					operation(static_cast<T>(parsed));
				}
				else {
					size_t parsedLength = 0;
					const auto parsed = std::stof(value, &parsedLength);
					if (parsedLength != value.size() || !std::isfinite(parsed)) {
						throw std::invalid_argument("value must be finite");
					}
					operation(static_cast<T>(parsed));
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Explosion {:08X}: invalid {} value '{}': {}"), object->formID, field, value, e.what());
			}
		}

		template <class T>
		void set_form(RE::BGSExplosion* object, std::string_view field, const std::string& identifier, RE::ENUM_FORM_ID type, T*& target)
		{
			if (identifier.empty() || identifier == "none") return;
			if (toLowerCase(identifier) == "null") {
				target = nullptr;
				return;
			}
			auto* form = GetFormFromIdentifier(identifier);
			if (form && form->formType == type) target = static_cast<T*>(form);
			else logger::warn(FMT_STRING("Explosion {:08X}: invalid {} form '{}'"), object->formID, field, identifier);
		}

	}

	line_content create_patch_instruction(const std::string& line)
	{
		line_content result;
		extractForms(line, "filterByExplosions\\s*=([^:]+)", result.objects);
		extractForms(line, "filterByExplosionsExcluded\\s*=([^:]+)", result.objectExcluded);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", result.modNames);
		extractValueString(line, "projectileSpread\\s*=([^:]+)", result.projectileSpread);
		extractValueString(line, "projectileCount\\s*=([^:]+)", result.projectileCount);
		extractValueString(line, "force\\s*=([^:]+)", result.force);
		extractValueString(line, "forceMult\\s*=([^:]+)", result.forceMult);
		extractValueString(line, "damage\\s*=([^:]+)", result.damage);
		extractValueString(line, "damageToAdd\\s*=([^:]+)", result.damageToAdd);
		extractValueString(line, "damageMult\\s*=([^:]+)", result.damageMult);
		extractValueString(line, "innerRadius\\s*=([^:]+)", result.innerRadius);
		extractValueString(line, "outerRadius\\s*=([^:]+)", result.outerRadius);
		extractValueString(line, "imageSpaceRadius\\s*=([^:]+)", result.imageSpaceRadius);
		extractValueString(line, "verticalOffsetMult\\s*=([^:]+)", result.verticalOffsetMult);
		extractValueString(line, "placedObjectFadeDelay\\s*=([^:]+)", result.placedObjectFadeDelay);
		extractValueString(line, "soundLevel\\s*=([^:]+)", result.soundLevel);
		extractValueString(line, "staggerMagnitude\\s*=([^:]+)", result.staggerMagnitude);
		extractValueString(line, "projectileVectorX\\s*=([^:]+)", result.projectileVectorX);
		extractValueString(line, "projectileVectorY\\s*=([^:]+)", result.projectileVectorY);
		extractValueString(line, "projectileVectorZ\\s*=([^:]+)", result.projectileVectorZ);
		extractValueString(line, "light\\s*=([^:]+)", result.light);
		extractValueString(line, "sound1\\s*=([^:]+)", result.sound1);
		extractValueString(line, "sound2\\s*=([^:]+)", result.sound2);
		extractValueString(line, "impactDataSet\\s*=([^:]+)", result.impactDataSet);
		extractValueString(line, "impactPlacedObject\\s*=([^:]+)", result.impactPlacedObject);
		extractValueString(line, "spawnProjectile\\s*=([^:]+)", result.spawnProjectile);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+?)\\s*~", result.fullName);
		return result;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		auto* dataHandler = RE::TESDataHandler::GetSingleton(false);
		if (!dataHandler) {
			logger::error("Explosion patching skipped because TESDataHandler is unavailable");
			return;
		}
		const auto& forms = dataHandler->GetFormArray<RE::BGSExplosion>();
		for (const auto& line : tokens) {
			std::unordered_set<RE::TESFormID> excludedForms;
			for (const auto& identifier : line.objectExcluded) {
				if (auto* form = GetFormFromIdentifier(identifier)) {
					excludedForms.emplace(form->formID);
				}
			}
			if (!line.objects.empty()) {
				for (const auto& identifier : line.objects) {
					auto* form = GetFormFromIdentifier(identifier);
					if (form && form->formType == RE::ENUM_FORM_ID::kEXPL && !excludedForms.contains(form->formID)) {
						if (FormMatchesModNames(form, line.modNames)) patch(line, static_cast<RE::BGSExplosion*>(form));
					}
					else logger::warn(FMT_STRING("Explosion filter contains invalid form '{}'"), identifier);
				}
				continue;
			}
			for (auto* object : forms) {
				if (object && !object->IsDeleted() && !excludedForms.contains(object->formID) && FormMatchesModNames(object, line.modNames)) patch(line, object);
			}
		}
	}

	void patch(const line_content& line, RE::BGSExplosion* object)
	{
		if (!object || ShouldSkipPatch("explosion", object)) {
			return;
		}
		apply_number<float>(object, "projectileSpread", line.projectileSpread, [&](float v) { object->data.projectileSpread = v; });
		apply_number<std::uint32_t>(object, "projectileCount", line.projectileCount, [&](std::uint32_t v) { object->data.projectileCount = v; });
		apply_number<float>(object, "force", line.force, [&](float v) { object->data.force = v; });
		apply_number<float>(object, "forceMult", line.forceMult, [&](float v) { object->data.force *= v; });
		apply_number<float>(object, "damage", line.damage, [&](float v) { object->data.damage = v; });
		apply_number<float>(object, "damageToAdd", line.damageToAdd, [&](float v) { object->data.damage += v; });
		apply_number<float>(object, "damageMult", line.damageMult, [&](float v) { object->data.damage *= v; });
		apply_number<float>(object, "innerRadius", line.innerRadius, [&](float v) { object->data.innerRadius = v; });
		apply_number<float>(object, "outerRadius", line.outerRadius, [&](float v) { object->data.outerRadius = v; });
		apply_number<float>(object, "imageSpaceRadius", line.imageSpaceRadius, [&](float v) { object->data.imageSpaceRadius = v; });
		apply_number<float>(object, "verticalOffsetMult", line.verticalOffsetMult, [&](float v) { object->data.verticalOffsetMult = v; });
		apply_number<float>(object, "placedObjectFadeDelay", line.placedObjectFadeDelay, [&](float v) { object->data.placedObjectFadeDelay = v; });
		apply_number<std::int32_t>(object, "soundLevel", line.soundLevel, [&](std::int32_t v) {
			if (v < 0 || v > 4) {
				logger::warn(FMT_STRING("Explosion {:08X}: soundLevel {} is outside the valid range 0-4"), object->formID, v);
				return;
			}
			object->data.soundLevel = static_cast<RE::SOUND_LEVEL>(v);
		});
		apply_number<std::int32_t>(object, "staggerMagnitude", line.staggerMagnitude, [&](std::int32_t v) { object->data.staggerMagnitude = static_cast<RE::STAGGER_MAGNITUDE>(v); });
		apply_number<float>(object, "projectileVectorX", line.projectileVectorX, [&](float v) { object->data.projectileVector.x = v; });
		apply_number<float>(object, "projectileVectorY", line.projectileVectorY, [&](float v) { object->data.projectileVector.y = v; });
		apply_number<float>(object, "projectileVectorZ", line.projectileVectorZ, [&](float v) { object->data.projectileVector.z = v; });

		set_form(object, "light", line.light, RE::ENUM_FORM_ID::kLIGH, object->data.light);
		set_form(object, "sound1", line.sound1, RE::ENUM_FORM_ID::kSNDR, object->data.sound1);
		set_form(object, "sound2", line.sound2, RE::ENUM_FORM_ID::kSNDR, object->data.sound2);
		set_form(object, "impactDataSet", line.impactDataSet, RE::ENUM_FORM_ID::kIPDS, object->data.impactDataSet);
		if (!line.impactPlacedObject.empty() && line.impactPlacedObject != "none") {
			if (toLowerCase(line.impactPlacedObject) == "null") object->data.impactPlacedObject = nullptr;
			else if (auto* form = GetFormFromIdentifier(line.impactPlacedObject); form && form->As<RE::TESBoundObject>()) object->data.impactPlacedObject = form->As<RE::TESBoundObject>();
			else logger::warn(FMT_STRING("Explosion {:08X}: invalid impactPlacedObject form '{}'"), object->formID, line.impactPlacedObject);
		}
		set_form(object, "spawnProjectile", line.spawnProjectile, RE::ENUM_FORM_ID::kPROJ, object->data.spawnProjectile);
		if (!line.fullName.empty() && line.fullName != "none") object->fullName = line.fullName;
	}

	void readConfig(const std::string& folder)
	{
		DIR* dir;
		std::list<std::string> directories{ folder };
		while (!directories.empty()) {
			auto currentFolder = directories.front();
			directories.pop_front();
			if ((dir = opendir(currentFolder.c_str())) == nullptr) {
				logger::info(FMT_STRING("Couldn't open directory {}."), currentFolder);
				continue;
			}
			while (auto* ent = readdir(dir)) {
				if (strcmp(ent->d_name, ".") == 0 || strcmp(ent->d_name, "..") == 0) continue;
				std::string fullPath = currentFolder + "\\" + ent->d_name;
				struct _stat st;
				if (_stat(fullPath.c_str(), &st) == 0 && (_S_IFDIR & st.st_mode)) {
					directories.push_back(fullPath);
					continue;
				}
				if (fullPath.size() < 4 || toLowerCase(fullPath.substr(fullPath.size() - 4)) != ".ini") continue;
				PATCH::RecordFile("explosion", fullPath);
				std::ifstream infile(fullPath);
				std::list<line_content> tokens;
				std::string line;
				while (std::getline(infile, line)) {
					if (line.empty() || line[0] == '/') continue;
					PATCH::RecordRule("explosion");
					tokens.push_back(create_patch_instruction(line));
				}
				logger::info(FMT_STRING("Processing explosion config file {}"), fullPath);
				process_patch_instructions(tokens);
			}
			closedir(dir);
		}
	}
}
