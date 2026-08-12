#include "object_projectile.h"

#include <unordered_set>

namespace PROJECTILE
{
	namespace
	{
		template <class T, class Operation>
		void apply_number(RE::BGSProjectile* object, std::string_view field, const std::string& value, Operation&& operation)
		{
			if (value.empty() || value == "none") {
				return;
			}
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
				} else {
					size_t parsedLength = 0;
					const auto parsed = std::stof(value, &parsedLength);
					if (parsedLength != value.size() || !std::isfinite(parsed)) {
						throw std::invalid_argument("value must be finite");
					}
					operation(static_cast<T>(parsed));
				}
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Projectile {:08X}: invalid {} value '{}': {}"), object->formID, field, value, e.what());
			}
		}

		template <class T>
		void set_form(RE::BGSProjectile* object, std::string_view field, const std::string& identifier, RE::ENUM_FORM_ID type, T*& target)
		{
			if (identifier.empty() || identifier == "none") {
				return;
			}
			if (toLowerCase(identifier) == "null") {
				target = nullptr;
				return;
			}
			auto* form = GetFormFromIdentifier(identifier);
			if (form && form->formType == type) {
				target = static_cast<T*>(form);
			} else {
				logger::warn(FMT_STRING("Projectile {:08X}: invalid {} form '{}'"), object->formID, field, identifier);
			}
		}

	}

	line_content create_patch_instruction(const std::string& line)
	{
		line_content result;
		extractForms(line, "filterByProjectiles\\s*=([^:]+)", result.objects);
		extractForms(line, "filterByProjectilesExcluded\\s*=([^:]+)", result.objectExcluded);
		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", result.modNames);
		extractValueString(line, "gravity\\s*=([^:]+)", result.gravity);
		extractValueString(line, "speed\\s*=([^:]+)", result.speed);
		extractValueString(line, "speedMult\\s*=([^:]+)", result.speedMult);
		extractValueString(line, "range\\s*=([^:]+)", result.range);
		extractValueString(line, "rangeMult\\s*=([^:]+)", result.rangeMult);
		extractValueString(line, "explosionProximity\\s*=([^:]+)", result.explosionProximity);
		extractValueString(line, "explosionTimer\\s*=([^:]+)", result.explosionTimer);
		extractValueString(line, "muzzleFlashDuration\\s*=([^:]+)", result.muzzleFlashDuration);
		extractValueString(line, "fadeOutTime\\s*=([^:]+)", result.fadeOutTime);
		extractValueString(line, "force\\s*=([^:]+)", result.force);
		extractValueString(line, "forceMult\\s*=([^:]+)", result.forceMult);
		extractValueString(line, "coneSpread\\s*=([^:]+)", result.coneSpread);
		extractValueString(line, "collisionRadius\\s*=([^:]+)", result.collisionRadius);
		extractValueString(line, "lifetime\\s*=([^:]+)", result.lifetime);
		extractValueString(line, "relaunchInterval\\s*=([^:]+)", result.relaunchInterval);
		extractValueString(line, "tracerFrequency\\s*=([^:]+)", result.tracerFrequency);
		extractValueString(line, "soundLevel\\s*=([^:]+)", result.soundLevel);
		extractValueString(line, "explosionType\\s*=([^:]+)", result.explosionType);
		extractValueString(line, "light\\s*=([^:]+)", result.light);
		extractValueString(line, "muzzleFlashLight\\s*=([^:]+)", result.muzzleFlashLight);
		extractValueString(line, "defaultWeaponSource\\s*=([^:]+)", result.defaultWeaponSource);
		extractValueString(line, "vatsProjectile\\s*=([^:]+)", result.vatsProjectile);
		extractValueString(line, "collisionLayer\\s*=([^:]+)", result.collisionLayer);
		extractValueString(line, "decalData\\s*=([^:]+)", result.decalData);
		extractValueString(line, "fullName\\s*=\\s*~([^~]+?)\\s*~", result.fullName);
		return result;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		auto* dataHandler = RE::TESDataHandler::GetSingleton(false);
		if (!dataHandler) {
			logger::error("Projectile patching skipped because TESDataHandler is unavailable");
			return;
		}
		const auto& forms = dataHandler->GetFormArray<RE::BGSProjectile>();
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
					if (form && form->formType == RE::ENUM_FORM_ID::kPROJ && !excludedForms.contains(form->formID)) {
						if (FormMatchesModNames(form, line.modNames)) patch(line, static_cast<RE::BGSProjectile*>(form));
					} else {
						logger::warn(FMT_STRING("Projectile filter contains invalid form '{}'"), identifier);
					}
				}
				continue;
			}
			for (auto* object : forms) {
				if (object && !object->IsDeleted() && !excludedForms.contains(object->formID) && FormMatchesModNames(object, line.modNames)) {
					patch(line, object);
				}
			}
		}
	}

	void patch(const line_content& line, RE::BGSProjectile* object)
	{
		if (!object || ShouldSkipPatch("projectile", object)) {
			return;
		}
		apply_number<float>(object, "gravity", line.gravity, [&](float v) { object->data.gravity = v; });
		apply_number<float>(object, "speed", line.speed, [&](float v) { object->data.speed = v; });
		apply_number<float>(object, "speedMult", line.speedMult, [&](float v) { object->data.speed *= v; });
		apply_number<float>(object, "range", line.range, [&](float v) { object->data.range = v; });
		apply_number<float>(object, "rangeMult", line.rangeMult, [&](float v) { object->data.range *= v; });
		apply_number<float>(object, "explosionProximity", line.explosionProximity, [&](float v) { object->data.explosionProximity = v; });
		apply_number<float>(object, "explosionTimer", line.explosionTimer, [&](float v) { object->data.explosionTimer = v; });
		apply_number<float>(object, "muzzleFlashDuration", line.muzzleFlashDuration, [&](float v) { object->data.muzzleFlashDuration = v; });
		apply_number<float>(object, "fadeOutTime", line.fadeOutTime, [&](float v) { object->data.fadeOutTime = v; });
		apply_number<float>(object, "force", line.force, [&](float v) { object->data.force = v; });
		apply_number<float>(object, "forceMult", line.forceMult, [&](float v) { object->data.force *= v; });
		apply_number<float>(object, "coneSpread", line.coneSpread, [&](float v) { object->data.coneSpread = v; });
		apply_number<float>(object, "collisionRadius", line.collisionRadius, [&](float v) { object->data.collisionRadius = v; });
		apply_number<float>(object, "lifetime", line.lifetime, [&](float v) { object->data.lifetime = v; });
		apply_number<float>(object, "relaunchInterval", line.relaunchInterval, [&](float v) { object->data.relaunchInterval = v; });
		apply_number<std::int8_t>(object, "tracerFrequency", line.tracerFrequency, [&](std::int8_t v) { object->data.tracerFrequency = v; });
		apply_number<std::int32_t>(object, "soundLevel", line.soundLevel, [&](std::int32_t v) {
			if (v < 0 || v > 4) {
				logger::warn(FMT_STRING("Projectile {:08X}: soundLevel {} is outside the valid range 0-4"), object->formID, v);
				return;
			}
			object->soundLevel = static_cast<RE::SOUND_LEVEL>(v);
		});

		set_form(object, "explosionType", line.explosionType, RE::ENUM_FORM_ID::kEXPL, object->data.explosionType);
		set_form(object, "light", line.light, RE::ENUM_FORM_ID::kLIGH, object->data.light);
		set_form(object, "muzzleFlashLight", line.muzzleFlashLight, RE::ENUM_FORM_ID::kLIGH, object->data.muzzleFlashLight);
		set_form(object, "defaultWeaponSource", line.defaultWeaponSource, RE::ENUM_FORM_ID::kWEAP, object->data.defaultWeaponSource);
		set_form(object, "vatsProjectile", line.vatsProjectile, RE::ENUM_FORM_ID::kPROJ, object->data.vatsProjectile);
		set_form(object, "collisionLayer", line.collisionLayer, RE::ENUM_FORM_ID::kCOLL, object->data.collisionLayer);
		set_form(object, "decalData", line.decalData, RE::ENUM_FORM_ID::kTXST, object->data.decalData);
		if (!line.fullName.empty() && line.fullName != "none") {
			object->fullName = line.fullName;
		}
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
				PATCH::RecordFile("projectile", fullPath);
				std::ifstream infile(fullPath);
				std::list<line_content> tokens;
				std::string line;
				while (std::getline(infile, line)) {
					if (line.empty() || line[0] == '/') continue;
					PATCH::RecordRule("projectile");
					tokens.push_back(create_patch_instruction(line));
				}
				logger::info(FMT_STRING("Processing projectile config file {}"), fullPath);
				process_patch_instructions(tokens);
			}
			closedir(dir);
		}
	}
}
