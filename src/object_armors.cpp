#include "object_armors.h"
#include "utility.h"
#include <cmath>
#include <limits>
#include <stdexcept>

namespace ARMORS
{
	template <class T>
	T checkedUnsignedValue(double value)
	{
		if (!std::isfinite(value) || value < 0.0 || value > static_cast<double>((std::numeric_limits<T>::max)())) {
			throw std::out_of_range("value is outside the target field range");
		}
		return static_cast<T>(value);
	}

struct patch_instruction_armor create_patch_instruction_armor(const std::string& line)
{
	patch_instruction_armor l;

	extractForms(line, "filterByArmors\\s*=([^:]+)", l.object);

	extractForms(line, "filterByArmorsExcluded\\s*=([^:]+)", l.objectExcluded);

	extractForms(line, "filterByKeywords\\s*=([^:]+)", l.keywords);

	extractForms(line, "filterByKeywordsOr\\s*=([^:]+)", l.keywordsOr);

	extractForms(line, "filterByKeywordsExcluded\\s*=([^:]+)", l.keywordsExcluded);

	extractValueString(line, "damageResist\\s*=([^:]+)", l.damageResist);
	extractValueString(line, "damageResistToAdd\\s*=([^:]+)", l.damageResistToAdd);
	extractValueString(line, "damageResistMult\\s*=([^:]+)", l.damageResistMult);

	extractValueString(line, "health\\s*=([^:]+)", l.health);

	extractValueString(line, "weight\\s*=([^:]+)", l.weight);

	extractValueString(line, "healthMult\\s*=([^:]+)", l.healthMult);

	extractValueString(line, "weightMult\\s*=([^:]+)", l.weightMult);

	extractValueString(line, "objectEffect\\s*=([^:]+)", l.objectEffect);

	extractForms(line, "keywordsToAdd\\s*=([^:]+)", l.keywordsToAdd);

	extractForms(line, "keywordsToRemove\\s*=([^:]+)", l.keywordsToRemove);

	extractForms(line, "attachParentSlotKeywordsToAdd\\s*=([^:]+)", l.attachParentSlotKeywordsToAdd);

	extractForms(line, "attachParentSlotKeywordsToRemove\\s*=([^:]+)", l.attachParentSlotKeywordsToRemove);

	extractStrings(line, "filterByBipedSlots\\s*=([^:]+)", l.bipedSlot);

	extractStrings(line, "filterByBipedSlotsOr\\s*=([^:]+)", l.bipedSlotOr);

	extractStrings(line, "filterByBipedSlotsExcluded\\s*=([^:]+)", l.bipedSlotExcluded);

	extractStrings(line, "bipedSlotsToAdd\\s*=([^:]+)", l.setBipedSlot);

	extractStrings(line, "bipedSlotsToRemove\\s*=([^:]+)", l.removeBipedSlot);

	extractMultiDataFormsFloat(line, "changeDamageTypes\\s*=([^:]+)", l.damageTypes, l.values1, l.values2);

	extractMultiDataFormsFloat(line, "changeDamageTypesByMult\\s*=([^:]+)", l.damageTypesMult, l.valuesMult1, l.valuesMult2);

	extractValueString(line, "fullName\\s*=\\s*~([^~]+?)\\s*~", l.fullName);

	extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

	extractValueString(line, "instanceNamingRule\\s*=([^:]+)", l.INRD);

