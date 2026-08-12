#include "object_omod.h"
#include "EngineAdapters.h"
#include <cstring>
#include <utility.h>
#include <unordered_set>
using namespace RE::detail;
namespace OMOD
{
	namespace
	{
		bool ReadData(RE::BGSMod::Attachment::Mod* omod, RE::BGSMod::Attachment::Mod::Data& data)
		{
			if (EngineAdapters::ReadOmodData(omod, data)) {
				return true;
			}

			static bool logged = false;
			if (!logged) {
				logger::critical("OMOD patching disabled because the verified FO4VR data adapter is unavailable");
				logged = true;
			}
			return false;
		}
	}

	struct line_content create_patch_instruction(const std::string& line)
	{
		line_content l;

		// extract connectionAnd
		std::regex connectionAnd_regex("filterConnection\\s*=([^:]+)", regex::icase);
		std::smatch connectionAnd_match;
		regexSearchParameter(line, connectionAnd_match, connectionAnd_regex);
		// extract the value after the equals sign
		if (connectionAnd_match.empty() || connectionAnd_match[1].str().empty()) {
		} else {
			std::string value = connectionAnd_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.connectionAnd = item;
			}
		}

		// extract objects
		std::regex objects_regex("filterByOMod\\s*=([^:]+)", regex::icase);
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

		extractForms(line, "filterByOModExcluded\\s*=([^:]+)", l.objectsExcluded);

		// extract targetType
		std::regex targetType_regex("filterByFormType\\s*=([^:]+)", regex::icase);
		std::smatch targetTypematch;
		regexSearchParameter(line, targetTypematch, targetType_regex);
		// extract the value after the equals sign
		if (targetTypematch.empty() || targetTypematch[1].str().empty()) {
			l.targetType = "none";
		} else {
			std::string value = targetTypematch[1].str();
			value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			l.targetType = value;
		}



