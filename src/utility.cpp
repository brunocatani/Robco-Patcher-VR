#include "utility.h"
#include <cstdint>
#include <regex>
#include <string_view>
#include <unordered_map>
#include <string>


//void RemoveOModEntry(RE::BGSMod::Attachment::Mod::Data* a_modData, uint32_t property)
//{
//	int propertiesToRemove = 0;
//
//		if (a_modData) {
//		RE::MemoryManager& mm = RE::MemoryManager::GetSingleton();
//		for (uint32_t i = 0; i < a_modData->propertyModCount; i++) {
//			auto& mod = a_modData->propertyMods[i];
//
//			if (mod.target == property) {
//				++propertiesToRemove;
//				logger::debug(FMT_STRING("propertiesToRemove {}"), propertiesToRemove);
//			}
//		}
//		logger::debug(FMT_STRING("property count {} propertiesToRemove {}"), a_modData->propertyModCount, propertiesToRemove);
//		if (a_modData->propertyModCount > 0 && propertiesToRemove > 0) {
//			auto test = a_modData->propertyMods;
//			RE::BGSMod::Property::Mod* newArray = (RE::BGSMod::Property::Mod*)mm.Allocate(sizeof(RE::BGSMod::Property::Mod*) * (a_modData->propertyModCount - propertiesToRemove), 0, false);
//			for (int i = 0, j = 0; i < a_modData->propertyModCount; ++i) {
//				const auto& kywd = a_modData->propertyMods[i];
//				if (kywd.target != property) {
//					std::memcpy(&newArray[j], &a_modData->propertyMods[i], sizeof(RE::BGSMod::Property::Mod));
//					++j;
//					logger::debug(FMT_STRING("j {}"), j);
//				}
//			}
//			mm.Deallocate(a_modData->propertyMods, false);
//
//			a_modData->propertyMods = newArray;
//			a_modData->propertyModCount - propertiesToRemove;
//		} else {
//			mm.Deallocate(a_modData->propertyMods, false);
//			a_modData->propertyMods = nullptr;
//			a_modData->propertyModCount = 0;
//		}
//	}
//}





uint32_t getPropertyFromString(std::string text)
{
	// Convert input text to lowercase
	std::transform(text.begin(), text.end(), text.begin(), ::tolower);

	// Define a hash map to map string values to integer values
	static const std::unordered_map<std::string, uint32_t> propertyMap = {
		{ "speed", 0 },
		{ "minrange", 2 },
		{ "maxrange", 3 },
		{ "attackdelaysec", 4 },
		{ "secondarydamage", 7 },
		{ "attackdamage", 28 },
		{ "value", 29 },
		{ "weight", 30 },
		{ "keywords", 31 },
		{ "aimmodelminconedegrees", 33 },
		{ "aimmodelmaxconedegrees", 34 },
		{ "aimmodelrecoilmaxdegpershot", 41 },
		{ "aimmodelrecoilmindegpershot", 42 },
		{ "aimmodelrecoilshotsforrunaway", 44 },
		{ "aimmodelconeironsightsmultiplier", 47 },
		{ "soundlevel", 59 },
		{ "ammo", 61 },
		{ "enchantments", 65 },
		{ "npcammolist", 75 },
		{ "damagetypevalues", 77 },
		{ "attackactionpointcost", 79 },
		{ "overrideprojectile", 80 },
		{ "sightedtransitionseconds", 83 },
		{ "colorremappingindex", 88 },
		{ "actorvalues", 94 }
	};

	// Look up the input text in the hash map
	auto it = propertyMap.find(text);
	if (it != propertyMap.end()) {
		return it->second;
	} else {
		return -1;
	}
}


template <typename T>
T GetOffset(const void* baseObject, int offset)
{
	return *reinterpret_cast<T*>((uintptr_t)baseObject + offset);
}

