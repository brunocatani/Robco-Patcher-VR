#include "utility.h"
#include "EngineAdapters.h"

#include <array>
#include <cmath>
#include <cstdint>
#include <limits>
#include <map>
#include <mutex>
#include <random>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_map>
#include <unordered_set>

namespace
{
	template <class Array>
	Array* CreateEngineOwnedArray()
	{
		auto& memoryManager = RE::MemoryManager::GetSingleton();
		auto* storage = memoryManager.Allocate(sizeof(Array), alignof(Array), false);
		if (!storage) {
			return nullptr;
		}

		try {
			return std::construct_at(static_cast<Array*>(storage));
		} catch (...) {
			memoryManager.Deallocate(storage, false);
			return nullptr;
		}
	}

	const std::regex& CachedRegex(std::string_view pattern, std::regex::flag_type flags)
	{
		static std::map<std::string, std::regex> cache;
		static std::mutex cacheMutex;

		std::string key = std::to_string(static_cast<unsigned int>(flags));
		key.push_back('\0');
		key.append(pattern);

		std::scoped_lock lock(cacheMutex);
		if (const auto it = cache.find(key); it != cache.end()) {
			return it->second;
		}
		return cache.emplace(key, std::regex(std::string(pattern), flags)).first->second;
	}
}

bool regexSearchParameter(const std::string& line, std::smatch& match, const std::regex& pattern)
{
	auto searchBegin = line.cbegin();
	while (searchBegin != line.cend()) {
		std::smatch candidate;
		if (!std::regex_search(searchBegin, line.cend(), candidate, pattern)) {
			break;
		}

		auto parameterStart = static_cast<std::size_t>(std::distance(line.cbegin(), candidate[0].first));
		while (parameterStart > 0 && std::isspace(static_cast<unsigned char>(line[parameterStart - 1]))) {
			--parameterStart;
		}
		if (parameterStart == 0 || line[parameterStart - 1] == ':') {
			match = std::move(candidate);
			return true;
		}

		if (candidate[0].first == candidate[0].second) {
			if (candidate[0].second == line.cend()) {
				break;
			}
			searchBegin = std::next(candidate[0].second);
		} else {
			searchBegin = candidate[0].second;
		}
	}

	match = {};
	return false;
}