					// extract FormAnd
		std::regex FormAnd_regex("filterByForms\\s*=([^:]+)", regex::icase);
		std::smatch FormAnd_match;
		regexSearchParameter(line, FormAnd_match, FormAnd_regex);
		// extract the value after the equals sign
		if (FormAnd_match.empty() || FormAnd_match[1].str().empty()) {
		} else {
			std::string value = FormAnd_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.FormAnd.push_back(item);
			}
		}

				// extract PropertyAnd
		std::regex PropertyAnd_regex("filterByPropertiesAnd\\s*=([^:]+)", regex::icase);
		std::smatch PropertyAnd_match;
		regexSearchParameter(line, PropertyAnd_match, PropertyAnd_regex);
		// extract the value after the equals sign
		if (PropertyAnd_match.empty() || PropertyAnd_match[1].str().empty()) {
		} else {
			std::string value = PropertyAnd_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.PropertyAnd.push_back(item);
			}
		}

				// extract PropertyOr
		std::regex PropertyOr_regex("filterByPropertiesOr\\s*=([^:]+)", regex::icase);
		std::smatch PropertyOr_match;
		regexSearchParameter(line, PropertyOr_match, PropertyOr_regex);
		// extract the value after the equals sign
		if (PropertyOr_match.empty() || PropertyOr_match[1].str().empty()) {
		} else {
			std::string value = PropertyOr_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.PropertyOr.push_back(item);
			}
		}

				// extract PropertyExclude
		std::regex PropertyExclude_regex("filterByPropertiesExclude\\s*=([^:]+)", regex::icase);
		std::smatch PropertyExclude_match;
		regexSearchParameter(line, PropertyExclude_match, PropertyExclude_regex);
		// extract the value after the equals sign
		if (PropertyExclude_match.empty() || PropertyExclude_match[1].str().empty()) {
		} else {
			std::string value = PropertyExclude_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.PropertyExclude.push_back(item);
			}
		}

			// extract stringContainsAnd
		std::regex stringContainsAnd_regex("filterByNameContainsAnd\\s*=([^:]+)", regex::icase);
		std::smatch stringContainsAnd_match;
		regexSearchParameter(line, stringContainsAnd_match, stringContainsAnd_regex);
		// extract the value after the equals sign
		if (stringContainsAnd_match.empty() || stringContainsAnd_match[1].str().empty()) {
		} else {
			std::string value = stringContainsAnd_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.stringContainsAnd.push_back(item);
			}
		}

					// extract stringContainsOr
		std::regex stringContainsOr_regex("filterByNameContainsOr\\s*=([^:]+)", regex::icase);
		std::smatch stringContainsOr_match;
		regexSearchParameter(line, stringContainsOr_match, stringContainsOr_regex);
		// extract the value after the equals sign
		if (stringContainsOr_match.empty() || stringContainsOr_match[1].str().empty()) {
		} else {
			std::string value = stringContainsOr_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				l.stringContainsOr.push_back(item);
			}
		}

		// extract stringContainsExclude
		std::regex stringContainsExclude_regex("filterByNameContainsExclude\\s*=([^:]+)", regex::icase);
		std::smatch stringContainsExclude_match;
		regexSearchParameter(line, stringContainsExclude_match, stringContainsExclude_regex);
		// extract the value after the equals sign
		if (stringContainsExclude_match.empty() || stringContainsExclude_match[1].str().empty()) {
		} else {
			std::string value = stringContainsExclude_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				l.stringContainsExclude.push_back(item);
			}
		}

				// extract keywords
		std::regex keywords_regex("filterByAttachPoint\\s*=([^:]+)", regex::icase);
		std::smatch keywords_match;
		regexSearchParameter(line, keywords_match, keywords_regex);
		std::vector<std::string> keywords;
		if (keywords_match.empty() || keywords_match[1].str().empty()) {
			//empty
		} else {
			std::string keywords_str = keywords_match[1];
			std::regex keywords_list_regex("[^,]+", regex::icase);
			std::sregex_iterator keywords_iterator(keywords_str.begin(), keywords_str.end(), keywords_list_regex);
			std::sregex_iterator keywords_end;
			while (keywords_iterator != keywords_end) {
				std::string keyword = (*keywords_iterator)[0].str();
				keyword.erase(keyword.begin(), std::find_if_not(keyword.begin(), keyword.end(), ::isspace));
				keyword.erase(std::find_if_not(keyword.rbegin(), keyword.rend(), ::isspace).base(), keyword.end());
				if (keyword != "none") {
					keywords.push_back(keyword);
				}
				++keywords_iterator;
			}
			l.attachPointKeywordsFilter = keywords;
		}

		std::regex property_regex("changeOModPropertiesFloat\\s*=([^:]+)", regex::icase);
		std::smatch property_match;
		regexSearchParameter(line, property_match, property_regex);
		std::vector<std::string> property;
		if (property_match.empty() || property_match[1].str().empty()) {
			//empty
		} else {
			std::string property_str = property_match[1];
			std::regex pattern("(\\w+)\\s*=\\s*([^,]+)", regex::icase);

			auto begin = std::sregex_iterator(property_str.begin(), property_str.end(), pattern);
			auto end = std::sregex_iterator();

			for (std::sregex_iterator i = begin; i != end; ++i) {
				std::smatch match = *i;
				l.properties.push_back(match[1]);
				l.propertiesValues.push_back(match[2]);
				//logger::debug(FMT_STRING("Match: {} {}"), match[1].str(), match[2].str());
			}
		}

		std::regex propertyVP_regex("changeOModPropertiesVP\\s*=([^:]+)", regex::icase);
		std::smatch propertyVP_match;
		regexSearchParameter(line, propertyVP_match, propertyVP_regex);
		std::vector<std::string> propertyVP;
		if (propertyVP_match.empty() || propertyVP_match[1].str().empty()) {
			//empty
		} else {
			std::string propertyVP_str = propertyVP_match[1];
			std::regex pattern("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([^,]+)", regex::icase);

			auto begin = std::sregex_iterator(propertyVP_str.begin(), propertyVP_str.end(), pattern);
			auto end = std::sregex_iterator();

			for (std::sregex_iterator i = begin; i != end; ++i) {
				std::smatch match = *i;
				l.propertiesVP.push_back(match[1]);
				l.propertiesVPValues.push_back(match[2]);
				//logger::debug(FMT_STRING("Match: {} {}"), match[1].str(), match[2].str());
			}
		}

		std::regex propertyForm_regex("changeOModPropertiesForm\\s*=([^:]+)", regex::icase);
		std::smatch propertyForm_match;
		regexSearchParameter(line, propertyForm_match, propertyForm_regex);
		std::vector<std::string> propertyForm;
		if (propertyForm_match.empty() || propertyForm_match[1].str().empty()) {
			//empty
		} else {
			std::string propertyForm_str = propertyForm_match[1];
			std::regex pattern("([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})\\s*=\\s*([^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8})", regex::icase);

			auto begin = std::sregex_iterator(propertyForm_str.begin(), propertyForm_str.end(), pattern);
			auto end = std::sregex_iterator();

			for (std::sregex_iterator i = begin; i != end; ++i) {
				std::smatch match = *i;
				l.propertiesForm.push_back(match[1]);
				l.propertiesFormValues.push_back(match[2]);
				//logger::debug(FMT_STRING("Match: {} {}"), match[1].str(), match[2].str());
			}
		}

		// extract removeProperty
		std::regex removeProperty_regex("removeOModProperties\\s*=([^:]+)", regex::icase);
		std::smatch removeProperty_match;
		regexSearchParameter(line, removeProperty_match, removeProperty_regex);
		// extract the value after the equals sign
		if (removeProperty_match.empty() || removeProperty_match[1].str().empty()) {
		} else {
			std::string value = removeProperty_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.propertiesToRemove.push_back(item);
			}
		}

				// extract addProperty
		std::regex addProperty_regex("oModPropertiesToAdd\\s*=([^:]+)", regex::icase);
		std::smatch addProperty_match;
		regexSearchParameter(line, addProperty_match, addProperty_regex);
		// extract the value after the equals sign
		if (addProperty_match.empty() || addProperty_match[1].str().empty()) {
		} else {
			std::string value = addProperty_match[1].str();
			value = std::regex_replace(value, std::regex("form\\s*,\\s*int", std::regex::icase), "formint");
			value = std::regex_replace(value, std::regex("form(?:id)?\\s*,\\s*float", std::regex::icase), "pair");
			//value.erase(std::add_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.propertiesToAdd.push_back(item);
			}
		}

				// extract removePropertyForm
		std::regex removePropertyForm_regex("removeOModPropertiesForm\\s*=([^:]+)", regex::icase);
		std::smatch removePropertyForm_match;
		regexSearchParameter(line, removePropertyForm_match, removePropertyForm_regex);
		// extract the value after the equals sign
		if (removePropertyForm_match.empty() || removePropertyForm_match[1].str().empty()) {
		} else {
			std::string value = removePropertyForm_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.propertiesToRemoveForm.push_back(item);
			}
		}

				// extract removePropertyPV
		std::regex removePropertyPV_regex("removeOModPropertiesVP\\s*=([^:]+)", regex::icase);
		std::smatch removePropertyPV_match;
		regexSearchParameter(line, removePropertyPV_match, removePropertyPV_regex);
		// extract the value after the equals sign
		if (removePropertyPV_match.empty() || removePropertyPV_match[1].str().empty()) {
		} else {
			std::string value = removePropertyPV_match[1].str();
			//value.erase(std::remove_if(value.begin(), value.end(), ::isspace), value.end());
			std::stringstream ss(value);
			std::string item;
			while (std::getline(ss, item, ',')) {
				item.erase(item.begin(), std::find_if_not(item.begin(), item.end(), ::isspace));
				item.erase(std::find_if_not(item.rbegin(), item.rend(), ::isspace).base(), item.end());
				//logger::debug(FMT_STRING("SearchString {}"), item);
				l.propertiesToRemovePV.push_back(item);
			}
		}

		// extract fullName
		std::regex fullName_regex("fullName\\s*=\\s*~([^~]+?)\\s*~");
		std::smatch namematch;
		regexSearchParameter(line, namematch, fullName_regex);
		// extract the value after the equals sign
		if (namematch.empty() || namematch[1].str().empty()) {
			l.fullName = "none";
		} else {
			std::string namevalue = namematch[1].str();
			namevalue.erase(namevalue.begin(), std::find_if_not(namevalue.begin(), namevalue.end(), ::isspace));
			namevalue.erase(std::find_if_not(namevalue.rbegin(), namevalue.rend(), ::isspace).base(), namevalue.end());
			l.fullName = namevalue;
			//logger::debug(FMT_STRING("omod filter {}"), l.fullName);
		}

		// extract functionType
		std::regex functionType_regex("changeOModFunctionType\\s*=([^:]+)", regex::icase);
		std::smatch functionType_match;
		regexSearchParameter(line, functionType_match, functionType_regex);
		std::vector<std::string> functionType;
		if (functionType_match.empty() || functionType_match[1].str().empty()) {
			//empty
		} else {
			std::string functionType_str = functionType_match[1];
			std::regex pattern("(\\w+)\\s*=\\s*([^,]+)", regex::icase);

			auto begin = std::sregex_iterator(functionType_str.begin(), functionType_str.end(), pattern);
			auto end = std::sregex_iterator();

			for (std::sregex_iterator i = begin; i != end; ++i) {
				std::smatch match = *i;
				l.functionTypeProperties.push_back(match[1]);
				l.functionTypeValues.push_back(match[2]);
				//logger::debug(FMT_STRING("Match: {} {}"), match[1].str(), match[2].str());
			}
		}

		// extract attachParentSlotKeywordsToAdd
		extractForms(line, "attachParentSlotKeywordsToAdd\\s*=([^:]+)", l.attachParentSlotKeywordsToAdd);
		//std::regex attachParentSlotKeywordsToAdd_regex("attachParentSlotKeywordsToAdd\\s*=([^:]+)", regex::icase);
		//std::smatch attachParentSlotKeywordsToAdd_match;
		//std::regex_search(line, attachParentSlotKeywordsToAdd_match, attachParentSlotKeywordsToAdd_regex);
		//std::vector<std::string> attachParentSlotKeywordsToAdd;
		//if (attachParentSlotKeywordsToAdd_match.empty() || attachParentSlotKeywordsToAdd_match[1].str().empty()) {
		//	// ammos_match[1] is null
		//} else {
		//	std::string attachParentSlotKeywordsToAdd_str = attachParentSlotKeywordsToAdd_match[1];
		//	std::regex attachParentSlotKeywordsToAdd_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator attachParentSlotKeywordsToAdd_iterator(attachParentSlotKeywordsToAdd_str.begin(), attachParentSlotKeywordsToAdd_str.end(), attachParentSlotKeywordsToAdd_list_regex);
		//	std::sregex_iterator attachParentSlotKeywordsToAdd_end;
		//	while (attachParentSlotKeywordsToAdd_iterator != attachParentSlotKeywordsToAdd_end) {
		//		std::string keywordToAdd = (*attachParentSlotKeywordsToAdd_iterator)[0].str();
		//		keywordToAdd.erase(keywordToAdd.begin(), std::find_if_not(keywordToAdd.begin(), keywordToAdd.end(), ::isspace));
		//		keywordToAdd.erase(std::find_if_not(keywordToAdd.rbegin(), keywordToAdd.rend(), ::isspace).base(), keywordToAdd.end());
		//		if (keywordToAdd != "none") {
		//			//logger::info(FMT_STRING("attachParentSlotKeywordsToAdd: {}"), keywordToAdd);
		//			attachParentSlotKeywordsToAdd.push_back(keywordToAdd);
		//		}
		//		++attachParentSlotKeywordsToAdd_iterator;
		//	}
		//	l.attachParentSlotKeywordsToAdd = attachParentSlotKeywordsToAdd;
		//}

		// extract attachParentSlotKeywordsToRemove
		extractForms(line, "attachParentSlotKeywordsToRemove\\s*=([^:]+)", l.attachParentSlotKeywordsToRemove);
		//std::regex attachParentSlotKeywordsToRemove_regex("attachParentSlotKeywordsToRemove\\s*=([^:]+)", regex::icase);
		//std::smatch attachParentSlotKeywordsToRemove_match;
		//std::regex_search(line, attachParentSlotKeywordsToRemove_match, attachParentSlotKeywordsToRemove_regex);
		//std::vector<std::string> attachParentSlotKeywordsToRemove;
		//if (attachParentSlotKeywordsToRemove_match.empty() || attachParentSlotKeywordsToRemove_match[1].str().empty()) {
		//	// ammos_match[1] is null
		//} else {
		//	std::string attachParentSlotKeywordsToRemove_str = attachParentSlotKeywordsToRemove_match[1];
		//	std::regex attachParentSlotKeywordsToRemove_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
		//	std::sregex_iterator attachParentSlotKeywordsToRemove_iterator(attachParentSlotKeywordsToRemove_str.begin(), attachParentSlotKeywordsToRemove_str.end(), attachParentSlotKeywordsToRemove_list_regex);
		//	std::sregex_iterator attachParentSlotKeywordsToRemove_end;
		//	while (attachParentSlotKeywordsToRemove_iterator != attachParentSlotKeywordsToRemove_end) {
		//		std::string keywordToRemove = (*attachParentSlotKeywordsToRemove_iterator)[0].str();
		//		keywordToRemove.erase(keywordToRemove.begin(), std::find_if_not(keywordToRemove.begin(), keywordToRemove.end(), ::isspace));
		//		keywordToRemove.erase(std::find_if_not(keywordToRemove.rbegin(), keywordToRemove.rend(), ::isspace).base(), keywordToRemove.end());
		//		if (keywordToRemove != "none") {
		//			//logger::info(FMT_STRING("attachParentSlotKeywordsToRemove: {}"), keywordToRemove);
		//			attachParentSlotKeywordsToRemove.push_back(keywordToRemove);
		//		}
		//		++attachParentSlotKeywordsToRemove_iterator;
		//	}
		//	l.attachParentSlotKeywordsToRemove = attachParentSlotKeywordsToRemove;
		//}

		// extract attachPoint
		extractValueString(line, "setAttachPoint\\s*=([^:]+)", l.attachPoint);
		//std::regex attachPoint_regex("setAttachPoint\\s*=([^:]+)", regex::icase);
		//std::smatch attachPointmatch;
		//std::regex_search(line, attachPointmatch, attachPoint_regex);
		//// extract the value after the equals sign
		//if (attachPointmatch.empty() || attachPointmatch[1].str().empty()) {
		//	l.attachPoint = "none";
		//} else {
		//	std::string keyword = attachPointmatch[1].str();
		//	keyword.erase(std::remove_if(keyword.begin(), keyword.end(), ::isspace), keyword.end());
		//	l.attachPoint = keyword;
		//}


		static const std::regex unsupportedTargetKeywordRegex(R"((^|:)\s*targetKeywordsTo(Add|Remove)\s*=)", std::regex::icase);
		if (std::regex_search(line, unsupportedTargetKeywordRegex)) {
			logger::warn("OMOD targetKeywordsToAdd/Remove is unsupported by both the flat and VR mutation paths; skipping the directive");
		}

		extractDataStrings(line, "filterByModNames\\s*=([^:]+)", l.modNames);

		return l;
	}

	void process_patch_instructions(const std::list<line_content>& tokens)
	{
		logger::debug("processing patch instructions");
		auto* dataHandler = RE::TESDataHandler::GetSingleton();
		if (!dataHandler) {
			logger::warn("OMOD Patcher: TESDataHandler is unavailable");
			return;
		}
		const auto& objectArray = dataHandler->GetFormArray<RE::BGSMod::Attachment::Mod>();

		for (const auto& line : tokens) {
			const auto connection = toLowerCase(line.connectionAnd);
			const bool connectWithAnd = connection == "and";
			if (!connection.empty() && connection != "none" && connection != "or" && !connectWithAnd) {
				logger::warn(FMT_STRING("OMOD Patcher: unknown filterConnection '{}'; expected 'and', 'or', or 'none'. Using OR."), line.connectionAnd);
			}

			const auto targetTypeName = toLowerCase(line.targetType);
			const bool hasTargetType = !targetTypeName.empty() && targetTypeName != "none";
			RE::ENUM_FORM_ID expectedTargetType = RE::ENUM_FORM_ID::kNONE;
			if (hasTargetType) {
				if (targetTypeName == "weapon") {
					expectedTargetType = RE::ENUM_FORM_ID::kWEAP;
				} else if (targetTypeName == "armor") {
					expectedTargetType = RE::ENUM_FORM_ID::kARMO;
				} else if (targetTypeName == "npc") {
					expectedTargetType = RE::ENUM_FORM_ID::kNPC_;
				} else {
					logger::warn(FMT_STRING("OMOD Patcher: unknown filterByFormType '{}'; rule skipped"), line.targetType);
					continue;
				}
			}

			std::unordered_set<std::uint32_t> selectedOMods;
			for (const auto& identifier : line.objects) {
				auto* form = GetFormFromIdentifier(identifier);
				if (!form || form->formType != RE::ENUM_FORM_ID::kOMOD) {
					logger::warn(FMT_STRING("OMOD Patcher: filterByOMod form not found or not an OMOD: {}"), identifier);
					continue;
				}
				selectedOMods.insert(form->formID);
			}

			std::unordered_set<std::uint32_t> excludedOMods;
			for (const auto& identifier : line.objectsExcluded) {
				auto* form = GetFormFromIdentifier(identifier);
				if (!form || form->formType != RE::ENUM_FORM_ID::kOMOD) {
					logger::warn(FMT_STRING("OMOD Patcher: filterByOModExcluded form not found or not an OMOD: {}"), identifier);
					continue;
				}
				excludedOMods.insert(form->formID);
			}

			std::vector<std::uint32_t> referencedForms;
			bool referencedFormsValid = true;
			for (const auto& identifier : line.FormAnd) {
				auto* form = GetFormFromIdentifier(identifier);
				if (!form) {
					logger::warn(FMT_STRING("OMOD Patcher: filterByForms form not found: {}"), identifier);
					referencedFormsValid = false;
					continue;
				}
				referencedForms.push_back(form->formID);
			}

			std::vector<std::uint32_t> attachPoints;
			for (const auto& identifier : line.attachPointKeywordsFilter) {
				auto* form = GetFormFromIdentifier(identifier);
				if (!form) {
					logger::warn(FMT_STRING("OMOD Patcher: filterByAttachPoint form not found: {}"), identifier);
					continue;
				}
				attachPoints.push_back(form->formID);
			}

			auto lowerValues = [](const std::vector<std::string>& values) {
				std::vector<std::string> result;
				result.reserve(values.size());
				for (const auto& value : values) {
					result.push_back(toLowerCase(value));
				}
				return result;
			};
			const auto nameAndValues = lowerValues(line.stringContainsAnd);
			const auto nameOrValues = lowerValues(line.stringContainsOr);
			const auto nameExcludeValues = lowerValues(line.stringContainsExclude);

			for (auto* curobj : objectArray) {
				if (!curobj || curobj->IsDeleted() || excludedOMods.contains(curobj->formID) || !FormMatchesModNames(curobj, line.modNames)) {
					continue;
				}

				RE::BGSMod::Attachment::Mod::Data data{};
				if (!ReadData(curobj, data)) {
					continue;
				}
				if (hasTargetType && data.targetFormType.get() != expectedTargetType) {
					continue;
				}

				const bool needsProperties = !line.PropertyAnd.empty() || !line.PropertyOr.empty() ||
					!line.PropertyExclude.empty() || !line.FormAnd.empty();
				if (needsProperties && data.propertyModCount > 0 && !data.propertyMods) {
					logger::warn(FMT_STRING("OMOD {:08X}: property count is non-zero but property data is missing"), curobj->formID);
					continue;
				}

				const bool directMatch = !line.objects.empty() && selectedOMods.contains(curobj->formID);
				std::string lowercaseFullName;
				if (!nameAndValues.empty() || !nameOrValues.empty() || !nameExcludeValues.empty()) {
					lowercaseFullName = toLowerCase(std::string(curobj->fullName.c_str()));
				}

				const bool nameAndMatch = nameAndValues.empty() || std::all_of(nameAndValues.begin(), nameAndValues.end(), [&](const auto& value) {
					return lowercaseFullName.find(value) != std::string::npos;
				});
				const bool nameOrMatch = nameOrValues.empty() || std::any_of(nameOrValues.begin(), nameOrValues.end(), [&](const auto& value) {
					return lowercaseFullName.find(value) != std::string::npos;
				});

				auto hasProperty = [&](const std::string& name) {
					const auto propertyID = getPropertyFromString(name, data.targetFormType.get());
					if (propertyID < 0) {
						return false;
					}
					for (std::uint32_t index = 0; index < data.propertyModCount; ++index) {
						if (data.propertyMods[index].target == static_cast<std::uint32_t>(propertyID)) {
							return true;
						}
					}
					return false;
				};
				const bool propertyAndMatch = line.PropertyAnd.empty() || std::all_of(line.PropertyAnd.begin(), line.PropertyAnd.end(), hasProperty);
				const bool propertyOrMatch = line.PropertyOr.empty() || std::any_of(line.PropertyOr.begin(), line.PropertyOr.end(), hasProperty);

				bool formsMatch = line.FormAnd.empty();
				if (!line.FormAnd.empty() && referencedFormsValid && referencedForms.size() == line.FormAnd.size()) {
					formsMatch = std::all_of(referencedForms.begin(), referencedForms.end(), [&](std::uint32_t formID) {
						for (std::uint32_t index = 0; index < data.propertyModCount; ++index) {
							const auto& property = data.propertyMods[index];
							if (property.type == RE::BGSMod::Property::TYPE::kForm && property.data.form && property.data.form->formID == formID) {
								return true;
							}
							if (property.type == RE::BGSMod::Property::TYPE::kPair && property.data.fv.formID == formID) {
								return true;
							}
						}
						return false;
					});
				}

				bool attachPointMatch = line.attachPointKeywordsFilter.empty();
				if (!line.attachPointKeywordsFilter.empty()) {
					const auto* attachPoint = BGSKeywordGetTypedKeywordByIndex(RE::KeywordType::kAttachPoint, curobj->attachPoint.keywordIndex);
					attachPointMatch = attachPoint && std::find(attachPoints.begin(), attachPoints.end(), attachPoint->formID) != attachPoints.end();
				}

				std::size_t suppliedFilters = 0;
				std::size_t matchedFilters = 0;
				auto addFilterResult = [&](bool supplied, bool matched) {
					if (supplied) {
						++suppliedFilters;
						matchedFilters += matched ? 1u : 0u;
					}
				};
				addFilterResult(!line.objects.empty(), directMatch);
				addFilterResult(!line.stringContainsAnd.empty(), nameAndMatch);
				addFilterResult(!line.stringContainsOr.empty(), nameOrMatch);
				addFilterResult(!line.PropertyAnd.empty(), propertyAndMatch);
				addFilterResult(!line.PropertyOr.empty(), propertyOrMatch);
				addFilterResult(!line.FormAnd.empty(), formsMatch);
				addFilterResult(!line.attachPointKeywordsFilter.empty(), attachPointMatch);

				const bool foundByPositiveFilters = suppliedFilters == 0 ||
					(connectWithAnd ? matchedFilters == suppliedFilters : matchedFilters > 0);
				bool found = foundByPositiveFilters;

				if (found && !nameExcludeValues.empty()) {
					found = std::none_of(nameExcludeValues.begin(), nameExcludeValues.end(), [&](const auto& value) {
						return lowercaseFullName.find(value) != std::string::npos;
					});
				}
				if (found && !line.PropertyExclude.empty()) {
					found = std::none_of(line.PropertyExclude.begin(), line.PropertyExclude.end(), hasProperty);
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
								PATCH::RecordFile("objectmodification", fullPath);
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

									PATCH::RecordRule("objectmodification");
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

	void patch(const OMOD::line_content& line, RE::BGSMod::Attachment::Mod* curobj)
	{
		if (!curobj || ShouldSkipPatch("objectmodification", curobj)) {
			return;
		}

		RE::BGSMod::Attachment::Mod::Data targetData;
		if (!ReadData(curobj, targetData)) {
			return;
		}
		const auto targetFormType = targetData.targetFormType.get();

		if (!line.properties.empty()) {
			for (uint32_t i = 0; i < line.properties.size(); i++) {
				if (i >= line.propertiesValues.size()) {
					logger::warn(FMT_STRING("OMOD {:08X}: missing value for property '{}'"), curobj->formID, line.properties[i]);
					continue;
				}
				int propertyId = getPropertyFromString(line.properties[i], targetFormType);
				RE::BGSMod::Attachment::Mod::Data data;
				if (!ReadData(curobj, data)) {
					continue;
				}
				if (data.propertyModCount == 0 || !data.propertyMods) {
					continue;
				}
				if (data.propertyModCount > 0 && !data.propertyMods) {
					logger::warn(FMT_STRING("OMOD {:08X}: property count is non-zero but property data is missing"), curobj->formID);
					continue;
				}

				for (uint32_t j = 0; j < data.propertyModCount; j++) {
					auto& mod = data.propertyMods[j];
					auto type = mod.type;

					switch (type) {
					case RE::BGSMod::Property::TYPE::kInt:
						//logger::debug(FMT_STRING("OMOD Data Type {} kInt {} {} | {} {}"), std::to_string(mod.target), mod.data.mm.min.i, mod.data.mm.max.i, mod.data.mm.min.f, mod.data.mm.max.f);
						break;
					case RE::BGSMod::Property::TYPE::kFloat:
						if (propertyId == static_cast<int>(mod.target)) {
							try {
								mod.data.mm.min.f = std::stof(line.propertiesValues[i]);
								logger::debug(FMT_STRING("omod {:08X} {} Value set {} to {}"), curobj->formID, curobj->fullName, line.properties[i], mod.data.mm.min.f);
							} catch (const std::exception& e) {
								logger::warn(FMT_STRING("OMOD {:08X}: invalid value '{}' for property '{}': {}"), curobj->formID, line.propertiesValues[i], line.properties[i], e.what());
							}
						}
						break;
					case RE::BGSMod::Property::TYPE::kBool:
						//logger::debug(FMT_STRING("OMOD Data Editor ID {:08X}"), mod.data.fv.formID);
						break;
					case RE::BGSMod::Property::TYPE::kString:
						break;
					case RE::BGSMod::Property::TYPE::kForm:
						//if (mod.data.mm.min.i != 0)
						//	logger::debug(FMT_STRING("OMOD Data Type {} kForm Editor ID {:08X}"), std::to_string(mod.target), mod.data.form->formID);
						break;
					case RE::BGSMod::Property::TYPE::kEnum:
						break;
					case RE::BGSMod::Property::TYPE::kPair:
						//logger::debug(FMT_STRING("OMOD Data Type {} kPair {:08X} value {} fmin {} fmax {} imin {} imax {} "), std::to_string(mod.target), mod.data.fv.formID, mod.data.fv.value, mod.data.mm.min.f, mod.data.mm.max.f, mod.data.mm.min.i, mod.data.mm.max.i);
						//mod.data.fv.value = 20.0;
						//logger::debug(FMT_STRING("OMOD Data Type {} kPair {:08X} value {} fmin {} fmax {} imin {} imax {} "), std::to_string(mod.target), mod.data.fv.formID, mod.data.fv.value, mod.data.mm.min.f, mod.data.mm.max.f, mod.data.mm.min.i, mod.data.mm.max.i);
						break;
					}
				}
			}
		}

		if (!line.propertiesVP.empty()) {
			for (uint32_t i = 0; i < line.propertiesVP.size(); i++) {
				if (i >= line.propertiesVPValues.size()) {
					logger::warn(FMT_STRING("OMOD {:08X}: missing pair value for '{}'"), curobj->formID, line.propertiesVP[i]);
					continue;
				}
				RE::BGSMod::Attachment::Mod::Data data;
				if (!ReadData(curobj, data)) {
					continue;
				}
				if (i >= line.propertiesVPValues.size() || data.propertyModCount == 0 || !data.propertyMods) {
					logger::warn(FMT_STRING("OMOD {:08X}: invalid changeOModPropertiesVP entry or missing property data"), curobj->formID);
					continue;
				}
				if (data.propertyModCount > 0 && !data.propertyMods) {
					logger::warn(FMT_STRING("OMOD {:08X}: property count is non-zero but property data is missing"), curobj->formID);
					continue;
				}

				for (uint32_t j = 0; j < data.propertyModCount; j++) {
					auto& mod = data.propertyMods[j];
					auto type = mod.type;

					switch (type) {
					case RE::BGSMod::Property::TYPE::kInt:
						//logger::debug(FMT_STRING("OMOD Data Type {} kInt {} {} | {} {}"), std::to_string(mod.target), mod.data.mm.min.i, mod.data.mm.max.i, mod.data.mm.min.f, mod.data.mm.max.f);
						break;
					case RE::BGSMod::Property::TYPE::kFloat:
						break;
					case RE::BGSMod::Property::TYPE::kBool:
						//logger::debug(FMT_STRING("OMOD Data Editor ID {:08X}"), mod.data.fv.formID);
						break;
					case RE::BGSMod::Property::TYPE::kString:
						break;
					case RE::BGSMod::Property::TYPE::kForm:
						//if (mod.data.mm.min.i != 0)
						//	logger::debug(FMT_STRING("OMOD Data Type {} kForm Editor ID {:08X}"), std::to_string(mod.target), mod.data.form->formID);
						break;
					case RE::BGSMod::Property::TYPE::kEnum:
						break;
					case RE::BGSMod::Property::TYPE::kPair:
						RE::TESForm* currentform = GetFormFromIdentifier(line.propertiesVP[i]);
						if (currentform && mod.data.fv.formID == currentform->formID) {
							try {
								mod.data.fv.value = std::stof(line.propertiesVPValues[i]);
								logger::debug(FMT_STRING("omod {:08X} {} Value set {:08X} to {}"), curobj->formID, curobj->fullName, currentform->formID, mod.data.fv.value);
							} catch (const std::exception& e) {
								logger::warn(FMT_STRING("OMOD {:08X}: invalid pair value '{}' for '{}': {}"), curobj->formID, line.propertiesVPValues[i], line.propertiesVP[i], e.what());
							}
						}

						break;
					}
				}
			}
		}

		if (!line.propertiesForm.empty()) {
			for (uint32_t i = 0; i < line.propertiesForm.size(); i++) {
				if (i >= line.propertiesFormValues.size()) {
					logger::warn(FMT_STRING("OMOD {:08X}: missing replacement form for '{}'"), curobj->formID, line.propertiesForm[i]);
					continue;
				}
				RE::BGSMod::Attachment::Mod::Data data;
				if (!ReadData(curobj, data)) {
					continue;
				}
				if (i >= line.propertiesFormValues.size() || data.propertyModCount == 0 || !data.propertyMods) {
					logger::warn(FMT_STRING("OMOD {:08X}: invalid changeOModPropertiesForm entry or missing property data"), curobj->formID);
					continue;
				}
				if (data.propertyModCount > 0 && !data.propertyMods) {
					logger::warn(FMT_STRING("OMOD {:08X}: property count is non-zero but property data is missing"), curobj->formID);
					continue;
				}

				bool replaced = false;
				for (uint32_t j = 0; j < data.propertyModCount; j++) {
					auto& mod = data.propertyMods[j];
					auto type = mod.type;

					switch (type) {
					case RE::BGSMod::Property::TYPE::kInt:
						//logger::debug(FMT_STRING("OMOD Data Type {} kInt {} {} | {} {}"), std::to_string(mod.target), mod.data.mm.min.i, mod.data.mm.max.i, mod.data.mm.min.f, mod.data.mm.max.f);
						break;
					case RE::BGSMod::Property::TYPE::kFloat:
						break;
					case RE::BGSMod::Property::TYPE::kBool:
						//logger::debug(FMT_STRING("OMOD Data Editor ID {:08X}"), mod.data.fv.formID);
						break;
					case RE::BGSMod::Property::TYPE::kString:
						break;
					case RE::BGSMod::Property::TYPE::kForm: {
						if (i < line.propertiesForm.size()) {
							RE::TESForm* currentform = GetFormFromIdentifier(line.propertiesForm[i]);
							RE::TESForm* currentformValue = GetFormFromIdentifier(line.propertiesFormValues[i]);
							// Compare the stored pointer directly. Never dereference it here:
							// malformed native OMOD data can contain a small integer (e.g. 0x3C).
							if (currentform && currentformValue && mod.data.form == currentform) {
								mod.data.form = currentformValue;
								replaced = true;
								logger::debug(FMT_STRING("omod {:08X} {} changed form {:08X} to {:08X}"), curobj->formID, curobj->fullName, currentform->formID, currentformValue->formID);
							}
						}
						break;
					}
					case RE::BGSMod::Property::TYPE::kEnum:
						break;
					case RE::BGSMod::Property::TYPE::kPair:

						break;
					}
				}
				if (!replaced && !line.objects.empty()) {
					logger::debug(FMT_STRING("omod {:08X} {} did not contain replacement source form {}"),
						curobj->formID,
						curobj->fullName,
						line.propertiesForm[i]);
				}
			}
		}

		if (!line.propertiesToRemoveForm.empty() || !line.propertiesToRemovePV.empty() || !line.propertiesToRemove.empty()) {
			RE::BGSMod::Attachment::Mod::Data data{};
			if (!ReadData(curobj, data)) {
				return;
			}
			if (data.propertyModCount > 0 && !data.propertyMods) {
				logger::warn(FMT_STRING("OMOD {:08X}: property count is non-zero but property data is missing"), curobj->formID);
			} else if (data.propertyMods && data.propertyModCount > 0) {
				std::unordered_set<const RE::TESForm*> formsToRemove;
				std::unordered_set<std::uint32_t> pairFormsToRemove;
				std::unordered_set<std::uint32_t> propertyIDsToRemove;

				for (const auto& identifier : line.propertiesToRemoveForm) {
					if (const auto* form = GetFormFromIdentifier(identifier)) {
						formsToRemove.insert(form);
					} else {
						logger::warn(FMT_STRING("OMOD {:08X}: remove form not found '{}'"), curobj->formID, identifier);
					}
				}
				for (const auto& identifier : line.propertiesToRemovePV) {
					if (const auto* form = GetFormFromIdentifier(identifier)) {
						pairFormsToRemove.insert(form->formID);
					} else {
						logger::warn(FMT_STRING("OMOD {:08X}: remove pair form not found '{}'"), curobj->formID, identifier);
					}
				}
				for (const auto& name : line.propertiesToRemove) {
					const auto propertyID = getPropertyFromString(name, targetFormType);
					if (propertyID <= 0x7FF) {
						propertyIDsToRemove.insert(propertyID);
					} else {
						logger::warn(FMT_STRING("OMOD {:08X}: unknown property '{}'"), curobj->formID, name);
					}
				}

				auto shouldRemove = [&](const RE::BGSMod::Property::Mod& property) {
					if (propertyIDsToRemove.contains(property.target)) {
						return true;
					}
					if (property.type == RE::BGSMod::Property::TYPE::kForm && formsToRemove.contains(property.data.form)) {
						return true;
					}
					return property.type == RE::BGSMod::Property::TYPE::kPair && pairFormsToRemove.contains(property.data.fv.formID);
				};

				std::uint32_t removedCount = 0;
				for (std::uint32_t i = 0; i < data.propertyModCount; ++i) {
					removedCount += shouldRemove(data.propertyMods[i]) ? 1u : 0u;
				}

				if (removedCount > 0) {
					const auto attachmentCount = data.attachments ? data.attachmentCount : 0u;
					std::unique_ptr<RE::BGSMod::Attachment::Instance[]> rebuiltAttachments;
					if (attachmentCount > 0) {
						rebuiltAttachments = std::make_unique<RE::BGSMod::Attachment::Instance[]>(attachmentCount);
						std::memcpy(rebuiltAttachments.get(), data.attachments, attachmentCount * sizeof(RE::BGSMod::Attachment::Instance));
					}

					const auto remainingCount = data.propertyModCount - removedCount;
					std::unique_ptr<RE::BGSMod::Property::Mod[]> rebuiltProperties;
					if (remainingCount > 0) {
						rebuiltProperties = std::make_unique<RE::BGSMod::Property::Mod[]>(remainingCount);
						std::uint32_t destination = 0;
						for (std::uint32_t i = 0; i < data.propertyModCount; ++i) {
							if (!shouldRemove(data.propertyMods[i])) {
								std::memcpy(std::addressof(rebuiltProperties[destination++]), std::addressof(data.propertyMods[i]), sizeof(RE::BGSMod::Property::Mod));
							}
						}
					}

					data.attachments = rebuiltAttachments.get();
					data.attachmentCount = attachmentCount;
					data.propertyMods = rebuiltProperties.get();
					data.propertyModCount = remainingCount;
					if (EngineAdapters::ReplaceOmodData(curobj, data)) {
						logger::debug(FMT_STRING("OMOD {:08X}: removed {} property definition(s)"), curobj->formID, removedCount);
					} else {
						logger::critical(FMT_STRING("OMOD {:08X}: property removal rejected; original buffer preserved"), curobj->formID);
					}
				}
			}
		}

		if (!line.propertiesToAdd.empty()) {
			RE::BGSMod::Attachment::Mod::Data data;
			if (!ReadData(curobj, data)) {
				return;
			}
			const auto existingCount = data.propertyMods ? data.propertyModCount : 0u;
			const auto capacity = existingCount + static_cast<std::uint32_t>(line.propertiesToAdd.size());
			const auto attachmentCount = data.attachments ? data.attachmentCount : 0u;
			std::unique_ptr<RE::BGSMod::Attachment::Instance[]> rebuiltAttachments;
			if (attachmentCount > 0) {
				rebuiltAttachments = std::make_unique<RE::BGSMod::Attachment::Instance[]>(attachmentCount);
				std::memcpy(rebuiltAttachments.get(), data.attachments, attachmentCount * sizeof(RE::BGSMod::Attachment::Instance));
			}
			std::unique_ptr<RE::BGSMod::Property::Mod[]> rebuilt;
			std::uint32_t rebuiltCount = existingCount;
			if (capacity > 0) {
				rebuilt = std::make_unique<RE::BGSMod::Property::Mod[]>(capacity);
			}
			if (data.propertyMods && data.propertyModCount > 0) {
				for (std::uint32_t i = 0; i < data.propertyModCount; ++i) {
					std::memcpy(std::addressof(rebuilt[i]), std::addressof(data.propertyMods[i]), sizeof(RE::BGSMod::Property::Mod));
				}
			}

			bool changed = false;
			std::size_t addedCount = 0;
			for (const auto& definition : line.propertiesToAdd) {
				std::vector<std::string> parts;
				std::stringstream parser(definition);
				std::string part;
				while (std::getline(parser, part, '~')) {
					parts.push_back(trim(part));
				}
				if (parts.size() < 4) {
					logger::warn(FMT_STRING("OMOD {:08X}: invalid oModPropertiesToAdd '{}'; expected property~type~operation~value"), curobj->formID, definition);
					continue;
				}

				const int propertyId = getPropertyFromString(parts[0], targetFormType);
				if (propertyId < 0 || propertyId > 0x7FF) {
					logger::warn(FMT_STRING("OMOD {:08X}: unknown property '{}'"), curobj->formID, parts[0]);
					continue;
				}

				if (!rebuilt || rebuiltCount >= capacity) {
					logger::warn(FMT_STRING("OMOD {:08X}: property buffer capacity exceeded"), curobj->formID);
					break;
				}
				RE::BGSMod::Property::Mod& property = rebuilt[rebuiltCount];
				property.data.form = nullptr;
				property.target = static_cast<std::uint32_t>(propertyId);
				property.op = RE::BGSMod::Property::OP::kSet;
				property.type = RE::BGSMod::Property::TYPE::kInt;
				property.step = 0;
				std::string type = toLowerCase(parts[1]);
				std::string operation = toLowerCase(parts[2]);
				if (operation != "set" && operation != "add" && operation != "mul" && operation != "mult" && operation != "multiply" && operation != "rem" && operation != "remove") {
					logger::warn(FMT_STRING("OMOD {:08X}: unknown property operation '{}'"), curobj->formID, parts[2]);
					continue;
				}
				const bool isFormType = type == "form" || type == "form,int" || type == "formint" || type == "formid,int" || type == "formidint";
				const bool isEnumType = type == "enum";
				if ((isFormType && (operation == "mul" || operation == "mult" || operation == "multiply")) ||
					(isEnumType && operation != "set")) {
					logger::warn(FMT_STRING("OMOD {:08X}: operation '{}' is not valid for type '{}'"), curobj->formID, parts[2], parts[1]);
					continue;
				}
				property.op = operation == "add" ? RE::BGSMod::Property::OP::kAdd :
					operation == "mul" || operation == "mult" || operation == "multiply" || operation == "rem" || operation == "remove" ? RE::BGSMod::Property::OP::kMul :
					RE::BGSMod::Property::OP::kSet;

				try {
					if (type == "float") {
						property.type = RE::BGSMod::Property::TYPE::kFloat;
						property.data.mm.min.f = operation == "rem" || operation == "remove" ? 0.0f : std::stof(parts[3]);
						property.data.mm.max.f = operation == "rem" || operation == "remove" ? 0.0f : parts.size() >= 5 ? std::stof(parts[4]) : 0.0f;
					} else if (type == "int" || type == "integer") {
						property.type = RE::BGSMod::Property::TYPE::kInt;
						property.data.mm.min.i = operation == "rem" || operation == "remove" ? 0 : std::stoi(parts[3]);
						property.data.mm.max.i = operation == "rem" || operation == "remove" ? 0 : parts.size() >= 5 ? std::stoi(parts[4]) : 0;
					} else if (type == "bool" || type == "enum") {
						property.type = type == "bool" ? RE::BGSMod::Property::TYPE::kBool : RE::BGSMod::Property::TYPE::kEnum;
						property.data.mm.min.i = operation == "rem" || operation == "remove" ? 0 : std::stoi(parts[3]);
						property.data.mm.max.i = 0;
					} else if (type == "form") {
						property.type = RE::BGSMod::Property::TYPE::kForm;
						auto* form = GetFormFromIdentifier(parts[3]);
						if (!form) {
							throw std::invalid_argument("form not found");
						}
						property.data.form = form;
					} else if ((type == "form,int" || type == "formint" || type == "formid,int" || type == "formidint") && parts.size() >= 5) {
						// Bethesda's runtime representation for these properties is a
						// form pointer in this native structure. The second integer is
						// an xEdit/serialized representation detail; writing it into
						// the DATATYPE union would overwrite the 64-bit form pointer.
						// Accept the documented form,int syntax, but keep the native
						// value identical to the safe form path.
						property.type = RE::BGSMod::Property::TYPE::kForm;
						auto* form = GetFormFromIdentifier(parts[3]);
						if (!form) {
							throw std::invalid_argument("form not found");
						}
						property.data.form = form;
					} else if (type == "pair" && parts.size() >= 5) {
						property.type = RE::BGSMod::Property::TYPE::kPair;
						auto* form = GetFormFromIdentifier(parts[3]);
						if (!form) {
							throw std::invalid_argument("pair form not found");
						}
						property.data.fv.formID = form->formID;
						property.data.fv.value = std::stof(parts[4]);
					} else {
						throw std::invalid_argument("unsupported property type or missing value");
					}
				} catch (const std::exception& e) {
					logger::warn(FMT_STRING("OMOD {:08X}: invalid property '{}': {}"), curobj->formID, definition, e.what());
					continue;
				}

				++rebuiltCount;
				changed = true;
				++addedCount;
			}

			if (changed) {
				data.attachments = rebuiltAttachments.get();
				data.attachmentCount = attachmentCount;
				data.propertyMods = rebuilt.get();
				data.propertyModCount = rebuiltCount;
				if (!EngineAdapters::ReplaceOmodData(curobj, data)) {
					logger::critical(FMT_STRING("OMOD {:08X}: property addition rejected; original buffer preserved"), curobj->formID);
					return;
				}
				RE::BGSMod::Attachment::Mod::Data verifyData;
				if (!ReadData(curobj, verifyData)) {
					return;
				}
				logger::debug(FMT_STRING("OMOD {:08X}: native SetData verification reports {} property definition(s)"), curobj->formID, verifyData.propertyModCount);
				logger::debug(FMT_STRING("OMOD {:08X}: added {} property definition(s)"), curobj->formID, addedCount);
				if (verifyData.propertyModCount > 0 && !verifyData.propertyMods) {
					logger::warn(FMT_STRING("OMOD {:08X}: native SetData returned a property count without property data"), curobj->formID);
				} else for (std::uint32_t i = 0; i < verifyData.propertyModCount; ++i) {
					const auto& mod = verifyData.propertyMods[i];
					if (mod.target != 61 && mod.target != 75) {
						continue;
					}
					std::uint64_t rawData = 0;
					std::memcpy(&rawData, std::addressof(mod.data), sizeof(rawData));
					logger::debug(FMT_STRING("OMOD {:08X}: property target {} type {} op {} rawData {:016X} formPtr {:016X} int {} step {}"),
						curobj->formID, static_cast<std::uint32_t>(mod.target), static_cast<std::uint32_t>(mod.type),
						static_cast<std::uint32_t>(mod.op), rawData,
						reinterpret_cast<std::uintptr_t>(mod.data.form), mod.data.mm.max.i, mod.step);
				}
			}
		}

		if (!line.fullName.empty() && line.fullName != "none") {
			logger::debug(FMT_STRING("omod formid: {:08X} {} changed fullname to {}"), curobj->formID, curobj->fullName, line.fullName);
			curobj->fullName = line.fullName;
		}
	
		if (!line.functionTypeProperties.empty() ) {
			for (uint32_t i = 0; i < line.functionTypeProperties.size(); i++) {
				if (i >= line.functionTypeValues.size()) {
					logger::warn(FMT_STRING("OMOD {:08X}: missing function type for property '{}'"), curobj->formID, line.functionTypeProperties[i]);
					continue;
				}
				std::string lowercaseFunctionType = line.functionTypeProperties[i];
				std::transform(lowercaseFunctionType.begin(), lowercaseFunctionType.end(), lowercaseFunctionType.begin(), [](unsigned char c) { return std::tolower(c); });
				std::string lowercaseFunctionTypeValues = line.functionTypeValues[i];
				std::transform(lowercaseFunctionTypeValues.begin(), lowercaseFunctionTypeValues.end(), lowercaseFunctionTypeValues.begin(), [](unsigned char c) { return std::tolower(c); });
				RE::BGSMod::Attachment::Mod::Data data;
				if (!ReadData(curobj, data)) {
					continue;
				}
				if (data.propertyModCount > 0 && !data.propertyMods) {
					logger::warn(FMT_STRING("OMOD {:08X}: property count is non-zero but property data is missing"), curobj->formID);
					continue;
				}
				int propertyId = getPropertyFromString(lowercaseFunctionType, targetFormType);

				for (uint32_t j = 0; j < data.propertyModCount; j++) {
					auto& mod = data.propertyMods[j];
					if (propertyId == static_cast<int>(mod.target)) {
						//logger::debug(FMT_STRING("omod {:08X} {} removed property {}"), curobj->formID, curobj->fullName, line.propertiesToRemove[i]);
						if (lowercaseFunctionTypeValues == "add") {
							mod.op = RE::BGSMod::Property::OP::kAdd;
							logger::debug(FMT_STRING("omod {:08X} {} changed function type of {} to ADD "), curobj->formID, curobj->fullName, lowercaseFunctionType);
						} else if (lowercaseFunctionTypeValues == "set") {
							mod.op = RE::BGSMod::Property::OP::kSet;
							logger::debug(FMT_STRING("omod {:08X} {} changed function type of {} to SET "), curobj->formID, curobj->fullName, lowercaseFunctionType);
						} else if (lowercaseFunctionTypeValues == "multandadd") {
							mod.op = RE::BGSMod::Property::OP::kMul;
							logger::debug(FMT_STRING("omod {:08X} {} changed function type of {} to MULT+ADD"), curobj->formID, curobj->fullName, lowercaseFunctionType);
						} else if ( lowercaseFunctionTypeValues == "rem") {
							mod.op = RE::BGSMod::Property::OP::kRem;
							logger::debug(FMT_STRING("omod {:08X} {} changed function type of {} to REM "), curobj->formID, curobj->fullName, lowercaseFunctionType);
						}
					}
				}



				//logger::debug(FMT_STRING("weapon formid: {:08X} {} changed weight {}"), curobj->formID);
			}
		}

		if (!line.attachParentSlotKeywordsToAdd.empty()) {
			//logger::debug("-requiered for whatever reason- 1");
			for (size_t i = 0; i < line.attachParentSlotKeywordsToAdd.size(); i++) {
				//logger::debug("-requiered for whatever reason- 2");
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.attachParentSlotKeywordsToAdd[i];
				currentform = GetFormFromIdentifier(string_form);
				//logger::debug("-requiered for whatever reason- {}", string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					//logger::debug("-requiered for whatever reason- 3");
					if (VRCompat::HasKeyword2(curobj->attachParents, (RE::BGSKeyword*)currentform)) {
					} else {
						//logger::debug("-requiered for whatever reason- 4");
						VRCompat::AddKeywordAttachPoint(curobj->attachParents, (RE::BGSKeyword*)currentform);
						logger::debug(FMT_STRING("omod formid: {:08X} {} added attach parent slot keyword {:08X} {} "), curobj->formID, curobj->fullName, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}
		}

		if (!line.attachParentSlotKeywordsToRemove.empty()) {
			for (size_t i = 0; i < line.attachParentSlotKeywordsToRemove.size(); i++) {
				RE::TESForm* currentform = nullptr;
				std::string string_form = line.attachParentSlotKeywordsToRemove[i];
				currentform = GetFormFromIdentifier(string_form);
				if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
					if (VRCompat::HasKeyword2(curobj->attachParents, (RE::BGSKeyword*)currentform)) {
						VRCompat::RemoveKeyword(curobj->attachParents, (RE::BGSKeyword*)currentform);

						logger::debug(FMT_STRING("omod formid: {:08X} removed attach parent slot keyword {:08X} {} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID, ((RE::BGSKeyword*)currentform)->formEditorID);
					}
				}
			}
		}

		if (!line.attachPoint.empty() && line.attachPoint != "none") {
			RE::TESForm* currentform = nullptr;
			std::string string_form = line.attachPoint;
			currentform = GetFormFromIdentifier(string_form);
			if (currentform && currentform->formType == RE::ENUM_FORM_ID::kKYWD) {
				curobj->attachPoint.keywordIndex = BGSKeywordGetIndexForTypedKeyword( ((RE::BGSKeyword*)currentform), RE::KeywordType::kAttachPoint );
				logger::debug(FMT_STRING("omod formid: {:08X} changed attachPoint to {:08X} "), curobj->formID, ((RE::BGSKeyword*)currentform)->formID);
			} else if (line.attachPoint == "null") {
				curobj->attachPoint.keywordIndex = 0;
				logger::debug(FMT_STRING("omod formid: {:08X} changed attachPoint to null (none) "), curobj->formID);
			}
		}

		return;
	}

}