namespace
{
	constexpr std::uint8_t kSmallFileSlot = 0xFE;
	constexpr RE::TESFormID kFullFormMask = 0x00FFFFFF;
	constexpr RE::TESFormID kSmallFormMask = 0x00000FFF;
	constexpr RE::TESFormID kSmallFormPrefix = 0xFE000000;

	struct LoadedFileResolution
	{
		const RE::TESFile* file{ nullptr };
		bool isFullPlugin{ false };
		bool usedDaytripperSmallFileStorage{ false };
	};

	/*
	 * RobCo patches run after Daytripper has loaded small files, but CommonLibF4VR's
	 * normal light-plugin lookup only checks its optional compiled-file collection.
	 * Daytripper's SmallFileLoader stores VR light plugins in TESDataHandler::loadedMods[0xFE]
	 * as a BSTArray<TESFile*>. RobCo must resolve that storage directly, otherwise
	 * light WOF configs are skipped and earlier zero-force projectile patches remain active.
	 */
	bool PluginNameEquals(const RE::TESFile* file, std::string_view pluginName)
	{
		if (!file) {
			return false;
		}

		const auto filename = file->GetFilename();
		return filename.size() == pluginName.size() &&
			_strnicmp(filename.data(), pluginName.data(), pluginName.size()) == 0;
	}

	bool IsDaytripperLoaded()
	{
		return GetModuleHandleW(L"falloutvresl") || GetModuleHandleW(L"falloutvresl.dll");
	}

	RE::BSTArray<RE::TESFile*>* GetDaytripperSmallFiles(RE::TESDataHandler* dataHandler)
	{
		if (!REL::Module::IsVR() || !IsDaytripperLoaded() || !dataHandler) {
			return nullptr;
		}

		const auto vrModData = dataHandler->GetVRModData();
		if (!vrModData) {
			return nullptr;
		}

		auto* rawSmallFiles = vrModData->loadedMods[kSmallFileSlot];
		if (!rawSmallFiles) {
			return nullptr;
		}

		return reinterpret_cast<RE::BSTArray<RE::TESFile*>*>(rawSmallFiles);
	}

	const RE::TESFile* LookupDaytripperSmallFileByName(RE::TESDataHandler* dataHandler, std::string_view pluginName)
	{
		auto* smallFiles = GetDaytripperSmallFiles(dataHandler);
		if (!smallFiles) {
			return nullptr;
		}

		for (std::uint32_t i = 0; i < smallFiles->size(); ++i) {
			const auto* file = smallFiles->at(i);
			if (PluginNameEquals(file, pluginName)) {
				return file;
			}
		}

		return nullptr;
	}

	void LogDaytripperSmallFileResolution(const RE::TESFile* file)
	{
		if (!file) {
			return;
		}

		static std::unordered_map<std::string, bool> loggedPlugins;
		if (loggedPlugins.emplace(file->filename, true).second) {
			logger::info(FMT_STRING("Resolved light plugin {} through Daytripper small-file storage (FE{:03X})."),
				file->filename,
				file->smallFileCompileIndex);
		}
	}

	LoadedFileResolution ResolveLoadedFile(RE::TESDataHandler* dataHandler, std::string_view pluginName)
	{
		if (!dataHandler || pluginName.empty()) {
			return {};
		}

		if (const auto* fullPlugin = dataHandler->LookupLoadedModByName(pluginName)) {
			return { fullPlugin, true, false };
		}

		if (const auto* lightPlugin = dataHandler->LookupLoadedLightModByName(pluginName)) {
			return { lightPlugin, false, false };
		}

		if (const auto* daytripperSmallFile = LookupDaytripperSmallFileByName(dataHandler, pluginName)) {
			LogDaytripperSmallFileResolution(daytripperSmallFile);
			return { daytripperSmallFile, false, true };
		}

		return {};
	}