std::int32_t getPropertyFromString(std::string text, RE::ENUM_FORM_ID targetFormType)
{
	// Convert input text to lowercase
	std::transform(text.begin(), text.end(), text.begin(), [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

	// Define a hash map to map string values to integer values
	static const std::unordered_map<std::string, uint32_t> weaponProperties = {
		{ "speed", 0 }, { "reach", 1 }, { "minrange", 2 }, { "maxrange", 3 }, { "attackdelaysec", 4 },
		{ "outrangedamagemult", 6 }, { "secondarydamage", 7 }, { "criticalchargebonus", 8 }, { "hitbehavior", 9 },
		{ "rank", 10 }, { "ammocapacity", 12 }, { "type", 15 }, { "playeronly", 16 }, { "npcuseammo", 17 },
		{ "charge", 18 }, { "crime", 19 }, { "fixedrange", 20 }, { "effectondeath", 21 }, { "alternaterumble", 22 },
		{ "nonhostile", 23 }, { "ignoreresist", 24 }, { "automatic", 25 }, { "cantdrop", 26 }, { "nonplayable", 27 },
		{ "attackdamage", 28 }, { "value", 29 }, { "weight", 30 }, { "keywords", 31 }, { "aimmodelminconedegrees", 33 },
		{ "aimmodelmaxconedegrees", 34 }, { "aimmodelconeincreasepershot", 35 }, { "aimmodelconedecreasepersec", 36 },
		{ "aimmodelconedecreasedelayms", 37 }, { "aimmodelconesneakmultiplier", 38 }, { "aimmodelrecoildiminishspringforce", 39 },
		{ "aimmodelrecoildiminishsightsmult", 40 }, { "aimmodelrecoilmaxdegpershot", 41 }, { "aimmodelrecoilmindegpershot", 42 },
		{ "aimmodelrecoilhipmult", 43 }, { "aimmodelrecoilshotsforrunaway", 44 }, { "aimmodelrecoilarcdeg", 45 },
		{ "aimmodelrecoilarcrotatedeg", 46 }, { "aimmodelconeironsightsmultiplier", 47 }, { "hasscope", 48 }, { "fovmult", 49 },
		{ "fireseconds", 50 }, { "numprojectiles", 51 }, { "attacksound", 52 }, { "attacksound2d", 53 }, { "attackloop", 54 },
		{ "attackfailsound", 55 }, { "idlesound", 56 }, { "equipsound", 57 }, { "unequipsound", 58 }, { "soundlevel", 59 },
		{ "impactdataset", 60 }, { "ammo", 61 }, { "effect", 62 }, { "enchantments", 65 }, { "aimmodelbasestability", 66 },
		{ "zoomdata", 67 }, { "zoomoverlay", 68 }, { "zoomis", 69 }, { "cameraoffsetx", 70 }, { "cameraoffsety", 71 },
		{ "cameraoffsetz", 72 }, { "equipslot", 73 }, { "soundlevelmult", 74 }, { "npcaddammolist", 75 }, { "reloadspeed", 76 },
		{ "damagetypes", 77 }, { "damagetypevalues", 77 }, { "accuracybonus", 78 }, { "attackactionpointcost", 79 },
		{ "rangedoverrideprojectile", 80 }, { "overrideprojectile", 80 }, { "boltaction", 81 }, { "staggervalue", 82 },
		{ "sightedtransitionseconds", 83 }, { "fullpowerseconds", 84 }, { "holdinputtopower", 85 }, { "repeatablesinglefire", 86 },
		{ "minpowerpershot", 87 }, { "colorremappingindex", 88 }, { "criticaldamagemult", 90 }, { "fastequipsound", 91 },
		{ "disableshells", 92 }, { "chargeattack", 93 }, { "actorvalues", 94 }
	};
	static const std::unordered_map<std::string, uint32_t> armorProperties = {
		{ "blockbashimpactdata", 1 }, { "blockbashmaterial", 2 }, { "keywords", 3 }, { "weight", 4 }, { "value", 5 },
		{ "rating", 6 }, { "index", 7 }, { "damagetypes", 9 }, { "damagetypevalues", 9 }, { "actorvalues", 10 },
		{ "health", 11 }, { "colorremappingindex", 12 }, { "materialswaps", 13 }
	};
	static const std::unordered_map<std::string, uint32_t> npcProperties = {
		{ "forcedinventory", 1 }, { "sxpoffset", 2 }, { "enchantments", 3 }, { "colorremappingindex", 4 }, { "materialswaps", 5 }
	};
	const auto& propertyMap = targetFormType == RE::ENUM_FORM_ID::kARMO ? armorProperties :
		targetFormType == RE::ENUM_FORM_ID::kNPC_ ? npcProperties : weaponProperties;

	// Look up the input text in the hash map
	auto it = propertyMap.find(text);
	if (it != propertyMap.end()) {
		return static_cast<std::int32_t>(it->second);
	} else {
		return -1;
	}
}


namespace
{
	constexpr RE::TESFormID kFullFormMask = 0x00FFFFFF;
	constexpr RE::TESFormID kSmallFormMask = 0x00000FFF;
	constexpr RE::TESFormID kSmallFormPrefix = 0xFE000000;
	constexpr std::uint32_t kMaximumNormalFiles = 0xFE;
	constexpr std::uint32_t kMaximumSmallFiles = 0x1000;
	constexpr std::size_t kPluginFilenameCapacity = 260;
	constexpr std::size_t kMaximumPluginNameLength = kPluginFilenameCapacity - 1;

	using GetCompiledFileCollection = const RE::TESFileCollection* (*)();
	struct DaytripperModuleCandidate
	{
		const wchar_t* moduleName;
		std::string_view displayName;
	};
	constexpr std::array kDaytripperModuleCandidates{
		DaytripperModuleCandidate{ L"Daytripper4.dll", "Daytripper4.dll" },
		DaytripperModuleCandidate{ L"falloutvresl.dll", "falloutvresl.dll" }
	};

	bool g_formResolverInitialized = false;
	std::unordered_map<std::string, RE::TESForm*> g_formCache;

	struct LoadedFileResolution
	{
		const RE::TESFile* file{ nullptr };
		bool isFullPlugin{ false };
		bool isSmallPlugin{ false };
	};

	bool IsReadableMemory(const void* address, std::size_t bytes)
	{
		if (!address || bytes == 0) {
			return false;
		}

		MEMORY_BASIC_INFORMATION region{};
		if (VirtualQuery(address, &region, sizeof(region)) != sizeof(region) ||
			region.State != MEM_COMMIT || (region.Protect & (PAGE_GUARD | PAGE_NOACCESS)) != 0) {
			return false;
		}

		const auto protection = region.Protect & 0xFF;
		const bool readable = protection == PAGE_READONLY || protection == PAGE_READWRITE ||
			protection == PAGE_WRITECOPY || protection == PAGE_EXECUTE_READ ||
			protection == PAGE_EXECUTE_READWRITE || protection == PAGE_EXECUTE_WRITECOPY;
		if (!readable) {
			return false;
		}

		const auto start = reinterpret_cast<std::uintptr_t>(address);
		const auto regionStart = reinterpret_cast<std::uintptr_t>(region.BaseAddress);
		const auto regionEnd = regionStart + region.RegionSize;
		return start >= regionStart && bytes <= regionEnd - start;
	}

	bool HasPlausibleFilename(const RE::TESFile* file)
	{
		if (!IsReadableMemory(file, sizeof(RE::TESFile))) {
			return false;
		}

		const auto length = strnlen_s(file->filename, kPluginFilenameCapacity);
		return length > 0 && length <= kMaximumPluginNameLength;
	}

	bool PluginNameEquals(const RE::TESFile* file, std::string_view pluginName)
	{
		if (!HasPlausibleFilename(file)) {
			return false;
		}

		const auto filenameLength = strnlen_s(file->filename, kPluginFilenameCapacity);
		return filenameLength == pluginName.size() &&
			_strnicmp(file->filename, pluginName.data(), pluginName.size()) == 0;
	}

	bool ValidateDaytripperCollection(const RE::TESFileCollection* collection)
	{
		if (!IsReadableMemory(collection, sizeof(RE::TESFileCollection))) {
			logger::error("Daytripper returned an unreadable TESFileCollection");
			return false;
		}

		const auto normalCount = collection->files.size();
		const auto smallCount = collection->smallFiles.size();
		if (normalCount == 0 || normalCount > kMaximumNormalFiles || smallCount > kMaximumSmallFiles) {
			logger::error(
				FMT_STRING("Rejected Daytripper collection with implausible counts: normal={}, light={}"),
				normalCount,
				smallCount);
			return false;
		}

		if (!IsReadableMemory(collection->files.data(), normalCount * sizeof(RE::TESFile*)) ||
			(smallCount > 0 && !IsReadableMemory(collection->smallFiles.data(), smallCount * sizeof(RE::TESFile*)))) {
			logger::error("Rejected Daytripper collection with unreadable array storage");
			return false;
		}

		std::unordered_set<const RE::TESFile*> uniqueFiles;
		std::unordered_set<std::uint8_t> normalIndices;
		std::unordered_set<std::uint16_t> smallIndices;
		uniqueFiles.reserve(normalCount + smallCount);

		for (const auto* file : collection->files) {
			if (!HasPlausibleFilename(file) || file->compileIndex >= 0xFE || file->IsLight() ||
				!uniqueFiles.emplace(file).second || !normalIndices.emplace(file->compileIndex).second) {
				logger::error("Rejected Daytripper collection because a normal plugin entry is invalid");
				return false;
			}
		}

		for (const auto* file : collection->smallFiles) {
			if (!HasPlausibleFilename(file) || file->compileIndex != 0xFE || !file->IsLight() ||
				file->smallFileCompileIndex >= kMaximumSmallFiles || !uniqueFiles.emplace(file).second ||
				!smallIndices.emplace(file->smallFileCompileIndex).second) {
				logger::error("Rejected Daytripper collection because a light plugin entry is invalid");
				return false;
			}
		}

		logger::info(
			FMT_STRING("Validated Daytripper compiled-file collection: {} normal, {} light plugins"),
			normalCount,
			smallCount);
		return true;
	}

	const RE::TESFileCollection* ResolveDaytripperCollection()
	{
		const DaytripperModuleCandidate* selectedCandidate = nullptr;
		HMODULE selectedModule = nullptr;
		for (const auto& candidate : kDaytripperModuleCandidates) {
			if (const auto module = GetModuleHandleW(candidate.moduleName)) {
				if (selectedModule && module != selectedModule) {
					logger::critical(
						FMT_STRING("Both {} and {} are loaded; refusing an ambiguous Daytripper provider"),
						selectedCandidate->displayName,
						candidate.displayName);
					return nullptr;
				}
				selectedCandidate = &candidate;
				selectedModule = module;
			}
		}

		if (!selectedModule || !selectedCandidate) {
			logger::warn(
				"Daytripper is not loaded under Daytripper4.dll or falloutvresl.dll; ESL form resolution is unavailable");
			return nullptr;
		}

		const auto address = GetProcAddress(selectedModule, "GetCompiledFileCollectionExtern");
		if (!address) {
			logger::error(
				FMT_STRING("Daytripper module {} does not export GetCompiledFileCollectionExtern; ESL form resolution is unavailable"),
				selectedCandidate->displayName);
			return nullptr;
		}

		const auto getCollection = reinterpret_cast<GetCompiledFileCollection>(address);
		const auto* collection = getCollection();
		if (!ValidateDaytripperCollection(collection)) {
			return nullptr;
		}
		logger::info(FMT_STRING("Using Daytripper ESL provider {}"), selectedCandidate->displayName);
		return collection;
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
			return { lightPlugin, false, true };
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

bool InitializeFormResolver()
{
	if (g_formResolverInitialized) {
		return RE::TESDataHandler::VRcompiledFileCollection != nullptr;
	}

	g_formResolverInitialized = true;
	g_formCache.clear();
	RE::TESDataHandler::VRcompiledFileCollection = nullptr;

	const auto* collection = ResolveDaytripperCollection();
	if (collection) {
		RE::TESDataHandler::VRcompiledFileCollection = const_cast<RE::TESFileCollection*>(collection);
	}

	return collection != nullptr;
}

bool FormMatchesModNames(const RE::TESForm* form, const std::vector<std::string>& modNames)
{
	if (modNames.empty()) {
		return true;
	}
	if (!form) {
		return false;
	}

	const auto* file = form->GetFile(0);
	if (!file) {
		logger::warn(FMT_STRING("Form {:08X}: filterByModNames cannot be evaluated because GetFile(0) is null"), form->formID);
		return false;
	}

	return std::ranges::any_of(modNames, [&](const auto& modName) {
		return PluginNameEquals(file, trim(modName));
	});
}

RE::TESForm* GetFormFromIdentifier(const std::string& identifier)
{
	auto* dataHandler = RE::TESDataHandler::GetSingleton(false);
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
	if (const auto cached = g_formCache.find(lookupIdentifier); cached != g_formCache.end()) {
		return cached->second;
	}

	auto delimiter = lookupIdentifier.find('|');
	if (delimiter != std::string::npos) {
		std::string modName = trim(lookupIdentifier.substr(0, delimiter));
		std::string modForm = trim(lookupIdentifier.substr(delimiter + 1));

		try {
			const auto resolution = ResolveLoadedFile(dataHandler, modName);
			if (!resolution.file) {
				if (PATCH::IsDiagnosticsEnabled()) {
					PATCH::RecordUnresolvedForm(PATCH::ActiveCategoryOr("forms"), lookupIdentifier, "plugin is not loaded");
				}
				g_formCache.emplace(lookupIdentifier, nullptr);
				return nullptr;
			}

			std::size_t parsedLength = 0;
			const auto parsedFormID = std::stoull(modForm, &parsedLength, 16);
			const auto maximumFormID = resolution.isSmallPlugin ? kSmallFormMask : kFullFormMask;
			if (parsedLength != modForm.size() || parsedFormID > maximumFormID) {
				throw std::out_of_range("form ID exceeds the loaded plugin's ID domain");
			}
			const auto rawFormID = static_cast<RE::TESFormID>(parsedFormID);

			auto* form = RE::TESForm::GetFormByID(BuildRuntimeFormID(resolution, rawFormID));
			if (!form && PATCH::IsDiagnosticsEnabled()) {
				PATCH::RecordUnresolvedForm(PATCH::ActiveCategoryOr("forms"), lookupIdentifier, "loaded form lookup returned null");
			}
			g_formCache.emplace(lookupIdentifier, form);
			return form;
		} catch (const std::exception&) {
			if (PATCH::IsDiagnosticsEnabled()) {
				PATCH::RecordUnresolvedForm(PATCH::ActiveCategoryOr("forms"), lookupIdentifier, "invalid hex form id");
			}
			g_formCache.emplace(lookupIdentifier, nullptr);
			return nullptr;
		}
	}
	if (PATCH::IsDiagnosticsEnabled()) {
		PATCH::RecordUnresolvedForm(PATCH::ActiveCategoryOr("forms"), lookupIdentifier, "identifier must be PluginName|FormID or @Alias");
	}
	g_formCache.emplace(lookupIdentifier, nullptr);
	return nullptr;
}
bool IsPluginInstalled(const char* name)
{
	if (!name) {
		return false;
	}

	auto* dataHandler = RE::TESDataHandler::GetSingleton(false);
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
	PATCH::RecordPatchCall(category);
	if (PATCH::IsDryRun()) {
		PATCH::RecordSkippedMutation(category, target, "dry-run enabled");
		return true;
	}
	return false;
}

int findPositionInArray(const RE::BSTArray<RE::TESForm*>& pArray, RE::TESForm* form)
{
	if (pArray.empty() || !form) {
		return -1;
	}

	for (RE::BSTArray<RE::TESForm*>::size_type i = 0; i < pArray.size(); ++i) {
		if (pArray[i] == form) {
			return i <= static_cast<decltype(i)>((std::numeric_limits<int>::max)()) ? static_cast<int>(i) : -1;
		}
	}

	return -1;
}



bool changeAVIF_NPC(RE::TESNPC* pNPC, RE::ActorValueInfo* pActorValueInfo, float pfValue)
{
	if (!pNPC || !pActorValueInfo) {
		return false;
	}
	if (pNPC->properties) {
		for (RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>::size_type i = 0; i < pNPC->properties->size(); ++i) {
			if (pNPC->properties[0][i].first && pNPC->properties[0][i].first->formID == pActorValueInfo->formID) {
				pNPC->properties[0][i].second.f = pfValue;
				//logger::info("avif changed!");
				return true;
			}
		}
	} else {
		pNPC->properties = CreateEngineOwnedArray<RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>>();
		if (!pNPC->properties) {
			return false;
		}
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
	if (!pNPC || !pActorValueInfo) {
		return false;
	}
	if (pNPC->properties) {
	for (RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>::size_type i = 0; i < pNPC->properties->size(); ++i) {
		if (pNPC->properties[0][i].first && pNPC->properties[0][i].first->formID == pActorValueInfo->formID) {
			pNPC->properties[0][i].second.f = pfValue;
			//logger::info("avif changed!");
			return true;
		}
	}
	} else {
		pNPC->properties = CreateEngineOwnedArray<RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>>();
		if (!pNPC->properties) {
			return false;
		}
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = pActorValueInfo;
	newTuple.second.f = pfValue;
	pNPC->properties[0].push_back(newTuple);
	//logger::info("avif added!");
	return true;
}

bool changeKeyword_TESLevItem(RE::TESLevItem* pNPC, RE::BGSKeyword* pActorValueInfo, float pfValue)
{
	if (!pNPC || !pActorValueInfo) {
		return false;
	}
	if (pNPC->keywordChances) {
		for (RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>::size_type i = 0; i < pNPC->keywordChances->size(); ++i) {
		if (pNPC->keywordChances[0][i].first && pNPC->keywordChances[0][i].first->formID == pActorValueInfo->formID) {
			pNPC->keywordChances[0][i].second.f = pfValue;
			//logger::info("avif changed!");
			return true;
		}
		}
	} else {
		pNPC->keywordChances = CreateEngineOwnedArray<RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>>();
		if (!pNPC->keywordChances) {
			return false;
		}
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = pActorValueInfo;
	newTuple.second.f = pfValue;
	pNPC->keywordChances[0].push_back(newTuple);
	//logger::info("avif added!");
	return true;
}

bool changeDamageType_Weapon(RE::TESObjectWEAP* object, RE::BGSDamageType* type, float pfValue)
{
	if (!object || !type || !std::isfinite(pfValue) || pfValue < 0.0f ||
		static_cast<double>(pfValue) > static_cast<double>((std::numeric_limits<std::uint32_t>::max)())) {
		return false;
	}
	if (object->weaponData.damageTypes) {
		for (RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>::size_type i = 0; i < object->weaponData.damageTypes->size(); ++i) {
			if (object->weaponData.damageTypes[0][i].first && object->weaponData.damageTypes[0][i].first->formID == type->formID) {
				object->weaponData.damageTypes[0][i].second.i = static_cast<uint32_t>(pfValue);
				return true;
			}
		}
	} else {
		object->weaponData.damageTypes = CreateEngineOwnedArray<RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>>();
		if (!object->weaponData.damageTypes) {
			return false;
		}
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = type;
	newTuple.second.i = static_cast<uint32_t>(pfValue);

	object->weaponData.damageTypes[0].push_back(newTuple);
	return true;
}

bool eraseDamageType_Weapon(RE::TESObjectWEAP* object, RE::BGSDamageType* type)
{
	if (!object || !type || !object->weaponData.damageTypes) {
		return false;
	}
	if (object->weaponData.damageTypes->size() > 0 && object->weaponData.damageTypes[0].size() > 0) {
	
    // Assuming object->weaponData.damageTypes is your vector
		auto& damageTypes = object->weaponData.damageTypes[0];
		const std::uint32_t formIDToMatch = type->formID;

		// Use std::remove_if and std::erase to remove elements based on the condition
		damageTypes.erase(
			std::remove_if(
				damageTypes.begin(),
				damageTypes.end(),
				[&](const RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>& type) {
					return type.first && type.first->formID == formIDToMatch;
				}),
			damageTypes.end());

		return true;
	}
	return false;
}

bool changeDamageType_Armor(RE::TESObjectARMO* object, RE::BGSDamageType* type, float pfValue)
{
	if (!object || !type || !std::isfinite(pfValue) || pfValue < 0.0f ||
		static_cast<double>(pfValue) > static_cast<double>((std::numeric_limits<std::uint32_t>::max)())) {
		return false;
	}
	if (object->armorData.damageTypes) {
		for (RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>::size_type i = 0; i < object->armorData.damageTypes->size(); ++i) {
			if (object->armorData.damageTypes[0][i].first && object->armorData.damageTypes[0][i].first->formID == type->formID) {
				object->armorData.damageTypes[0][i].second.i = static_cast<uint32_t>(pfValue);
				return true;
			}
		}
	} else {
		object->armorData.damageTypes = CreateEngineOwnedArray<RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>>();
		if (!object->armorData.damageTypes) {
			return false;
		}
	}
	RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal> newTuple;
	newTuple.first = type;
	//newTuple.second.f = pfValue;
	newTuple.second.i = static_cast<uint32_t>(pfValue);

	object->armorData.damageTypes[0].push_back(newTuple);

	return true;
}

bool changeDamageTypeMult_Armor(RE::TESObjectARMO* object, RE::BGSDamageType* type, float pfValue)
{
	if (!object || !type) {
		return false;
	}
	if (object->armorData.damageTypes) {
		for (RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>::size_type i = 0; i < object->armorData.damageTypes->size(); ++i) {
			if (object->armorData.damageTypes[0][i].first && object->armorData.damageTypes[0][i].first->formID == type->formID) {
				const auto value = static_cast<double>(object->armorData.damageTypes[0][i].second.i) * pfValue;
				if (!std::isfinite(value) || value < 0.0 || value > static_cast<double>((std::numeric_limits<std::uint32_t>::max)())) {
					return false;
				}
				object->armorData.damageTypes[0][i].second.i = static_cast<std::uint32_t>(value);
				return true;
			}
		}
	}
	return false;
}

//Misc Items

bool hasMISC_Item(RE::TESObjectMISC* pMisc, RE::BGSComponent* component)
{
	if (!pMisc || !component || !pMisc->componentData) {
		return false;
	}
	for (const auto& entry : pMisc->componentData[0]) {
		if (entry.first && entry.first->formID == component->formID) {
			return true;
		}
	}
	return false;
}

bool changeMISC_Item(RE::TESObjectMISC* pMisc, RE::BGSComponent* component, float pfValue)
{
	if (!pMisc || !component) {
		return false;
	}
	if (pMisc->componentData) {
		for (RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>::size_type i = 0; i < pMisc->componentData->size(); ++i) {
			if (pMisc->componentData[0][i].first && pMisc->componentData[0][i].first->formID == component->formID) {
				pMisc->componentData[0][i].second.f = pfValue;
				//logger::info("avif changed!");
				return true;
			}
		}
	} else {
		pMisc->componentData = CreateEngineOwnedArray<RE::BSTArray<RE::BSTTuple<RE::TESForm*, RE::BGSTypedFormValuePair::SharedVal>>>();
		if (!pMisc->componentData) {
			return false;
		}
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
	static const std::regex re(R"(([<>]=?|[<>])\s*(\d+))");
	std::smatch match;
	std::vector<std::string> result;

	if (std::regex_search(input, match, re)) {
		std::string relation_sign = match[1];
		std::string number_str = match[2];
		result.push_back(number_str);
		result.push_back(relation_sign);
	} else if (std::regex_search(input, CachedRegex(R"(\d+)", std::regex::ECMAScript))) {
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

int getRandomNumberCustom(int min, int max) {
	if (min > max) {
		std::swap(min, max);
	}
	thread_local std::mt19937 gen{ std::random_device{}() };
	std::uniform_int_distribution<> distribution(min, max);  // Generates random integers in the range [1, 100]

	return distribution(gen);

}

int getRandomNumber()
{
	thread_local std::mt19937 gen{ std::random_device{}() };
	std::uniform_int_distribution<> distribution(1, 100);  // Generates random integers in the range [1, 100]

	return distribution(gen);
}

bool HasIniExtension(const std::string& fileName)
{
	constexpr std::string_view extension = ".ini";
	return fileName.size() >= extension.size() &&
		toLowerCase(fileName.substr(fileName.size() - extension.size())) == extension;
}

float getRandomFloat(float min, float max)
{
	if (min > max) {
		std::swap(min, max);
	}
	thread_local std::mt19937 gen{ std::random_device{}() };
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(gen);
}


////extract functions
//// Function to extract data from a line using a regex pattern
//void extractData(const std::string& line, const std::string& pattern, std::vector<std::string>& destination)
//{
//	std::regex regex_pattern(pattern, std::regex::icase);
//	std::smatch match;
//	std::regex_search(line, match, regex_pattern);
//
//	std::shared_ptr<std::vector<std::string>> data = std::make_shared<std::vector<std::string>>();
//	if (!match.empty() && !match[1].str().empty()) {
//		std::string data_str = match[1];
//		std::regex list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", std::regex::icase);
//		std::sregex_iterator iterator(data_str.begin(), data_str.end(), list_regex);
//		std::sregex_iterator end;
//
//		while (iterator != end) {
//			std::string tempVar = (*iterator)[0].str();
//			tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
//			tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
//
//			if (tempVar != "none") {
//				(*data).push_back(tempVar);
//			}
//			++iterator;
//		}
//	}
//
//	destination = (*data);
//}

//extract functions
// Function to extract data from a line using a regex pattern
void extractForms(const std::string& line, const std::string& pattern, std::vector<std::string>& destination)
{
	const auto& regex_pattern = CachedRegex(pattern, std::regex::icase);
	std::smatch match;

	// Find all matches in the input line
	regexSearchParameter(line, match, regex_pattern);

	if (!match.empty() && !match[1].str().empty()) {
		std::string data_str = match[1];

		// Check if data_str contains "|"
		const auto& list_regex = CachedRegex("[^,]+", std::regex::icase);
		std::sregex_iterator iterator(data_str.begin(), data_str.end(), list_regex);
		std::sregex_iterator end;

		while (iterator != end) {
			std::string tempVar = (*iterator)[0].str();
			tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
			tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());

			if (tempVar != "none") {
				destination.push_back(tempVar);
			}
			++iterator;
		}
	}
}

void extractDataStrings(const std::string& line, const std::string& pattern, std::vector<std::string>& destination)
{
	const auto& regex_pattern = CachedRegex(pattern, std::regex::icase);
	std::smatch match;
	regexSearchParameter(line, match, regex_pattern);

	std::shared_ptr<std::vector<std::string>> data = std::make_shared<std::vector<std::string>>();
	if (!match.empty() && !match[1].str().empty()) {
		std::string data_str = match[1];
		const auto& list_regex = CachedRegex("[^,]+", std::regex::icase);
		std::sregex_iterator iterator(data_str.begin(), data_str.end(), list_regex);
		std::sregex_iterator end;

		while (iterator != end) {
			std::string tempVar = (*iterator)[0].str();
			tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
			tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());

			if (tempVar != "none") {
				(*data).push_back(tempVar);
			}
			++iterator;
		}
	}

	destination = (*data);
}

void extractMultiDataString(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<std::string>& pcMultFlag_min_values, std::vector<std::string>& pcMultFlag_max_values)
{
	const auto& pcMultFlag_regex = CachedRegex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	regexSearchParameter(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
	const auto& pcMultFlag_list_regex = CachedRegex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back((*pcMultFlag_iterator)[2]);
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? (*pcMultFlag_iterator)[3] : (*pcMultFlag_iterator)[2]);

			++pcMultFlag_iterator;
		}
	}
}

void extractMultiDataInt(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<int>& pcMultFlag_min_values, std::vector<int>& pcMultFlag_max_values)
{
	const auto& pcMultFlag_regex = CachedRegex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	regexSearchParameter(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
	const auto& pcMultFlag_list_regex = CachedRegex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back(std::stoi((*pcMultFlag_iterator)[2]));
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? std::stoi((*pcMultFlag_iterator)[3]) : std::stoi((*pcMultFlag_iterator)[2]));

			++pcMultFlag_iterator;
		}
	}
}

void extractMultiDataFloat(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<float>& pcMultFlag_min_values, std::vector<float>& pcMultFlag_max_values)
{
	const auto& pcMultFlag_regex = CachedRegex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	regexSearchParameter(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
	const auto& pcMultFlag_list_regex = CachedRegex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back(std::stof((*pcMultFlag_iterator)[2]));
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? std::stof((*pcMultFlag_iterator)[3]) : std::stof((*pcMultFlag_iterator)[2]));

			++pcMultFlag_iterator;
		}
	}
}

void extractMultiDataFormsFloat(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<float>& pcMultFlag_min_values, std::vector<float>& pcMultFlag_max_values)
{
	const auto& pcMultFlag_regex = CachedRegex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	regexSearchParameter(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
	const auto& pcMultFlag_list_regex = CachedRegex("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back(std::stof((*pcMultFlag_iterator)[2]));
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? std::stof((*pcMultFlag_iterator)[3]) : std::stof((*pcMultFlag_iterator)[2]));

			++pcMultFlag_iterator;
		}
	}
}

void extractValueString(const std::string& line, const std::string& regex_pattern_str, std::string& target_value)
{
	const auto& regex_pattern = CachedRegex(regex_pattern_str, std::regex::icase);
	std::smatch match;
	regexSearchParameter(line, match, regex_pattern);

	if (match.empty() || match[1].str().empty()) {
		target_value = "none";
	} else {
		std::string value = match[1].str();

		// Remove leading whitespaces
		value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), ::isspace));

		// Remove trailing whitespaces
		value.erase(std::find_if_not(value.rbegin(), value.rend(), ::isspace).base(), value.end());

		target_value = value;
	}
}




