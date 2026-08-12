#include "object_leveledLists.h"
#include "EngineAdapters.h"

#include <unordered_map>
namespace LEVELEDLISTS
{
	bool tryParseInt(const std::string& text, int& value)
	{
		try {
			size_t parsedLength = 0;
			const auto parsed = std::stoll(text, &parsedLength, 10);
			if (parsedLength != text.size() || parsed < (std::numeric_limits<int>::min)() || parsed > (std::numeric_limits<int>::max)()) {
				return false;
			}
			value = static_cast<int>(parsed);
			return true;
		} catch (const std::exception&) {
			return false;
		}
	}

	//void __fastcall TESLeveledList::FreeLeveledList(TESLeveledList* this)
	//{
	//	int* v2;             // rsi
	//	int v3;              // ebx
	//	LEVELED_OBJECT* v4;  // rcx
	//	void* v5;            // rbp
	//	v2 = (int*)(*((_QWORD*)NtCurrentTeb()->ThreadLocalStoragePointer + (unsigned int)tls_index) + 2496i64);
	//	v3 = *v2;
	//	*v2 = 98;
	//	v4 = (LEVELED_OBJECT*)*((_QWORD*)this + 3);
	//	if (v4)
	//		LEVELED_OBJECT::`vector deleting destructor'(v4, 3); v5 = (void*)*((_QWORD*)this + 4);
	//	*((_QWORD*)this + 3) = 0i64;
	//	*((_BYTE*)this + 41) = 0;
	//	if ( `MemoryManager::Instance '::`2' ::uiinitFence != 2)
	//		MemoryManager::UpdateInitState(
	//     `MemoryManager::Instance '::`2' ::cbuffer,
	//			(unsigned int*)&`MemoryManager::Instance '::`2' ::uiinitFence);
	//	MemoryManager::Deallocate((MemoryManager*)`MemoryManager::Instance '::`2' ::cbuffer, v5, 0);
	//	*((_QWORD*)this + 4) = 0i64;
	//	*((_BYTE*)this + 40) = 0;
	//	*v2 = v3;
	//}

	bool FreeLeveledList(RE::LEVELED_OBJECT* objects)
	{
		return EngineAdapters::DestroyLeveledObjectArray(objects);
	}

	struct_ll* reserveNewLevL(uint32_t count)
	{
		if (count == 0) {
			return nullptr;
		}
		if (count > 255) {
			count = 255;  // baseListCount is an unsigned byte in the engine
		}

		auto& mm = RE::MemoryManager::GetSingleton();

		const std::size_t bytes = 8u + (std::size_t)count * sizeof(RE::LEVELED_OBJECT);  // 8 + 24*count
		auto* mem = (struct_ll*)mm.Allocate(bytes, 0, 0);
		if (!mem) {
			return nullptr;
		}

		mem->size = count;
		mem->pad = 0;
		std::memset(mem->ll, 0, (std::size_t)count * sizeof(RE::LEVELED_OBJECT));
		return mem;
	}

	void DeallocateUnownedBlock(struct_ll* block)
	{
		if (block) {
			auto& mm = RE::MemoryManager::GetSingleton();
			mm.Deallocate(block, false);
		}
	}

	std::vector<RE::LEVELED_OBJECT> convertLLtoVec(RE::TESLeveledList* levl)
	{
		std::vector<RE::LEVELED_OBJECT> loVec;

		if (!levl || !levl->leveledLists || levl->baseListCount == 0) {
			return loVec;
		}

		const auto count = static_cast<std::uint32_t>(static_cast<std::uint8_t>(levl->baseListCount));
		loVec.reserve(count);

		for (std::uint32_t i = 0; i < count; i++) {
			loVec.push_back(levl->leveledLists[i]);
		}

		return loVec;
	}