	RE::TESFormID BuildRuntimeFormID(const LoadedFileResolution& resolution, RE::TESFormID rawFormID)
	{
		if (!resolution.file) {
			return 0;
		}

		if (resolution.isFullPlugin) {
			return (static_cast<RE::TESFormID>(resolution.file->compileIndex) << 24) | (rawFormID & kFullFormMask);
		}

		return kSmallFormPrefix |
			((static_cast<RE::TESFormID>(resolution.file->smallFileCompileIndex) & 0x0FFF) << 12) |
			(rawFormID & kSmallFormMask);
	}
}

RE::TESForm* GetFormFromIdentifier(const std::string& identifier)
{
	auto dataHandler = RE::TESDataHandler::GetSingleton();
	if (!dataHandler) {
		return nullptr;
	}

	const auto resolvedIdentifier = PATCH::ResolveAlias(identifier);
	if (!resolvedIdentifier) {
		return nullptr;
	}

	const auto lookupIdentifier = trim(*resolvedIdentifier);
	if (lookupIdentifier.empty() || toLowerCase(lookupIdentifier) == "none") {
		return nullptr;
	}

	auto delimiter = lookupIdentifier.find('|');
	if (delimiter != std::string::npos) {
		std::string modName = trim(lookupIdentifier.substr(0, delimiter));
		std::string modForm = trim(lookupIdentifier.substr(delimiter + 1));

		try {
			const auto rawFormID = static_cast<RE::TESFormID>(std::stoul(modForm, nullptr, 16));
			const auto resolution = ResolveLoadedFile(dataHandler, modName);
			if (!resolution.file) {
				if (PATCH::IsDiagnosticsEnabled()) {
					PATCH::RecordUnresolvedForm(PATCH::ActiveCategoryOr("forms"), lookupIdentifier, "plugin is not loaded");
				}
				return nullptr;
			}

			auto* form = RE::TESForm::GetFormByID(BuildRuntimeFormID(resolution, rawFormID));
			if (!form && PATCH::IsDiagnosticsEnabled()) {
				PATCH::RecordUnresolvedForm(PATCH::ActiveCategoryOr("forms"), lookupIdentifier, "loaded form lookup returned null");
			}
			return form;
		} catch (const std::exception&) {
			if (PATCH::IsDiagnosticsEnabled()) {
				PATCH::RecordUnresolvedForm(PATCH::ActiveCategoryOr("forms"), lookupIdentifier, "invalid hex form id");
			}
			return nullptr;
		}
	}
	if (PATCH::IsDiagnosticsEnabled()) {
		PATCH::RecordUnresolvedForm(PATCH::ActiveCategoryOr("forms"), lookupIdentifier, "identifier must be PluginName|FormID or @Alias");
	}
	return nullptr;
}
bool IsPluginInstalled(const char* name)
{
	if (!name) {
		return false;
	}

	auto dataHandler = RE::TESDataHandler::GetSingleton();
	return ResolveLoadedFile(dataHandler, name).file != nullptr;
}
std::string FormatFormID(RE::TESForm* form)
{
	if (!form) {
		return "none";
	}
	return fmt::format(FMT_STRING("{:08X}"), form->formID);
}

bool ShouldSkipPatch(const std::string& category, RE::TESForm* form)
{
	const auto target = FormatFormID(form);
	PATCH::RecordMatch(category, target);
	if (PATCH::IsDryRun()) {
		PATCH::RecordSkippedMutation(category, target, "dry-run enabled");
		return true;
	}
	return false;
}

int findPositionInArray(RE::BSTArray<RE::TESForm*> pArray, RE::TESForm* form)
{
	if (pArray.empty() || !form) {
		return -1;
	}

	for (size_t i = 0; i < pArray.size(); i++) {
		if (pArray[i] == form) {
			return i;
		}
	}

	return -1;
}



