#include "object_ammos.h"
#include <cmath>
#include <limits>
#include <stdexcept>
#include <unordered_set>
namespace AMMOS
{
	std::int32_t checkedInt32(double value)
	{
		if (!std::isfinite(value) || value < static_cast<double>((std::numeric_limits<std::int32_t>::min)()) || value > static_cast<double>((std::numeric_limits<std::int32_t>::max)())) {
			throw std::out_of_range("value is outside the int32 range");
		}
		return static_cast<std::int32_t>(value);
	}

	struct line_content create_patch_instruction_ammo(const std::string& line)
	{
		line_content l;

		extractForms(line, "filterByAmmos\\s*=([^:]+)", l.ammo);

		extractStrings(line, "filterByNameContainsAnd\\s*=([^:]+)", l.stringContainsAnd);

		extractStrings(line, "filterByNameContainsOr\\s*=([^:]+)", l.stringContainsOr);

		extractStrings(line, "filterByNameContainsExclude\\s*=([^:]+)", l.stringContainsExclude);

		extractValueString(line, "filterByWeightLessThan\\s*=([^:]+)", l.weightLessThan);

		extractValueString(line, "ammoCategory\\s*=([^:]+)", l.type);

		extractValueString(line, "weight\\s*=([^:]+)", l.weight);

		extractValueString(line, "attackDamage\\s*=([^:]+)", l.damage);
		extractValueString(line, "attackDamageToAdd\\s*=([^:]+)", l.attackDamageToAdd);
		extractValueString(line, "attackDamageMult\\s*=([^:]+)", l.attackDamageMult);

		extractValueString(line, "value\\s*=([^:]+)", l.value);

		extractValueString(line, "valueMult\\s*=([^:]+)", l.valueMult);

		extractValueString(line, "setNewProjectile\\s*=([^:]+)", l.projectile);

		extractForms(line, "addToFormList\\s*=([^:]+)", l.formList);

		extractValueString(line, "fullName\\s*=\\s*~([^~]+?)\\s*~", l.fullName);

		extractForms(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		return l;
	}

	void process_patch_instructions_ammo(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
	const auto& AmmoArray = dataHandler->GetFormArray<RE::TESAmmo>();
		for (const auto& line : tokens) {
			std::unordered_set<std::uint32_t> directlyPatched;

			if (!line.ammo.empty()) {
				//logger::info("npc not empty");
				for (const auto& ammostring : line.ammo) {
					RE::TESForm* currentform = nullptr;
					RE::TESAmmo* ammo = nullptr;

					std::string string_form = ammostring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kAMMO) {
						ammo = (RE::TESAmmo*)currentform;
						if (FormMatchesModNames(ammo, line.modNames)) {
							patch(line, ammo);
							directlyPatched.insert(ammo->formID);
						}
					}
				}
			}

			if (!line.ammo.empty() && !line.stringContainsAnd.empty() && !line.stringContainsOr.empty()) {
				//logger::info("continue");
				continue;
			}

			for (const auto& curobj : AmmoArray) {
				if (!curobj) {
					continue;
				}
				if (directlyPatched.contains(curobj->formID)) {
					continue;
				}
				//logger::debug("Mod: {} || Weapon Type: {} || Damage: {} || Projectile: {} || addToFormList: {}", line.ammo.c_str(), line.type.c_str(), line.damage.c_str(), line.projectile.c_str(), line.armorPenetration.c_str());
				bool found = false;
				bool stringAnd = false;
				bool stringOr = false;
				//logger::info("Bad");


				if (curobj->IsDeleted()) {
					continue;
				}

				if (!FormMatchesModNames(curobj, line.modNames)) {
					continue;
				}


				if (!line.weightLessThan.empty() && line.weightLessThan != "none") {
					try {
						if (curobj->weight < std::stof(line.weightLessThan)) {
							found = true;
						}
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("Ammo {:08X}: invalid weightLessThan value '{}': {}"), curobj->formID, line.weightLessThan, e.what());
					}
				}

				if (!line.stringContainsAnd.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.stringContainsAnd) {
						std::string searchString = keywordstring;
						std::string fullname = curobj->fullName.c_str();
						std::transform(searchString.begin(), searchString.end(), searchString.begin(), [](unsigned char c) { return std::tolower(c); });
						std::transform(fullname.begin(), fullname.end(), fullname.begin(), [](unsigned char c) { return std::tolower(c); });
						if (fullname.find(searchString) != std::string::npos) {
							stringAnd = true;
							//logger::info("OMOD found.");
						} else {
							stringAnd = false;
							break;
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordAnd is empty, we pass true."));
					stringAnd = true;
				}
				if (!line.stringContainsOr.empty()) {
					//logger::info("keywords not empty");
					for (const auto& keywordstring : line.stringContainsOr) {
						std::string searchString = keywordstring;
						std::string fullname = curobj->fullName.c_str();
						std::transform(searchString.begin(), searchString.end(), searchString.begin(), [](unsigned char c) { return std::tolower(c); });
						std::transform(fullname.begin(), fullname.end(), fullname.begin(), [](unsigned char c) { return std::tolower(c); });
						if (fullname.find(searchString) != std::string::npos) {
							stringOr = true;
							break;
						}
					}
				} else {
					//logger::debug(FMT_STRING("KeywordOr is empty, we pass true."));
					stringOr = true;
				}

				if ((!line.stringContainsAnd.empty() || !line.stringContainsOr.empty()) && stringAnd && stringOr) {
					//logger::debug(FMT_STRING("Found true. {:08X} {}"), curobj->formID, curobj->fullName);
					found = true;
				}

				if (line.ammo.empty() && line.weightLessThan.empty() && line.stringContainsAnd.empty() && line.stringContainsOr.empty()) {
					found = true;
				}

				if (!line.stringContainsExclude.empty()) {
					for (const auto& keywordstring : line.stringContainsExclude) {
						std::string searchString = keywordstring;
						std::string fullname = curobj->fullName.c_str();
						std::transform(searchString.begin(), searchString.end(), searchString.begin(), [](unsigned char c) { return std::tolower(c); });
						std::transform(fullname.begin(), fullname.end(), fullname.begin(), [](unsigned char c) { return std::tolower(c); });

						if (fullname.find(searchString) != std::string::npos) {
							found = false;
							//logger::debug(FMT_STRING("omod propertyExcluded {:08X} {}"), curobj->formID, curobj->fullName);
							break;
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
								PATCH::RecordFile("ammo", fullPath);
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

									PATCH::RecordRule("ammo");
									tokens.push_back(create_patch_instruction_ammo(line));
								}
								infile.close();
								process_patch_instructions_ammo(tokens);
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

	void patch(const AMMOS::line_content& line, RE::TESAmmo* curobj) {
		if (!curobj || ShouldSkipPatch("ammo", curobj)) {
			return;
		}

		if (!line.damage.empty() && line.damage != "none") {
			try {
				curobj->data.damage = std::stof(line.damage);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed damage {}"), curobj->formID, curobj->fullName, curobj->data.damage);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Ammo {:08X}: invalid damage value '{}': {}"), curobj->formID, line.damage, e.what());
			}
		}

		if (!line.attackDamageToAdd.empty() && line.attackDamageToAdd != "none") {
			try {
				curobj->data.damage = curobj->data.damage + std::stof(line.attackDamageToAdd);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} added damage {}"), curobj->formID, curobj->fullName, line.attackDamageToAdd);
			}
			catch (const std::exception& e) {
				logger::warn(FMT_STRING("Ammo {:08X}: invalid attackDamageToAdd value '{}': {}"), curobj->formID, line.attackDamageToAdd, e.what());
			}
		}

		if (!line.attackDamageMult.empty() && line.attackDamageMult != "none") {
			try {
				curobj->data.damage = curobj->data.damage * std::stof(line.attackDamageMult);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} multiplied attackDamage to {}"), curobj->formID, curobj->fullName, curobj->data.damage);
			}
			catch (const std::exception& e) {
				logger::warn(FMT_STRING("Ammo {:08X}: invalid attackDamageMult value '{}': {}"), curobj->formID, line.attackDamageMult, e.what());
			}
		}

		if (!line.value.empty() && line.value != "none") {
			try {
				curobj->value = checkedInt32(std::stod(line.value));
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Ammo {:08X}: invalid value '{}': {}"), curobj->formID, line.value, e.what());
			}
		}

		if (!line.valueMult.empty() && line.valueMult != "none") {
			try {
				curobj->value = checkedInt32(static_cast<double>(curobj->value) * std::stod(line.valueMult));
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed(multiplied) value {}"), curobj->formID, curobj->fullName, curobj->value);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Ammo {:08X}: invalid valueMult value '{}': {}"), curobj->formID, line.valueMult, e.what());
			}
		}