	return l;
}



void process_patch_instructions_armor(const std::list<patch_instruction_armor>& tokens)
{
	logger::debug("processing patch instructions");
	const auto dataHandler = RE::TESDataHandler::GetSingleton();
	const auto& ArmorArray = dataHandler->GetFormArray<RE::TESObjectARMO>();
		for (const auto& line : tokens) {
			for (const auto& curobj : ArmorArray) {
				if (!curobj) {
					continue;
				}
				bool found = false;
			bool keywordAnd = false;
			bool keywordOr = false;
			bool bipedSlotAnd = false;
			bool bipedSlotOr = false;


			if (curobj->IsDeleted()) {
				continue;
			}

			if (!FormMatchesModNames(curobj, line.modNames)) {
				continue;
			}


			if (!line.object.empty()) {
				//logger::info("npc not empty");
				for (const auto& ammostring : line.object) {
					RE::TESForm* currentform = nullptr;
					RE::TESObjectARMO* ammo = nullptr;

					std::string string_form = ammostring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kARMO) {
						ammo = (RE::TESObjectARMO*)currentform;

						if (curobj->formID == ammo->formID) {
							found = true;
							//logger::info("NPC found.");
							break;
						}
					}
				}
			}

			if (!line.bipedSlot.empty()) {
				//logger::info("keywords not empty");
				for (const auto& bipedSlot : line.bipedSlot) {
					int iSlot = 0;
					try {
						iSlot = std::stoi(bipedSlot);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("Armor {:08X}: invalid bipedSlot filter '{}': {}"), curobj->formID, bipedSlot, e.what());
						bipedSlotAnd = false;
						continue;
					}

					auto slot = getBipedObjectSlot(iSlot);
					if (curobj->bipedModelData.bipedObjectSlots & static_cast<std::uint32_t>(slot)) {
						bipedSlotAnd = true;

					} else {
						bipedSlotAnd = false;

						break;
					}
				}

			} else {
				bipedSlotAnd = true;
			}

			if (!line.bipedSlotOr.empty()) {
				//logger::info("keywords not empty");
				for (const auto& bipedSlot : line.bipedSlotOr) {
					int iSlot = 0;
					try {
						iSlot = std::stoi(bipedSlot);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("Armor {:08X}: invalid bipedSlotOr filter '{}': {}"), curobj->formID, bipedSlot, e.what());
						continue;
					}

					auto slot = getBipedObjectSlot(iSlot);
					if (curobj->bipedModelData.bipedObjectSlots & static_cast<std::uint32_t>(slot)) {
						bipedSlotOr = true;
						break;
					}
				}

				if (found) {
					logger::debug(FMT_STRING("Armor has bipedSlot. {:08X} {}"), curobj->formID, curobj->fullName);
				}
			} else {
				bipedSlotOr = true;
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
							//logger::debug(FMT_STRING("KeywordAnd armor does not have all keywords"));
							break;
						}
						//logger::debug(FMT_STRING("KeywordAnd armor true"));
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

			if ((!line.bipedSlot.empty() || !line.bipedSlotOr.empty()) && bipedSlotAnd && bipedSlotOr) {
				//logger::debug(FMT_STRING("Found a matching armor by bipedSlots. {:08X} {}"), curobj->formID, curobj->fullName);
				found = true;
			}

			if ((!line.keywords.empty() || !line.keywordsOr.empty()) && keywordAnd && keywordOr) {
				//logger::debug(FMT_STRING("Found a matching armor by keywords. {:08X} {}"), curobj->formID, curobj->fullName);
				found = true;
			}

			if (line.object.empty() && line.keywords.empty() && line.keywordsOr.empty() && line.bipedSlot.empty() && line.bipedSlotOr.empty()) {
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

			if (!line.bipedSlotExcluded.empty()) {
				//logger::info("keywords not empty");
				for (const auto& bipedSlot : line.bipedSlotExcluded) {
					int iSlot = 0;
					try {
						iSlot = std::stoi(bipedSlot);
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("Armor {:08X}: invalid bipedSlotExcluded filter '{}': {}"), curobj->formID, bipedSlot, e.what());
						continue;
					}

					auto slot = getBipedObjectSlot(iSlot);
					if (curobj->bipedModelData.bipedObjectSlots & static_cast<std::uint32_t>(slot)) {
						found = false;
						//logger::debug(FMT_STRING("Armor Excluded has bipedSlot. {:08X} {}"), curobj->formID, curobj->fullName);
						break;
					}
					//logger::debug(FMT_STRING("KeywordAnd armor true"));
				}
			}