void extractStrings(const std::string& line, const std::string& pattern, std::vector<std::string>& destination)
{
	const auto& regex_pattern = CachedRegex(pattern, std::regex::icase);
	std::smatch match;
	regexSearchParameter(line, match, regex_pattern);

	std::shared_ptr<std::vector<std::string>> data = std::make_shared<std::vector<std::string>>();
	if (!match.empty() && !match[1].str().empty()) {
		std::string data_str = match[1];
		const auto& list_regex = CachedRegex("[^,]+", std::regex::icase);
		std::sregex_iterator iterator(data_str.begin(), data_str.end(), list_regex);
		std::sregex_iterator end;

		while (iterator != end) {
			std::string tempVar = (*iterator)[0].str();
			tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
			tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());

			if (tempVar != "none") {
				(*data).push_back(tempVar);
			}
			++iterator;
		}
	}

	destination = (*data);
}

void extractMultiFormsString(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<std::string>& pcMultFlag_min_values, std::vector<std::string>& pcMultFlag_max_values)
{
	const auto& pcMultFlag_regex = CachedRegex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	regexSearchParameter(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
	const auto& pcMultFlag_list_regex = CachedRegex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back((*pcMultFlag_iterator)[2]);
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? (*pcMultFlag_iterator)[3] : (*pcMultFlag_iterator)[2]);

			++pcMultFlag_iterator;
		}
	}
}