		if (!line.type.empty() && line.type != "none") {
			if (line.type == "pistol") {
				listPistol.push_back(curobj);
				logger::debug(FMT_STRING("ammo {:08X} {} set to Pistol FormList {}"), curobj->formID, curobj->fullName, line.type.c_str());
			} else if (line.type == "rifle") {
				listRifle.push_back(curobj);
				logger::debug(FMT_STRING("ammo {:08X} {} set to Rifle FormList {}"), curobj->formID, curobj->fullName, line.type.c_str());
			}
		}

		if (!line.projectile.empty() && line.projectile != "none") {
			RE::TESForm* projectileform = nullptr;
			RE::BGSProjectile* currentprojectile = nullptr;
			std::string string_form = line.projectile.c_str();
			projectileform = GetFormFromIdentifier(string_form);
			if (projectileform && projectileform->formType == RE::ENUM_FORM_ID::kPROJ) {
				currentprojectile = (RE::BGSProjectile*)projectileform;
				curobj->data.projectile = currentprojectile;
				logger::debug(FMT_STRING("projectile set to {:08X} {}"), currentprojectile->formID, currentprojectile->fullName);
			} else {
				//logger::debug("Projectile not set.");
			}
		}

		if (!line.keywordsToAdd.empty()) {
			for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.keywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					curobj->AddKeyword((RE::BGSKeyword*)currentform);
					logger::debug(FMT_STRING("weapon formid: {:08X} {} added keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
				}
			}
		}

		if (!line.formList.empty()) {
			//logger::info("found! patching values");
			//for (const auto& avifstring : line.avifs)
			for (size_t i = 0; i < line.formList.size(); i++) {
				RE::TESForm* currentform = nullptr;
				RE::BGSListForm* listForm = nullptr;

				std::string string_form = line.formList[i].c_str();
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kFLST) {
					listForm = (RE::BGSListForm*)currentform;
					listForm->arrayOfForms.push_back(curobj);
					logger::debug(FMT_STRING("added ammo {:08X} {} to FormList: {:08X}"), curobj->formID, curobj->fullName, listForm->formID);
				} else {
					//logger::info("Armor Penetration not set.");
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			try {
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Ammo {:08X}: failed to change full name: {}"), curobj->formID, e.what());
			}
		}

		if (!line.weight.empty() && line.weight != "none") {
			try {
				curobj->weight = std::stof(line.weight);
				logger::debug(FMT_STRING("ammo formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->weight);
			} catch (const std::exception& e) {
				logger::warn(FMT_STRING("Ammo {:08X}: invalid weight value '{}': {}"), curobj->formID, line.weight, e.what());
			}
		}
		return;
	}

}