	void createLL(RE::TESLeveledList* levl, const std::vector<RE::LEVELED_OBJECT>& levo)
	{
		if (!levl) {
			return;
		}

		if (levo.empty()) {
			if (levl->leveledLists) {
				if (!FreeLeveledList(levl->leveledLists)) {
					logger::critical("Leveled list clear rejected because the verified destructor is unavailable");
					return;
				}
				levl->leveledLists = nullptr;
				levl->baseListCount = 0;
			}
			return;
		}

		std::uint32_t size = static_cast<std::uint32_t>(levo.size());
		if (size > 255) {
			logger::warn(FMT_STRING("Leveled list requested {} entries; keeping the engine maximum of 255"), size);
			size = 255;
		}

		struct_ll* newLL = reserveNewLevL(size);
		if (!newLL) {
			logger::warn(FMT_STRING("Leveled list: failed to allocate {} replacement entries; keeping the original list"), size);
			return;
		}

		for (std::uint32_t i = 0; i < size; i++) {
			newLL->ll[i] = levo[i];
		}

		if (levl->leveledLists) {
			if (!EngineAdapters::VerifyLeveledObjectDestructor()) {
				logger::critical("Leveled list replacement rejected because the verified destructor is unavailable");
				DeallocateUnownedBlock(newLL);
				return;
			}

			const auto oldCount = static_cast<std::uint32_t>(static_cast<std::uint8_t>(levl->baseListCount));
			std::unordered_map<RE::ContainerItemExtra*, std::uint32_t> oldExtras;
			std::unordered_map<RE::ContainerItemExtra*, std::uint32_t> transferredExtras;
			for (std::uint32_t i = 0; i < oldCount; ++i) {
				if (auto* extra = levl->leveledLists[i].itemExtra) {
					++oldExtras[extra];
				}
			}
			for (std::uint32_t i = 0; i < size; ++i) {
				if (auto* extra = newLL->ll[i].itemExtra) {
					const auto old = oldExtras.find(extra);
					if (old == oldExtras.end() || ++transferredExtras[extra] > old->second) {
						logger::critical("Leveled list replacement rejected because auxiliary ownership cannot be transferred safely");
						DeallocateUnownedBlock(newLL);
						return;
					}
				}
			}

			for (std::uint32_t i = 0; i < oldCount; ++i) {
				auto* extra = levl->leveledLists[i].itemExtra;
				if (!extra) {
					continue;
				}
				auto transfer = transferredExtras.find(extra);
				if (transfer != transferredExtras.end() && transfer->second > 0) {
					levl->leveledLists[i].itemExtra = nullptr;
					--transfer->second;
				}
			}

			if (!FreeLeveledList(levl->leveledLists)) {
				logger::critical("Leveled list replacement aborted after ownership preparation");
				DeallocateUnownedBlock(newLL);
				return;
			}
		}
		levl->leveledLists = newLL->ll;
		levl->baseListCount = static_cast<std::uint8_t>(size);
	}