void extractMultiFormsInt(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<int>& pcMultFlag_min_values, std::vector<int>& pcMultFlag_max_values)
{
	const auto& pcMultFlag_regex = CachedRegex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	regexSearchParameter(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
	const auto& pcMultFlag_list_regex = CachedRegex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back(std::stoi((*pcMultFlag_iterator)[2]));
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? std::stoi((*pcMultFlag_iterator)[3]) : std::stoi((*pcMultFlag_iterator)[2]));

			++pcMultFlag_iterator;
		}
	}
}

void extractMultiFormsFloat(const std::string& line, const std::string& pattern, std::vector<std::string>& pcMultFlag,
	std::vector<float>& pcMultFlag_min_values, std::vector<float>& pcMultFlag_max_values)
{
	const auto& pcMultFlag_regex = CachedRegex(pattern, std::regex::icase);
	std::smatch pcMultFlag_match;
	regexSearchParameter(line, pcMultFlag_match, pcMultFlag_regex);

	if (!pcMultFlag_match.empty() && !pcMultFlag_match[1].str().empty()) {
		std::string pcMultFlag_str = pcMultFlag_match[1];
	const auto& pcMultFlag_list_regex = CachedRegex("(\\w+)\\s*=\\s*([\\d.]+)(?:\\s*~\\s*([\\d.]+))?", std::regex::icase);

		std::sregex_iterator pcMultFlag_iterator(pcMultFlag_str.begin(), pcMultFlag_str.end(), pcMultFlag_list_regex);
		std::sregex_iterator pcMultFlag_end;

		while (pcMultFlag_iterator != pcMultFlag_end) {
			std::string avif = (*pcMultFlag_iterator)[1].str();
			avif.erase(avif.begin(), std::find_if_not(avif.begin(), avif.end(), ::isspace));
			avif.erase(std::find_if_not(avif.rbegin(), avif.rend(), ::isspace).base(), avif.end());

			if (avif == "none") {
				break;
			}

			pcMultFlag.push_back(avif);
			pcMultFlag_min_values.push_back(std::stof((*pcMultFlag_iterator)[2]));
			pcMultFlag_max_values.push_back((*pcMultFlag_iterator)[3] != "" ? std::stof((*pcMultFlag_iterator)[3]) : std::stof((*pcMultFlag_iterator)[2]));

			++pcMultFlag_iterator;
		}
	}
}