			if (!line.objectExcluded.empty()) {
				//logger::info("npc not empty");
				for (const auto& npcstring : line.objectExcluded) {
					RE::TESForm* currentform = nullptr;
					RE::TESObjectARMO* npc = nullptr;

					std::string string_form = npcstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kARMO) {
						npc = (RE::TESObjectARMO*)currentform;

						if (curobj->formID == npc->formID) {
							found = false;
							//logger::info("NPC found.");
							break;
						}
					}
				}
			}

			if (found && ShouldSkipPatch("armor", curobj)) {
				continue;
			}

			if (found && !line.damageResist.empty() && line.damageResist != "none") {
				try {
					curobj->armorData.rating = checkedUnsignedValue<std::uint16_t>(std::stod(line.damageResist));
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed damage resist {}"), curobj->formID, curobj->fullName, curobj->armorData.rating);
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("Armor {:08X}: invalid damageResist value '{}': {}"), curobj->formID, line.damageResist, e.what());
				}
			}

			if (found && !line.damageResistToAdd.empty() && line.damageResistToAdd != "none") {
				try {
					const auto value = static_cast<double>(curobj->armorData.rating) + std::stod(line.damageResistToAdd);
					curobj->armorData.rating = checkedUnsignedValue<std::uint16_t>(value);
					logger::debug(FMT_STRING("armor formid: {:08X} {} added damage resist {}"), curobj->formID, curobj->fullName, curobj->armorData.rating);
				}
				catch (const std::exception& e) {
					logger::warn(FMT_STRING("Armor {:08X}: invalid damageResistToAdd value '{}': {}"), curobj->formID, line.damageResistToAdd, e.what());
				}
			}

			if (found && !line.damageResistMult.empty() && line.damageResistMult != "none") {
				try {
					const auto value = static_cast<double>(curobj->armorData.rating) * std::stod(line.damageResistMult);
					curobj->armorData.rating = checkedUnsignedValue<std::uint16_t>(value);
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed(multiplied) damage resist {}"), curobj->formID, curobj->fullName, curobj->armorData.rating);
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("Armor {:08X}: invalid damageResistMult value '{}': {}"), curobj->formID, line.damageResistMult, e.what());
				}
			}

			if (found && !line.objectEffect.empty() && line.objectEffect != "none") {


				RE::TESForm* currentform = nullptr;
				RE::EnchantmentItem* obj = nullptr;

				std::string string_form = line.objectEffect;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kENCH) {
					obj = (RE::EnchantmentItem*)currentform;

					curobj->formEnchanting = obj;
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed object effect to {:08X}"), curobj->formID, curobj->fullName, obj->formID);
				} else if (toLowerCase(line.objectEffect) == "null") {
					curobj->formEnchanting = nullptr;
					logger::debug(FMT_STRING("armor formid: {:08X} {} removed object effect"), curobj->formID, curobj->fullName);
				}
				

			}

			if (found && !line.health.empty() && line.health != "none") {
				try {
					curobj->armorData.health = checkedUnsignedValue<std::uint32_t>(std::stod(line.health));
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed health {}"), curobj->formID, curobj->fullName, curobj->armorData.health);
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("Armor {:08X}: invalid health value '{}': {}"), curobj->formID, line.health, e.what());
				}
			}

			if (found && !line.healthMult.empty() && line.healthMult != "none") {
				try {
					const auto value = static_cast<double>(curobj->armorData.health) * std::stod(line.healthMult);
					curobj->armorData.health = checkedUnsignedValue<std::uint32_t>(value);
					logger::debug(FMT_STRING("armor formid: {:08X} {} multiplied health {}"), curobj->formID, curobj->fullName, curobj->armorData.health);
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("Armor {:08X}: invalid healthMult value '{}': {}"), curobj->formID, line.healthMult, e.what());
				}
			}

			if (found && !line.weight.empty() && line.weight != "none") {
				try {
					curobj->armorData.weight = stof(line.weight);
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed weight {}"), curobj->formID, curobj->fullName, curobj->armorData.weight);
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("Armor {:08X}: invalid weight value '{}': {}"), curobj->formID, line.weight, e.what());
				}
			}

			if (found && !line.weightMult.empty() && line.weightMult != "none") {
				try {
					curobj->armorData.weight = curobj->armorData.weight * stof(line.weightMult);
					logger::debug(FMT_STRING("armor formid: {:08X} {} multiplied weight {}"), curobj->formID, curobj->fullName, curobj->armorData.weight);
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("Armor {:08X}: invalid weightMult value '{}': {}"), curobj->formID, line.weightMult, e.what());
				}
			}

			if (found && !line.keywordsToAdd.empty()) {
				for (size_t i = 0; i < line.keywordsToAdd.size(); i++) {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.keywordsToAdd[i];
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
						curobj->AddKeyword((RE::BGSKeyword*)currentform);
						logger::debug(FMT_STRING("armor formid: {:08X} added keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
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
						logger::debug(FMT_STRING("armor formid: {:08X} removed keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}

			if (found && !line.attachParentSlotKeywordsToAdd.empty()) {
				for (size_t i = 0; i < line.attachParentSlotKeywordsToAdd.size(); i++) {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.attachParentSlotKeywordsToAdd[i];
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
						if (!curobj->attachParents.HasKeyword((RE::BGSKeyword*)currentform)) {
							VRCompat::AddKeywordAttachPoint(curobj->attachParents, (RE::BGSKeyword*)currentform);
							logger::debug(FMT_STRING("armor formid: {:08X} added attach parent slot keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
						}
					}
				}
			}

			if (found && !line.attachParentSlotKeywordsToRemove.empty()) {
				for (size_t i = 0; i < line.attachParentSlotKeywordsToRemove.size(); i++) {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.attachParentSlotKeywordsToRemove[i];
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
						if (curobj->attachParents.HasKeyword((RE::BGSKeyword*)currentform)) {
							VRCompat::RemoveKeyword(curobj->attachParents, (RE::BGSKeyword*)currentform);

							logger::debug(FMT_STRING("armor formid: {:08X} removed attach parent slot keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
						}
					}
				}
			}

			if (found && !line.setBipedSlot.empty()) {
				for (const auto& slotString : line.setBipedSlot) {
					try {
						const int slot = std::stoi(slotString);
						const auto slotMask = static_cast<std::uint32_t>(getBipedObjectSlot(slot));
						curobj->bipedModelData.bipedObjectSlots |= slotMask;
						logger::debug(FMT_STRING("armor added bipedSlot to {} Slot {} and all its ARMAs"), curobj->fullName, slotString);
						for (const auto& arma : curobj->modelArray) {
							if (arma.armorAddon) {
								arma.armorAddon->bipedModelData.bipedObjectSlots |= slotMask;
							}
						}
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("Armor {:08X}: invalid biped slot '{}': {}"), curobj->formID, slotString, e.what());
					}
				}
			}

			if (found && !line.removeBipedSlot.empty()) {
				for (const auto& slotString : line.removeBipedSlot) {
					try {
						const int slot = std::stoi(slotString);
						const auto slotMask = static_cast<std::uint32_t>(getBipedObjectSlot(slot));
						curobj->bipedModelData.bipedObjectSlots &= ~slotMask;
						logger::debug(FMT_STRING("armor removed bipedSlot to {} Slot {} and all its ARMAs"), curobj->fullName, slotString);
						for (const auto& arma : curobj->modelArray) {
							if (arma.armorAddon) {
								arma.armorAddon->bipedModelData.bipedObjectSlots &= ~slotMask;
							}
						}
					} catch (const std::exception& e) {
						logger::warn(FMT_STRING("Armor {:08X}: invalid biped slot '{}': {}"), curobj->formID, slotString, e.what());
					}
				}
			}

			if (found && !line.damageTypes.empty()) {
				//logger::info("found! patching values");
				//for (const auto& avifstring : line.avifs)
				for (size_t i = 0; i < line.damageTypes.size(); i++) {
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.damageTypes[i];
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kDMGT) {
						//logger::info("avif valid!");
						float finalValue = 0;

						if (i < line.values1.size() && i < line.values2.size()) {
							finalValue = getRandomFloat(line.values1[i], line.values2[i]);
						} else {
							logger::warn(FMT_STRING("Armor {:08X}: missing damageTypes value range for entry {}"), curobj->formID, i);
							continue;
						}

						changeDamageType_Armor(curobj, (RE::BGSDamageType*)currentform, finalValue);
						logger::debug(FMT_STRING("armor formid: {:08X} {} changed damage type {:08X} to {}"), curobj->formID, curobj->fullName, ((RE::BGSDamageType*)currentform)->formID, finalValue);
					}
				}
			}

			if (found && !line.damageTypesMult.empty()) {
				//logger::info("found! patching values");
				//for (const auto& avifstring : line.avifs)
				for (size_t i = 0; i < line.damageTypesMult.size(); i++) {
					//logger::info("found! patching values");
					RE::TESForm* currentform = nullptr;
					std::string string_form = line.damageTypesMult[i];
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kDMGT) {
						//logger::info("avif valid!");
						float finalValue = 0;

						if (i < line.valuesMult1.size() && i < line.valuesMult2.size()) {
							finalValue = line.valuesMult1[i];
						} else {
							logger::warn(FMT_STRING("Armor {:08X}: missing damageTypesMult value for entry {}"), curobj->formID, i);
							continue;
						}

						changeDamageTypeMult_Armor(curobj, (RE::BGSDamageType*)currentform, finalValue);
						logger::debug(FMT_STRING("armor formid: {:08X} {} changed(multiplied) damage type {:08X} by {}"), curobj->formID, curobj->fullName, ((RE::BGSDamageType*)currentform)->formID, finalValue);
					}
				}
			}

			if (found && !line.fullName.empty() && line.fullName != "none") {
				logger::debug(FMT_STRING("armor formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
				curobj->fullName = line.fullName;
			}

			if (found && !line.INRD.empty()) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.INRD;
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kINNR) {
					curobj->instanceNamingRules = ((RE::BGSInstanceNamingRules*)currentform);
					logger::debug(FMT_STRING("armor formid: {:08X} {} changed InstanceNamingRules to {:08X} "), curobj->formID, curobj->fullName, currentform->formID);
				} else if (toLowerCase(line.INRD) == "null") {
					curobj->instanceNamingRules = nullptr;
					logger::debug(FMT_STRING("armor formid: {:08X} changed InstanceNamingRules to null (none) "), curobj->formID);
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
							PATCH::RecordFile("armor", fullPath);
							std::string line;
							std::ifstream infile;
							std::list<patch_instruction_armor> tokens;
							infile.open(fullPath);
							while (std::getline(infile, line)) {
								if (line.empty()) {
									continue;
								}
								if (line[0] == skipChar) {
									continue;
								}

								PATCH::RecordRule("armor");
								tokens.push_back(create_patch_instruction_armor(line));
							}
							infile.close();
							process_patch_instructions_armor(tokens);
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