	void ClearLL(RE::TESLeveledList* levl)
	{
		if (!levl) {
			return;
		}

		if (levl->leveledLists) {
			if (!FreeLeveledList(levl->leveledLists)) {
				logger::critical("Leveled list clear rejected because the verified destructor is unavailable");
				return;
			}

			levl->leveledLists = nullptr;
			levl->baseListCount = 0;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;

		// The singular addToLL syntax never had a mutation path. Reject it explicitly
		// instead of silently accepting a rule that cannot be applied safely.
		static const std::regex unsupportedAddToLLRegex(R"((^|:)\s*addToLL\s*=)", std::regex::icase);
		if (std::regex_search(line, unsupportedAddToLLRegex)) {
			logger::warn("Unsupported legacy addToLL directive; use addToLLs instead");
		}

		// extract objects
		std::regex objects_regex("filterByLLs\\s*=([^:]+)", regex::icase);
		std::smatch objects_match;
		regexSearchParameter(line, objects_match, objects_regex);
		std::vector<std::string> objects;
		if (objects_match.empty() || objects_match[1].str().empty()) {
			//empty
		} else {
			std::string objects_str = objects_match[1];
			std::regex objects_list_regex("[^,]+", regex::icase);
			std::sregex_iterator objects_iterator(objects_str.begin(), objects_str.end(), objects_list_regex);
			std::sregex_iterator objects_end;
			while (objects_iterator != objects_end) {
				std::string tempVar = (*objects_iterator)[0].str();
				tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
				tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
				//logger::info(FMT_STRING("Race: {}"), race);
				if (tempVar != "none") {
					objects.push_back(tempVar);
				}
				++objects_iterator;
			}
			l.objects = objects;
		}

				// extract noFilterLL
		std::regex noFilterLL_regex("noFilterLL\\s*=([^:]+)", regex::icase);
		std::smatch noFilterLLmatch;
		regexSearchParameter(line, noFilterLLmatch, noFilterLL_regex);
		// extract the value after the equals sign
		if (noFilterLLmatch.empty() || noFilterLLmatch[1].str().empty()) {
			l.noFilterLL = "none";
		} else {
			std::string noFilterLLvalue = noFilterLLmatch[1].str();
			noFilterLLvalue.erase(noFilterLLvalue.begin(), std::find_if_not(noFilterLLvalue.begin(), noFilterLLvalue.end(), ::isspace));
			noFilterLLvalue.erase(std::find_if_not(noFilterLLvalue.rbegin(), noFilterLLvalue.rend(), ::isspace).base(), noFilterLLvalue.end());
			l.noFilterLL = noFilterLLvalue;
		}

		// extract objectsContainer
		std::regex objectsContainer_regex("filterByContainers\\s*=([^:]+)", regex::icase);
		std::smatch objectsContainer_match;
		regexSearchParameter(line, objectsContainer_match, objectsContainer_regex);
		std::vector<std::string> objectsContainer;
		if (objectsContainer_match.empty() || objectsContainer_match[1].str().empty()) {
			//empty
		} else {
			std::string objectsContainer_str = objectsContainer_match[1];
			std::regex objectsContainer_list_regex("[^,]+", regex::icase);
			std::sregex_iterator objectsContainer_iterator(objectsContainer_str.begin(), objectsContainer_str.end(), objectsContainer_list_regex);
			std::sregex_iterator objectsContainer_end;
			while (objectsContainer_iterator != objectsContainer_end) {
				std::string tempVar = (*objectsContainer_iterator)[0].str();
				tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
				tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
				//logger::info(FMT_STRING("Race: {}"), race);
				if (tempVar != "none") {
					objectsContainer.push_back(tempVar);
				}
				++objectsContainer_iterator;
			}
			l.containerObjects = objectsContainer;
		}

		std::regex add_regex("addToLLs\\s*=([^:]+)", regex::icase);
		std::smatch add_match;
		regexSearchParameter(line, add_match, add_regex);
		std::vector<std::string> add;
		if (add_match.empty() || add_match[1].str().empty()) {
			//empty
		} else {
			std::string valueLine = add_match[1].str();
			std::vector<std::string> arr;

			// exclude the addToLL= part from the first string
			size_t startPos = valueLine.find("=") + 1;
			size_t pos = 0;
			std::string token;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				token = valueLine.substr(startPos, pos - startPos);
				token = trim(token);  // remove leading and trailing white spaces
				arr.push_back(token);
				startPos = pos + 1;
			}
			token = valueLine.substr(startPos);
			token = trim(token);  // remove leading and trailing white spaces
			arr.push_back(token);

			std::vector<std::vector<std::string>> arr2D(arr.size());

			for (int i = 0; i < arr.size(); i++) {
				std::vector<std::string> splitArr;
				size_t innerPos = 0;
				std::string innerToken;
				while ((innerPos = arr[i].find("~")) != std::string::npos) {
					innerToken = arr[i].substr(0, innerPos);
					innerToken = trim(innerToken);  // remove leading and trailing white spaces
					splitArr.push_back(innerToken);
					arr[i].erase(0, innerPos + 1);
				}
				innerToken = arr[i];
				innerToken = trim(innerToken);  // remove leading and trailing white spaces
				splitArr.push_back(innerToken);
				arr2D[i] = splitArr;
			}
			l.addedObjects = arr2D;
		}

		std::regex addContainer_regex("addToContainers\\s*=([^:]+)", regex::icase);
		std::smatch addContainer_match;
		regexSearchParameter(line, addContainer_match, addContainer_regex);
		std::vector<std::string> addContainer;
		if (addContainer_match.empty() || addContainer_match[1].str().empty()) {
			//empty
		} else {
			std::string valueLine = addContainer_match[1].str();
			std::vector<std::string> arr;

			// exclude the addToLL= part from the first string
			size_t startPos = valueLine.find("=") + 1;
			size_t pos = 0;
			std::string token;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				token = valueLine.substr(startPos, pos - startPos);
				token = trim(token);  // remove leading and trailing white spaces
				arr.push_back(token);
				startPos = pos + 1;
			}
			token = valueLine.substr(startPos);
			token = trim(token);  // remove leading and trailing white spaces
			arr.push_back(token);

			std::vector<std::vector<std::string>> arr2D(arr.size());

			for (int i = 0; i < arr.size(); i++) {
				std::vector<std::string> splitArr;
				size_t innerPos = 0;
				std::string innerToken;
				while ((innerPos = arr[i].find("~")) != std::string::npos) {
					innerToken = arr[i].substr(0, innerPos);
					innerToken = trim(innerToken);  // remove leading and trailing white spaces
					splitArr.push_back(innerToken);
					arr[i].erase(0, innerPos + 1);
				}
				innerToken = arr[i];
				innerToken = trim(innerToken);  // remove leading and trailing white spaces
				splitArr.push_back(innerToken);
				arr2D[i] = splitArr;
			}
			l.addedContainerObjects = arr2D;
		}

				// extract objectsToRemove
		std::regex objectsToRemove_regex("removeFromContainers\\s*=([^:]+)", regex::icase);
		std::smatch objectsToRemove_match;
		regexSearchParameter(line, objectsToRemove_match, objectsToRemove_regex);
		std::vector<std::string> objectsToRemove;
		if (objectsToRemove_match.empty() || objectsToRemove_match[1].str().empty()) {
			//empty
		} else {
			std::string objectsToRemove_str = objectsToRemove_match[1];
			std::regex objectsToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator objectsToRemove_iterator(objectsToRemove_str.begin(), objectsToRemove_str.end(), objectsToRemove_list_regex);
			std::sregex_iterator objectsToRemove_end;
			while (objectsToRemove_iterator != objectsToRemove_end) {
				std::string spellToAdd = (*objectsToRemove_iterator)[0].str();
				spellToAdd.erase(spellToAdd.begin(), std::find_if_not(spellToAdd.begin(), spellToAdd.end(), ::isspace));
				spellToAdd.erase(std::find_if_not(spellToAdd.rbegin(), spellToAdd.rend(), ::isspace).base(), spellToAdd.end());
				if ((*objectsToRemove_iterator)[0].str() != "none") {
					//logger::info(FMT_STRING("objectsToRemove: {}"), spellToAdd);
					objectsToRemove.push_back(spellToAdd);
				}
				++objectsToRemove_iterator;
			}
			l.removedContainerObjects = objectsToRemove;
		}