bool changeAVIF_NPC(RE::TESNPC* pNPC, RE::ActorValueInfo* pActorValueInfo, float pfValue)
{
	if (pNPC->properties) {
		for (size_t i = 0; i < pNPC->properties->size(); i++) {
			if (pNPC->properties[0][i].first->formID == pActorValueInfo->formID) {
				pNPC->properties[0][i].second.f = pfValue;
				//logger::info("avif changed!");
				return true;
			}
		}
	} else {
		pNPC->properties = new RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>;
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = pActorValueInfo;
	newTuple.second.f = pfValue;
	pNPC->properties[0].push_back(newTuple);
	//logger::info("avif added!");
	return true;
}

bool changeAVIF_Race(RE::TESRace* pNPC, RE::ActorValueInfo* pActorValueInfo, float pfValue)
{
	if (pNPC->properties) {
	for (size_t i = 0; i < pNPC->properties->size(); i++) {
		if (pNPC->properties[0][i].first->formID == pActorValueInfo->formID) {
			pNPC->properties[0][i].second.f = pfValue;
			//logger::info("avif changed!");
			return true;
		}
	}
	} else {
		pNPC->properties = new RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>;
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = pActorValueInfo;
	newTuple.second.f = pfValue;
	pNPC->properties[0].push_back(newTuple);
	//logger::info("avif added!");
	return true;
}

RE::EffectItem* createNewEffectItem(RE::EffectItem* item, RE::EffectSetting* setting, float magni, int dur, int area) {

	using func_t = decltype(&createNewEffectItem);
	REL::Relocation<func_t> func{ REL::ID(1219158) };
	return func(item, setting, magni, dur, area);


}

bool changeKeyword_TESLevItem(RE::TESLevItem* pNPC, RE::BGSKeyword* pActorValueInfo, float pfValue)
{
	if (pNPC->keywordChances) {
		for (size_t i = 0; i < pNPC->keywordChances->size(); i++) {
		if (pNPC->keywordChances[0][i].first->formID == pActorValueInfo->formID) {
			pNPC->keywordChances[0][i].second.f = pfValue;
			//logger::info("avif changed!");
			return true;
		}
		}
	} else {
		pNPC->keywordChances = new RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>;
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = pActorValueInfo;
	newTuple.second.i = static_cast<uint32_t>(pfValue);
	pNPC->keywordChances[0].push_back(newTuple);
	//logger::info("avif added!");
	return true;
}

bool changeDamageType_Weapon(RE::TESObjectWEAP* object, RE::BGSDamageType* type, float pfValue)
{
	if (object->weaponData.damageTypes) {
		for (size_t i = 0; i < object->weaponData.damageTypes->size(); i++) {
			if (object->weaponData.damageTypes[0][i].first->formID == type->formID) {
				object->weaponData.damageTypes[0][i].second.i = static_cast<uint32_t>(pfValue);
				return true;
			}
		}
	} else {
		object->weaponData.damageTypes = new RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>> ;
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = type;
	newTuple.second.i = static_cast<uint32_t>(pfValue);

	object->weaponData.damageTypes[0].push_back(newTuple);
	return true;
}

bool eraseDamageType_Weapon(RE::TESObjectWEAP* object, RE::BGSDamageType* type)
{
	if (object->weaponData.damageTypes->size() > 0 && object->weaponData.damageTypes[0].size() > 0) {
	
    // Assuming object->weaponData.damageTypes is your vector
		auto& damageTypes = object->weaponData.damageTypes[0];
		int formIDToMatch = type->formID;  // The formID you want to match

		// Use std::remove_if and std::erase to remove elements based on the condition
		damageTypes.erase(
			std::remove_if(
				damageTypes.begin(),
				damageTypes.end(),
				[&](const RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>& type) {
					return type.first->formID == formIDToMatch;
				}),
			damageTypes.end());

		return true;
	}
	return false;
}

bool changeDamageType_Armor(RE::TESObjectARMO* object, RE::BGSDamageType* type, float pfValue)
{
	if (object->armorData.damageTypes) {
		for (size_t i = 0; i < object->armorData.damageTypes->size(); i++) {
			if (object->armorData.damageTypes[0][i].first->formID == type->formID) {
				object->armorData.damageTypes[0][i].second.i = static_cast<uint32_t>(pfValue);
				return true;
			}
		}
	} else {
		object->armorData.damageTypes = new RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>;
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = type;
	//newTuple.second.f = pfValue;
	newTuple.second.i = static_cast<uint32_t>(pfValue);

	object->armorData.damageTypes[0].push_back(newTuple);

	return true;
}

//Misc Items

bool hasMISC_Item(RE::TESObjectMISC* pMisc, RE::BGSComponent* component)
{


	return false;
}

bool changeMISC_Item(RE::TESObjectMISC* pMisc, RE::BGSComponent* component, float pfValue)
{
	if (pMisc->componentData) {
		for (size_t i = 0; i < pMisc->componentData->size(); i++) {
			if (pMisc->componentData[0][i].first->formID == component->formID) {
				pMisc->componentData[0][i].second.f = pfValue;
				//logger::info("avif changed!");
				return true;
			}
		}
	} else {
		pMisc->componentData = new RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>;
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = component;
	newTuple.second.f = pfValue;
	pMisc->componentData[0].push_back(newTuple);
	//logger::info("avif added!");
	return true;
}


//template <typename T>
//T* getForm(std::string form) {
//
//	RE::TESForm* currentform = nullptr;
//	currentform = GetFormFromIdentifier(form);
//
//	if (currentform) {
//		return currentform->As<T*>();
//	}
//	return nullptr;
//}

std::string to_string(RE::BipedObjectSlot slot)
{
	switch (slot) {
	case RE::BipedObjectSlot::kNone:
		return "None";
	case RE::BipedObjectSlot::kHairTop:
		return "HairTop";
	case RE::BipedObjectSlot::kHairlong:
		return "Hairlong";
	case RE::BipedObjectSlot::kBodyFaceGenHead:
		return "BodyFaceGenHead";
	case RE::BipedObjectSlot::kBody:
		return "Body";
	case RE::BipedObjectSlot::kLleftHand:
		return "LleftHand";
	case RE::BipedObjectSlot::kRightHand:
		return "RightHand";
	case RE::BipedObjectSlot::kUTorso:
		return "UTorso";
	case RE::BipedObjectSlot::kULeftArm:
		return "ULeftArm";
	case RE::BipedObjectSlot::kURrightArm:
		return "URrightArm";
	case RE::BipedObjectSlot::kULeftLeg:
		return "ULeftLeg";
	case RE::BipedObjectSlot::kURirghtLeg:
		return "URirghtLeg";
	case RE::BipedObjectSlot::kATorso:
		return "ATorso";
	case RE::BipedObjectSlot::kALeftArm:
		return "ALeftArm";
	case RE::BipedObjectSlot::kARrightArm:
		return "ARrightArm";
	case RE::BipedObjectSlot::kALeftLeg:
		return "ALeftLeg";
	case RE::BipedObjectSlot::kARightLeg:
		return "ARightLeg";
	case RE::BipedObjectSlot::kHeadband:
		return "Headband";
	case RE::BipedObjectSlot::kEyes:
		return "Eyes";
	case RE::BipedObjectSlot::kBeard:
		return "Beard";
	case RE::BipedObjectSlot::kMouth:
		return "Mouth";
	case RE::BipedObjectSlot::kNeck:
		return "Neck";
	case RE::BipedObjectSlot::kRing:
		return "Ring";
	case RE::BipedObjectSlot::kScalp:
		return "Scalp";
	case RE::BipedObjectSlot::kDecapitation:
		return "Decapitation";
	case RE::BipedObjectSlot::kUnnamed1:
		return "Unnamed1";
	case RE::BipedObjectSlot::kUnnamed2:
		return "Unnamed2";
	case RE::BipedObjectSlot::kUnnamed3:
		return "Unnamed3";
	case RE::BipedObjectSlot::kUnnamed4:
		return "Unnamed4";
	case RE::BipedObjectSlot::kUnnamed5:
		return "Unnamed5";
	case RE::BipedObjectSlot::kShield:
		return "Shield";
	case RE::BipedObjectSlot::kPipboy:
		return "Pipboy";
	case RE::BipedObjectSlot::kFX01:
		return "FX01";
	default:
		return "Unknown";
	}
}

RE::BipedObjectSlot getBipedObjectSlot(int slot)
{
	switch (slot) {
	case 0:
		return RE::BipedObjectSlot::kHairTop;
	case 1:
		return RE::BipedObjectSlot::kHairlong;
	case 2:
		return RE::BipedObjectSlot::kBodyFaceGenHead;
	case 3:
		return RE::BipedObjectSlot::kBody;
	case 4:
		return RE::BipedObjectSlot::kLleftHand;
	case 5:
		return RE::BipedObjectSlot::kRightHand;
	case 6:
		return RE::BipedObjectSlot::kUTorso;
	case 7:
		return RE::BipedObjectSlot::kULeftArm;
	case 8:
		return RE::BipedObjectSlot::kURrightArm;
	case 9:
		return RE::BipedObjectSlot::kULeftLeg;
	case 10:
		return RE::BipedObjectSlot::kURirghtLeg;
	case 11:
		return RE::BipedObjectSlot::kATorso;
	case 12:
		return RE::BipedObjectSlot::kALeftArm;
	case 13:
		return RE::BipedObjectSlot::kARrightArm;
	case 14:
		return RE::BipedObjectSlot::kALeftLeg;
	case 15:
		return RE::BipedObjectSlot::kARightLeg;
	case 16:
		return RE::BipedObjectSlot::kHeadband;
	case 17:
		return RE::BipedObjectSlot::kEyes;
	case 18:
		return RE::BipedObjectSlot::kBeard;
	case 19:
		return RE::BipedObjectSlot::kMouth;
	case 20:
		return RE::BipedObjectSlot::kNeck;
	case 21:
		return RE::BipedObjectSlot::kRing;
	case 22:
		return RE::BipedObjectSlot::kScalp;
	case 23:
		return RE::BipedObjectSlot::kDecapitation;
	case 24:
		return RE::BipedObjectSlot::kUnnamed1;
	case 25:
		return RE::BipedObjectSlot::kUnnamed2;
	case 26:
		return RE::BipedObjectSlot::kUnnamed3;
	case 27:
		return RE::BipedObjectSlot::kUnnamed4;
	case 28:
		return RE::BipedObjectSlot::kUnnamed5;
	case 29:
		return RE::BipedObjectSlot::kShield;
	case 30:
		return RE::BipedObjectSlot::kPipboy;
	case 31:
		return RE::BipedObjectSlot::kFX01;
	default:
		return RE::BipedObjectSlot::kNone;
	}
}



std::string trim(const std::string& str)
{
	size_t first = str.find_first_not_of(' ');
	if (std::string::npos == first) {
		return str;
	}
	size_t last = str.find_last_not_of(' ');
	return str.substr(first, (last - first + 1));
}

std::vector<std::string> splitRelationNumber(const std::string& input)
{
	std::regex re(R"(([<>]=?|[<>])\s*(\d+))");
	std::smatch match;
	std::vector<std::string> result;

	if (std::regex_search(input, match, re)) {
		std::string relation_sign = match[1];
		std::string number_str = match[2];
		result.push_back(number_str);
		result.push_back(relation_sign);
	} else if (std::regex_search(input, std::regex(R"(\d+)"))) {
		result.push_back(input);
	} else {
		result.push_back("none");
	}

	return result;
}

std::string toLowerCase(std::string pString) {
	std::string lowercasepString = pString;
	std::transform(lowercasepString.begin(), lowercasepString.end(), lowercasepString.begin(), [](unsigned char c) { return std::tolower(c); });

	return lowercasepString;
}