//void extractValueString(const std::string& line, const std::string& regex_pattern_str, std::string& target_value)
//{
//	std::regex regex_pattern(regex_pattern_str);
//	std::smatch match;
//	std::regex_search(line, match, regex_pattern);
//
//	if (match.empty() || match[1].str().empty()) {
//		target_value = "none";
//	} else {
//		std::string value = match[1].str();
//
//		// Remove leading whitespaces
//		value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), ::isspace));
//
//		// Remove trailing whitespaces
//		value.erase(std::find_if_not(value.rbegin(), value.rend(), ::isspace).base(), value.end());
//		target_value = value;
//	}
//}

void extractToArr2D(const std::string& line, const std::string& regex_pattern_str, std::vector<std::vector<std::string>>& target_value)
{
	const auto& regex_pattern = CachedRegex(regex_pattern_str, std::regex::icase);
	std::smatch match;
	regexSearchParameter(line, match, regex_pattern);
	std::vector<std::string> arr;

	if (match.empty() || match[1].str().empty()) {
		// Empty
	} else {
		std::string valueLine = match[1].str();
		//logger::debug("valueLine {}", valueLine);
		// Exclude the part after "=" from the first string
		size_t startPos = valueLine.find("=") + 1;
		size_t pos = 0;
		std::string token;
		while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
			token = valueLine.substr(startPos, pos - startPos);
			token = trim(token);
			arr.push_back(token);
			startPos = pos + 1;
		}
		token = valueLine.substr(startPos);
		token = trim(token);
		arr.push_back(token);

		std::vector<std::vector<std::string>> arr2D(arr.size());

		for (int i = 0; i < arr.size(); i++) {
			std::vector<std::string> splitArr;
			size_t innerPos = 0;
			std::string innerToken;
			while ((innerPos = arr[i].find("~")) != std::string::npos) {
				innerToken = arr[i].substr(0, innerPos);
				innerToken = trim(innerToken);
				splitArr.push_back(innerToken);
				arr[i].erase(0, innerPos + 1);
			}
			innerToken = arr[i];
			innerToken = trim(innerToken);
			splitArr.push_back(innerToken);
			arr2D[i] = splitArr;
		}
		target_value = arr2D;
	}
}