		std::regex remove_regex("removeFromLLs\\s*=([^:]+)", regex::icase);
		std::smatch remove_match;
		regexSearchParameter(line, remove_match, remove_regex);
		std::vector<std::string> remove;
		if (remove_match.empty() || remove_match[1].str().empty()) {
			//empty
		} else {
			std::string valueLine = remove_match[1].str();
			std::vector<std::string> arr;

			// exclude the removeToLL= part from the first string
			size_t startPos = valueLine.find("=") + 1;
			size_t pos = 0;
			std::string token;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				token = valueLine.substr(startPos, pos - startPos);
				token = trim(token);  // remove leading and trailing white spaces
				arr.push_back(token);
				startPos = pos + 1;
			}
			token = valueLine.substr(startPos);
			token = trim(token);  // remove leading and trailing white spaces
			arr.push_back(token);

			std::vector<std::vector<std::string>> arr2D(arr.size());

			for (int i = 0; i < arr.size(); i++) {
				std::vector<std::string> splitArr;
				size_t innerPos = 0;
				std::string innerToken;
				while ((innerPos = arr[i].find("~")) != std::string::npos) {
					innerToken = arr[i].substr(0, innerPos);
					innerToken = trim(innerToken);  // remove leading and trailing white spaces
					splitArr.push_back(innerToken);
					arr[i].erase(0, innerPos + 1);
				}
				innerToken = arr[i];
				innerToken = trim(innerToken);  // remove leading and trailing white spaces
				splitArr.push_back(innerToken);
				arr2D[i] = splitArr;
			}
			l.removedObjects = arr2D;
		}

		// extract clear
		std::regex clearlist_regex("clear\\s*=([^:]+)", regex::icase);
		std::smatch clearlistmatch;
		regexSearchParameter(line, clearlistmatch, clearlist_regex);
		// extract the value after the equals sign
		if (clearlistmatch.empty() || clearlistmatch[1].str().empty()) {
			l.clear = "none";
		} else {
			std::string clearlistvalue = clearlistmatch[1].str();
			clearlistvalue.erase(std::remove_if(clearlistvalue.begin(), clearlistvalue.end(), ::isspace), clearlistvalue.end());
			l.clear = clearlistvalue;
		}

				// extract calcforLevel
		std::regex calcforLevellist_regex("calcForLevel\\s*=([^:]+)", regex::icase);
		std::smatch calcforLevellistmatch;
		regexSearchParameter(line, calcforLevellistmatch, calcforLevellist_regex);
		// extract the value after the equals sign
		if (calcforLevellistmatch.empty() || calcforLevellistmatch[1].str().empty()) {
			l.calcForLevel = "none";
		} else {
			std::string calcforLevellistvalue = calcforLevellistmatch[1].str();
			calcforLevellistvalue.erase(std::remove_if(calcforLevellistvalue.begin(), calcforLevellistvalue.end(), ::isspace), calcforLevellistvalue.end());
			l.calcForLevel = calcforLevellistvalue;
		}



				// extract calcEachItem
		std::regex calcEachItemlist_regex("calcEachItem\\s*=([^:]+)", regex::icase);
		std::smatch calcEachItemlistmatch;
		regexSearchParameter(line, calcEachItemlistmatch, calcEachItemlist_regex);
		// extract the value after the equals sign
		if (calcEachItemlistmatch.empty() || calcEachItemlistmatch[1].str().empty()) {
			l.calcEachItem = "none";
		} else {
			std::string calcEachItemlistvalue = calcEachItemlistmatch[1].str();
			calcEachItemlistvalue.erase(std::remove_if(calcEachItemlistvalue.begin(), calcEachItemlistvalue.end(), ::isspace), calcEachItemlistvalue.end());
			l.calcEachItem = calcEachItemlistvalue;
		}

						// extract calcLevelAndEachItem
		std::regex calcLevelAndEachItemlist_regex("calcLevelAndEachItem\\s*=([^:]+)", regex::icase);
		std::smatch calcLevelAndEachItemlistmatch;
		regexSearchParameter(line, calcLevelAndEachItemlistmatch, calcLevelAndEachItemlist_regex);
		// extract the value after the equals sign
		if (calcLevelAndEachItemlistmatch.empty() || calcLevelAndEachItemlistmatch[1].str().empty()) {
			l.calcForLevelAndEachItem = "none";
		} else {
			std::string calcLevelAndEachItemlistvalue = calcLevelAndEachItemlistmatch[1].str();
			calcLevelAndEachItemlistvalue.erase(std::remove_if(calcLevelAndEachItemlistvalue.begin(), calcLevelAndEachItemlistvalue.end(), ::isspace), calcLevelAndEachItemlistvalue.end());
			l.calcForLevelAndEachItem = calcLevelAndEachItemlistvalue;
		}

						// extract calcUseAll
		std::regex calcUseAlllist_regex("calcUseAll\\s*=([^:]+)", regex::icase);
		std::smatch calcUseAlllistmatch;
		regexSearchParameter(line, calcUseAlllistmatch, calcUseAlllist_regex);
		// extract the value after the equals sign
		if (calcUseAlllistmatch.empty() || calcUseAlllistmatch[1].str().empty()) {
			l.calcUseAll = "none";
		} else {
			std::string calcUseAlllistvalue = calcUseAlllistmatch[1].str();
			calcUseAlllistvalue.erase(std::remove_if(calcUseAlllistvalue.begin(), calcUseAlllistvalue.end(), ::isspace), calcUseAlllistvalue.end());
			l.calcUseAll = calcUseAlllistvalue;
		}

		std::regex templateKeyword_regex("addTemplateKeywords\\s*=([^:]+)", regex::icase);
		std::smatch templateKeyword_match;
		regexSearchParameter(line, templateKeyword_match, templateKeyword_regex);
		std::vector<std::string> templateKeyword;
		if (templateKeyword_match.empty() || templateKeyword_match[1].str().empty()) {
			//empty
		} else {
			std::string valueLine = templateKeyword_match[1].str();
			std::vector<std::string> arr;

			// exclude the templateKeywordToLL= part from the first string
			size_t startPos = valueLine.find("=") + 1;
			size_t pos = 0;
			std::string token;
			while ((pos = valueLine.find(",", startPos)) != std::string::npos) {
				token = valueLine.substr(startPos, pos - startPos);
				token = trim(token);  // remove leading and trailing white spaces
				arr.push_back(token);
				startPos = pos + 1;
			}
			token = valueLine.substr(startPos);
			token = trim(token);  // remove leading and trailing white spaces
			arr.push_back(token);

			std::vector<std::vector<std::string>> arr2D(arr.size());

			for (int i = 0; i < arr.size(); i++) {
				std::vector<std::string> splitArr;
				size_t innerPos = 0;
				std::string innerToken;
				while ((innerPos = arr[i].find("~")) != std::string::npos) {
					innerToken = arr[i].substr(0, innerPos);
					innerToken = trim(innerToken);  // remove leading and trailing white spaces
					splitArr.push_back(innerToken);
					arr[i].erase(0, innerPos + 1);
				}
				innerToken = arr[i];
				innerToken = trim(innerToken);  // remove leading and trailing white spaces
				splitArr.push_back(innerToken);
				arr2D[i] = splitArr;
			}
			l.templateKeyword = arr2D;
		}

		// extract removeWeaponsByKeyword
		std::regex removeWeaponsByKeyword_regex("removeObjectsByKeyword\\s*=([^:]+)", regex::icase);
		std::smatch removeWeaponsByKeyword_match;
		regexSearchParameter(line, removeWeaponsByKeyword_match, removeWeaponsByKeyword_regex);
		std::vector<std::string> removeWeaponsByKeyword;
		if (removeWeaponsByKeyword_match.empty() || removeWeaponsByKeyword_match[1].str().empty()) {
			//empty
		} else {
			std::string removeWeaponsByKeyword_str = removeWeaponsByKeyword_match[1];
			std::regex removeWeaponsByKeyword_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator removeWeaponsByKeyword_iterator(removeWeaponsByKeyword_str.begin(), removeWeaponsByKeyword_str.end(), removeWeaponsByKeyword_list_regex);
			std::sregex_iterator removeWeaponsByKeyword_end;
			while (removeWeaponsByKeyword_iterator != removeWeaponsByKeyword_end) {
				std::string keywordToRemove = (*removeWeaponsByKeyword_iterator)[0].str();
				keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
				keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
				if (keywordToRemove != "none") {
					//logger::info(FMT_STRING("removeWeaponsByKeyword: {}"), keywordToRemove);
					removeWeaponsByKeyword.push_back(keywordToRemove);
				}
				++removeWeaponsByKeyword_iterator;
			}
			l.removeItemsByKeyword = removeWeaponsByKeyword;
		}

		// extract chanceRobCo
		std::regex chanceRobCo_regex("chance\\s*=([^:]+)", regex::icase);
		std::smatch match;
		regexSearchParameter(line, match, chanceRobCo_regex);
		// extract the value after the equals sign
		if (match.empty() || match[1].str().empty()) {
			l.chanceRobCo = "none";
		} else {
			std::string value = match[1].str();
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			l.chanceRobCo = value;
		}

		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);


		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		const auto& objectArray = dataHandler->GetFormArray<RE::TESLevItem>();

		
		for (const auto& line : tokens) {
			std::unordered_set<std::uint32_t> directlyPatchedLists;
			
			


			//New
			if (!line.objects.empty()) {
				for (const auto& objectstring : line.objects) {
					RE::TESForm* currentform = nullptr;
					RE::TESLevItem* object = nullptr;

					std::string string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kLVLI && FormMatchesModNames(currentform, line.modNames)) {
						object = (RE::TESLevItem*)currentform;
						//logger::debug(FMT_STRING("leveled lists {:08X} call patch."), object->formID);
						patch(line, object);
						directlyPatchedLists.insert(object->formID);
					}
				}
			}

			if (!line.containerObjects.empty()) {
				//logger::info("npc not empty");
				for (const auto& objectstring : line.containerObjects) {
					RE::TESForm* currentform = nullptr;
					RE::TESObjectCONT* object = nullptr;
					std::string string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kCONT && FormMatchesModNames(currentform, line.modNames)) {
						object = (RE::TESObjectCONT*)currentform;
						patchContainer(line, object);
					}
				}
			}

			if (!line.noFilterLL.empty() && toLowerCase(line.noFilterLL) == "true") {
				for (const auto& curobj : objectArray) {
					if (!curobj) {
						continue;
					}
					if (directlyPatchedLists.contains(curobj->formID)) {
						continue;
					}

					if (curobj->IsDeleted()) {
						continue;
					}

					if (!FormMatchesModNames(curobj, line.modNames)) {
						continue;
					}

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
								PATCH::RecordFile("leveledlist", fullPath);
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

									PATCH::RecordRule("leveledlist");
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

	void patch(const LEVELEDLISTS::line_content& line, RE::TESLevItem* curobj)
	{
		if (!curobj || ShouldSkipPatch("leveledlist", curobj)) {
			return;
		}

		if (!line.chanceRobCo.empty() && line.chanceRobCo != "none") {
			int random_number = getRandomNumber();
			int chance = 0;
			if (!tryParseInt(line.chanceRobCo, chance)) {
				logger::warn(FMT_STRING("Leveled list {:08X}: invalid chance value '{}'; skipping condition"), curobj->formID, line.chanceRobCo);
			} else if (random_number > chance) {
				logger::debug("Skipped {:08X} by chance {} > {}", curobj->formID, random_number, chance);
				return;
			}
		}

		if (!line.removedObjects.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);

			for (const auto& objectsToRemove : line.removedObjects) {
				if (objectsToRemove.empty()) {
					logger::warn(FMT_STRING("Leveled list {:08X}: empty removeFromLLs entry"), curobj->formID);
					continue;
				}
				RE::TESForm* delForm = GetFormFromIdentifier(objectsToRemove[0]);
				if (!delForm) {
					logger::debug(FMT_STRING("Form not found {}, skipping."), objectsToRemove[0]);
					continue;
				}

				struct NumericFilter
				{
					bool enabled{ false };
					std::uint16_t value{ 0 };
					std::string relation;
				};

				auto parseFilter = [&](std::size_t index, std::string_view label, NumericFilter& filter) {
					if (objectsToRemove.size() <= index || toLowerCase(objectsToRemove[index]) == "none") {
						return true;
					}

					const auto parts = splitRelationNumber(objectsToRemove[index]);
					int parsedNumber = 0;
					if (parts.empty() || parts.size() > 2 || !tryParseInt(parts[0], parsedNumber) ||
						parsedNumber < 0 || parsedNumber > (std::numeric_limits<std::uint16_t>::max)()) {
						logger::warn(FMT_STRING("Leveled list {:08X}: invalid {} filter '{}'"), curobj->formID, label, objectsToRemove[index]);
						return false;
					}

					filter.relation = parts.size() == 2 ? parts[1] : "";
					if (filter.relation != "" && filter.relation != "<" && filter.relation != ">" &&
						filter.relation != "<=" && filter.relation != ">=") {
						logger::warn(FMT_STRING("Leveled list {:08X}: invalid {} relation '{}'"), curobj->formID, label, filter.relation);
						return false;
					}

					filter.enabled = true;
					filter.value = static_cast<std::uint16_t>(parsedNumber);
					return true;
				};

				NumericFilter levelFilter;
				NumericFilter countFilter;
				if (!parseFilter(1, "level", levelFilter) || !parseFilter(2, "count", countFilter)) {
					continue;
				}

				auto matches = [](std::uint16_t candidate, const NumericFilter& filter) {
					if (!filter.enabled) {
						return true;
					}
					if (filter.relation == "<") {
						return candidate < filter.value;
					}
					if (filter.relation == ">") {
						return candidate > filter.value;
					}
					if (filter.relation == "<=") {
						return candidate <= filter.value;
					}
					if (filter.relation == ">=") {
						return candidate >= filter.value;
					}
					return candidate == filter.value;
				};

				const auto oldSize = llVec.size();
				llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& entry) {
					return entry.form == delForm && matches(entry.level, levelFilter) && matches(entry.count, countFilter);
				}),
					llVec.end());
				logger::debug(FMT_STRING("Leveled list {:08X}: removed {} matching entries for form {:08X}"), curobj->formID, oldSize - llVec.size(), delForm->formID);
			}

			std::sort(llVec.begin(), llVec.end(), [](const RE::LEVELED_OBJECT& a, const RE::LEVELED_OBJECT& b) {
				return a.level < b.level;
			});
			createLL(curobj, llVec);
		}

		if (!line.removeItemsByKeyword.empty()) {
			std::vector<RE::LEVELED_OBJECT> llVec = convertLLtoVec(curobj);
			for (size_t i = 0; i < line.removeItemsByKeyword.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.removeItemsByKeyword[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
				
					llVec.erase(std::remove_if(llVec.begin(), llVec.end(), [&](const RE::LEVELED_OBJECT& x) {
						if (!x.form) {
							return false;
						}
						RE::BGSKeywordForm* keyForm = x.form->As<RE::BGSKeywordForm>();
						if (keyForm && keyForm->HasKeyword((RE::BGSKeyword*)currentform)) {
							logger::debug(FMT_STRING("leveled Lists {:08X} removed item {:08X} with keyword {:08X} {} "), curobj->formID, x.form->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
							return true;
						}
						//logger::debug(FMT_STRING("leveled Lists {:08X} cannot removed item {:08X} not a keyword {:08X} {} "), curobj->formID, x.form->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);

						return false; 
					}),
						llVec.end());
				
				}
			}
			createLL(curobj, llVec);
		}

		if (!line.templateKeyword.empty()) {
			for (const auto& object : line.templateKeyword) {
				if (object.size() < 2) {
					logger::warn(FMT_STRING("Leveled list {:08X}: malformed addTemplateKeywords entry"), curobj->formID);
					continue;
				}
				auto* addForm = GetFormFromIdentifier(object[0]);
				if (!addForm || addForm->formType != RE::ENUM_FORM_ID::kKYWD) {
					logger::warn(FMT_STRING("Leveled list {:08X}: invalid template keyword '{}'"), curobj->formID, object[0]);
					continue;
				}
				int chance = 0;
				if (!tryParseInt(object[1], chance)) {
					logger::warn(FMT_STRING("Leveled list {:08X}: invalid template keyword chance '{}'"), curobj->formID, object[1]);
					continue;
				}
				changeKeyword_TESLevItem(curobj, static_cast<RE::BGSKeyword*>(addForm), static_cast<float>(chance));
				logger::debug(FMT_STRING("leveled Lists formid: {:08X} added/changed {:08X} chance {}"), curobj->formID, addForm->formID, object[1]);
			}

			//curobj->keywordChances
		}

		if (!line.clear.empty() && line.clear != "none") {
			if (line.clear == "yes" || line.clear == "true") {
				logger::debug(FMT_STRING("leveled lists {:08X} cleared"), curobj->formID);
				ClearLL(curobj);
			}
		}
		if (!line.calcForLevel.empty() && line.calcForLevel != "none") {
			if (line.calcForLevel == "yes" || line.calcForLevel == "true") {
				curobj->llFlags = 1;
				logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate from all levels <= players level"), curobj->formID);
			}
		}

		if (!line.calcEachItem.empty() && line.calcEachItem != "none") {
			if (line.calcEachItem == "yes" || line.calcEachItem == "true") {
				curobj->llFlags = 2;
				logger::debug(FMT_STRING("leveled lists {:08X} set to Calculate for each item in count"), curobj->formID);
			}
		}

		if (!line.calcForLevelAndEachItem.empty() && line.calcForLevelAndEachItem != "none") {
			if (line.calcForLevelAndEachItem == "yes" || line.calcForLevelAndEachItem == "true") {
				curobj->llFlags = 3;
				logger::debug(FMT_STRING("leveled lists {:08X} set to  Calculate from all levels <= players level and for each item in count"), curobj->formID);
			}
		}

		if (!line.calcUseAll.empty() && line.calcUseAll != "none") {
			int maxUseAllCount = 0;
			if (!tryParseInt(line.calcUseAll, maxUseAllCount) || maxUseAllCount < 0 || maxUseAllCount > (std::numeric_limits<std::int8_t>::max)()) {
				logger::warn(FMT_STRING("Leveled list {:08X}: invalid calcUseAll value '{}'"), curobj->formID, line.calcUseAll);
			} else {
				curobj->llFlags = 4;
				curobj->maxUseAllCount = static_cast<std::int8_t>(maxUseAllCount);
				logger::debug(FMT_STRING("leveled lists {:08X} set to Use All"), curobj->formID);
			}
		}

		if (!line.addedObjects.empty()) {
			auto entries = convertLLtoVec(curobj);
			bool changed = false;
			for (const auto& objectToAdd : line.addedObjects) {
				if (objectToAdd.size() < 4) {
					logger::warn(FMT_STRING("Leveled list {:08X}: malformed addToLLs entry; expected form~level~count~chanceNone"), curobj->formID);
					continue;
				}
				std::string addFormStr = objectToAdd[0];
				int levelValue = 0;
				int countValue = 0;
				int chanceNone = 0;
				if (!tryParseInt(objectToAdd[1], levelValue) || !tryParseInt(objectToAdd[2], countValue) || !tryParseInt(objectToAdd[3], chanceNone) ||
					levelValue < 0 || levelValue > (std::numeric_limits<std::uint16_t>::max)() ||
					countValue < 0 || countValue > (std::numeric_limits<std::uint16_t>::max)() ||
					chanceNone < (std::numeric_limits<std::int8_t>::min)() || chanceNone > (std::numeric_limits<std::int8_t>::max)()) {
					logger::warn(FMT_STRING("Leveled list {:08X}: invalid addToLLs values '{}~{}~{}'"), curobj->formID, objectToAdd[1], objectToAdd[2], objectToAdd[3]);
					continue;
				}
				const auto level = static_cast<std::uint16_t>(levelValue);
				const auto count = static_cast<std::uint16_t>(countValue);
				RE::TESForm* addForm = GetFormFromIdentifier(addFormStr);

				if (addForm) {
					if (entries.size() >= 255u) {
						logger::warn(FMT_STRING("Leveled list {:08X}: maximum of 255 entries reached; skipping '{}'"), curobj->formID, addFormStr);
						continue;
					}
					if (!curobj->GetCanContainFormsOfType(addForm->formType.get())) {
						logger::warn(FMT_STRING("Leveled list {:08X}: form {:08X} cannot be contained; skipping addToLLs"), curobj->formID, addForm->formID);
						continue;
					}
					RE::LEVELED_OBJECT entry{};
					entry.form = addForm;
					entry.itemExtra = nullptr;
					entry.count = count;
					entry.level = level;
					entry.chanceNone = static_cast<std::int8_t>(chanceNone);
					entries.push_back(entry);
					changed = true;
					logger::debug(FMT_STRING("leveled Lists {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				} else {
					logger::critical(FMT_STRING("leveled Lists {:08X} Form not found: {}"), curobj->formID, objectToAdd[0]);
				}
			}
			if (changed) {
				std::sort(entries.begin(), entries.end(), [](const RE::LEVELED_OBJECT& lhs, const RE::LEVELED_OBJECT& rhs) {
					return lhs.level < rhs.level;
				});
				createLL(curobj, entries);
			}
		}
	}

	void patchContainer(const LEVELEDLISTS::line_content& line, RE::TESObjectCONT* curobj)
	{
		if (!curobj || ShouldSkipPatch("leveledlist", curobj)) {
			return;
		}

		if (!line.chanceRobCo.empty() && line.chanceRobCo != "none") {
			int random_number = getRandomNumber();
			int chance = 0;
			if (!tryParseInt(line.chanceRobCo, chance)) {
				logger::warn(FMT_STRING("Container {:08X}: invalid chance value '{}'; skipping condition"), curobj->formID, line.chanceRobCo);
			} else if (random_number > chance) {
				logger::debug("Skipped {:08X} by chance {} > {}", curobj->formID, random_number, chance);
				return;
			}
		}

		if (!line.addedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.addedContainerObjects) {
				if (objectToAdd.size() < 2) {
					logger::warn(FMT_STRING("Container {:08X}: malformed addToContainers entry; expected form~count"), curobj->formID);
					continue;
				}
				std::string addFormStr = objectToAdd[0];
				int count = 0;
				if (!tryParseInt(objectToAdd[1], count) || count < 0) {
					logger::warn(FMT_STRING("Container {:08X}: invalid count '{}' in addToContainers"), curobj->formID, objectToAdd[1]);
					continue;
				}
				auto* addForm = GetFormFromIdentifier(addFormStr);
				auto* boundObject = addForm ? addForm->As<RE::TESBoundObject>() : nullptr;

				if (boundObject) {
					curobj->AddObject(boundObject, static_cast<std::int32_t>(count), nullptr);
					logger::debug(FMT_STRING("leveled Lists - container {:08X} added Form {:08X}"), curobj->formID, addForm->formID);
				} else if (addForm) {
					logger::warn(FMT_STRING("Container {:08X}: form {:08X} cannot be contained; skipping addToContainers"), curobj->formID, addForm->formID);
				} else {
					logger::warn(FMT_STRING("Container {:08X}: '{}' is not a valid TESBoundObject; skipping addToContainers"), curobj->formID, addFormStr);
				}
			}
		}

		if (!line.removedContainerObjects.empty()) {
			for (const auto& objectToAdd : line.removedContainerObjects) {
				//logger::debug(FMT_STRING("LL found: {} {} {} {}"), objectToAdd[0], objectToAdd[1], objectToAdd[2], objectToAdd[3]);
				std::string addFormStr = objectToAdd;
				auto* addForm = GetFormFromIdentifier(addFormStr);
				auto* boundObject = addForm ? addForm->As<RE::TESBoundObject>() : nullptr;

				if (boundObject) {
					VRCompat::RemoveObject(curobj, boundObject);
					logger::debug(FMT_STRING("leveled Lists - container {:08X} removed Form {:08X}"), curobj->formID, boundObject->formID);
				} else {
					logger::critical(FMT_STRING("leveled Lists - container {:08X} Form not found: {}"), curobj->formID, objectToAdd);
				}
			}
		}
	}

}
