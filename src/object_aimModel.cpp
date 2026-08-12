#include "object_aimModel.h"

namespace AIMMODEL
{
	namespace
	{
		std::string extractSingleValue(const std::string& line, const std::regex& fieldRegex)
		{
			std::smatch match;
			std::regex_search(line, match, fieldRegex);
			if (match.empty() || match[1].str().empty()) {
				return "none";
			}
			std::string value = match[1].str();
			value.erase(value.begin(), std::find_if_not(value.begin(), value.end(), ::isspace));
			value.erase(std::find_if_not(value.rbegin(), value.rend(), ::isspace).base(), value.end());
			return value;
		}
	}

	struct patch_instruction create_patch_instruction(const std::string& line)
	{
		patch_instruction l;

		// extract objects
		std::regex objects_regex("filterByAimModels\\s*=([^:]+)", regex::icase);
		std::smatch objects_match;
		std::regex_search(line, objects_match, objects_regex);
		std::vector<std::string> objects;
		if (objects_match.empty() || objects_match[1].str().empty()) {
			//empty
		} else {
			std::string objects_str = objects_match[1];
			std::regex objects_list_regex("[^,]+[ ]*[|][ ]*[a-zA-Z0-9]{1,8}", regex::icase);
			std::sregex_iterator objects_iterator(objects_str.begin(), objects_str.end(), objects_list_regex);
			std::sregex_iterator objects_end;
			while (objects_iterator != objects_end) {
				std::string tempVar = (*objects_iterator)[0].str();
				tempVar.erase(tempVar.begin(), std::find_if_not(tempVar.begin(), tempVar.end(), ::isspace));
				tempVar.erase(std::find_if_not(tempVar.rbegin(), tempVar.rend(), ::isspace).base(), tempVar.end());
				if (tempVar != "none") {
					objects.push_back(tempVar);
				}
				++objects_iterator;
			}
			l.object = objects;
		}

		l.aimModelMinConeDegrees = extractSingleValue(line, std::regex("aimModelMinConeDegrees\\s*=([^:]+)", regex::icase));
		l.aimModelMaxConeDegrees = extractSingleValue(line, std::regex("aimModelMaxConeDegrees\\s*=([^:]+)", regex::icase));
		l.aimModelConeIncreasePerShot = extractSingleValue(line, std::regex("aimModelConeIncreasePerShot\\s*=([^:]+)", regex::icase));
		l.aimModelConeDecreasePerSec = extractSingleValue(line, std::regex("aimModelConeDecreasePerSec\\s*=([^:]+)", regex::icase));
		l.aimModelConeDecreaseDelayMs = extractSingleValue(line, std::regex("aimModelConeDecreaseDelayMs\\s*=([^:]+)", regex::icase));
		l.aimModelConeSneakMultiplier = extractSingleValue(line, std::regex("aimModelConeSneakMultiplier\\s*=([^:]+)", regex::icase));
		l.aimModelRecoilDiminishSpringForce = extractSingleValue(line, std::regex("aimModelRecoilDiminishSpringForce\\s*=([^:]+)", regex::icase));
		l.aimModelRecoilDiminishSightsMult = extractSingleValue(line, std::regex("aimModelRecoilDiminishSightsMult\\s*=([^:]+)", regex::icase));
		l.aimModelRecoilMaxDegPerShot = extractSingleValue(line, std::regex("aimModelRecoilMaxDegPerShot\\s*=([^:]+)", regex::icase));
		l.aimModelRecoilMinDegPerShot = extractSingleValue(line, std::regex("aimModelRecoilMinDegPerShot\\s*=([^:]+)", regex::icase));
		l.aimModelRecoilHipMult = extractSingleValue(line, std::regex("aimModelRecoilHipMult\\s*=([^:]+)", regex::icase));
		l.aimModelRecoilShotsForRunaway = extractSingleValue(line, std::regex("aimModelRecoilShotsForRunaway\\s*=([^:]+)", regex::icase));
		l.aimModelRecoilArcDeg = extractSingleValue(line, std::regex("aimModelRecoilArcDeg\\s*=([^:]+)", regex::icase));
		l.aimModelRecoilArcRotateDeg = extractSingleValue(line, std::regex("aimModelRecoilArcRotateDeg\\s*=([^:]+)", regex::icase));
		l.aimModelConeIronSightsMultiplier = extractSingleValue(line, std::regex("aimModelConeIronSightsMultiplier\\s*=([^:]+)", regex::icase));
		l.aimModelBaseStability = extractSingleValue(line, std::regex("aimModelBaseStability\\s*=([^:]+)", regex::icase));

		logger::debug(FMT_STRING("aimmodel: {}"), l.object.size());
		return l;
	}

	void patch(AIMMODEL::patch_instruction line, RE::BGSAimModel* curobj)
	{
		if (!curobj || ShouldSkipPatch("aimmodel", curobj)) {
			return;
		}

		auto& data = curobj->aimModelData;

		if (!line.aimModelMinConeDegrees.empty() && line.aimModelMinConeDegrees != "none") {
			try {
				data.aimModelMinConeDegrees = std::stof(line.aimModelMinConeDegrees);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelMinConeDegrees {}"), curobj->formID, data.aimModelMinConeDegrees);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelMaxConeDegrees.empty() && line.aimModelMaxConeDegrees != "none") {
			try {
				data.aimModelMaxConeDegrees = std::stof(line.aimModelMaxConeDegrees);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelMaxConeDegrees {}"), curobj->formID, data.aimModelMaxConeDegrees);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelConeIncreasePerShot.empty() && line.aimModelConeIncreasePerShot != "none") {
			try {
				data.aimModelConeIncreasePerShot = std::stof(line.aimModelConeIncreasePerShot);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelConeIncreasePerShot {}"), curobj->formID, data.aimModelConeIncreasePerShot);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelConeDecreasePerSec.empty() && line.aimModelConeDecreasePerSec != "none") {
			try {
				data.aimModelConeDecreasePerSec = std::stof(line.aimModelConeDecreasePerSec);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelConeDecreasePerSec {}"), curobj->formID, data.aimModelConeDecreasePerSec);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelConeDecreaseDelayMs.empty() && line.aimModelConeDecreaseDelayMs != "none") {
			try {
				data.aimModelConeDecreaseDelayMs = static_cast<std::uint32_t>(std::stoul(line.aimModelConeDecreaseDelayMs));
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelConeDecreaseDelayMs {}"), curobj->formID, data.aimModelConeDecreaseDelayMs);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelConeSneakMultiplier.empty() && line.aimModelConeSneakMultiplier != "none") {
			try {
				data.aimModelConeSneakMultiplier = std::stof(line.aimModelConeSneakMultiplier);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelConeSneakMultiplier {}"), curobj->formID, data.aimModelConeSneakMultiplier);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelRecoilDiminishSpringForce.empty() && line.aimModelRecoilDiminishSpringForce != "none") {
			try {
				data.aimModelRecoilDiminishSpringForce = std::stof(line.aimModelRecoilDiminishSpringForce);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelRecoilDiminishSpringForce {}"), curobj->formID, data.aimModelRecoilDiminishSpringForce);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelRecoilDiminishSightsMult.empty() && line.aimModelRecoilDiminishSightsMult != "none") {
			try {
				data.aimModelRecoilDiminishSightsMult = std::stof(line.aimModelRecoilDiminishSightsMult);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelRecoilDiminishSightsMult {}"), curobj->formID, data.aimModelRecoilDiminishSightsMult);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelRecoilMaxDegPerShot.empty() && line.aimModelRecoilMaxDegPerShot != "none") {
			try {
				data.aimModelRecoilMaxDegPerShot = std::stof(line.aimModelRecoilMaxDegPerShot);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelRecoilMaxDegPerShot {}"), curobj->formID, data.aimModelRecoilMaxDegPerShot);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelRecoilMinDegPerShot.empty() && line.aimModelRecoilMinDegPerShot != "none") {
			try {
				data.aimModelRecoilMinDegPerShot = std::stof(line.aimModelRecoilMinDegPerShot);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelRecoilMinDegPerShot {}"), curobj->formID, data.aimModelRecoilMinDegPerShot);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelRecoilHipMult.empty() && line.aimModelRecoilHipMult != "none") {
			try {
				data.aimModelRecoilHipMult = std::stof(line.aimModelRecoilHipMult);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelRecoilHipMult {}"), curobj->formID, data.aimModelRecoilHipMult);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelRecoilShotsForRunaway.empty() && line.aimModelRecoilShotsForRunaway != "none") {
			try {
				data.aimModelRecoilShotsForRunaway = static_cast<std::uint32_t>(std::stoul(line.aimModelRecoilShotsForRunaway));
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelRecoilShotsForRunaway {}"), curobj->formID, data.aimModelRecoilShotsForRunaway);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelRecoilArcDeg.empty() && line.aimModelRecoilArcDeg != "none") {
			try {
				data.aimModelRecoilArcDeg = std::stof(line.aimModelRecoilArcDeg);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelRecoilArcDeg {}"), curobj->formID, data.aimModelRecoilArcDeg);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelRecoilArcRotateDeg.empty() && line.aimModelRecoilArcRotateDeg != "none") {
			try {
				data.aimModelRecoilArcRotateDeg = std::stof(line.aimModelRecoilArcRotateDeg);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelRecoilArcRotateDeg {}"), curobj->formID, data.aimModelRecoilArcRotateDeg);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelConeIronSightsMultiplier.empty() && line.aimModelConeIronSightsMultiplier != "none") {
			try {
				data.aimModelConeIronSightsMultiplier = std::stof(line.aimModelConeIronSightsMultiplier);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelConeIronSightsMultiplier {}"), curobj->formID, data.aimModelConeIronSightsMultiplier);
			} catch (const std::invalid_argument&) {}
		}
		if (!line.aimModelBaseStability.empty() && line.aimModelBaseStability != "none") {
			try {
				data.aimModelBaseStability = std::stof(line.aimModelBaseStability);
				logger::debug(FMT_STRING("aimmodel formid: {:08X} changed aimModelBaseStability {}"), curobj->formID, data.aimModelBaseStability);
			} catch (const std::invalid_argument&) {}
		}

		return;
	}

	void process_patch_instructions(const std::list<patch_instruction>& tokens)
	{
		logger::debug("processing patch instructions");
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		RE::BSTArray<RE::BGSAimModel*> AimModelArray = dataHandler->GetFormArray<RE::BGSAimModel>();

		for (const auto& line : tokens) {
			if (!line.object.empty()) {
				for (const auto& objectstring : line.object) {
					RE::TESForm* currentform = nullptr;
					std::string string_form = objectstring;
					currentform = GetFormFromIdentifier(string_form);
					if (currentform && currentform->formType == RE::ENUM_FORM_ID::kAMDL) {
						patch(line, (RE::BGSAimModel*)currentform);
					}
				}
				continue;
			}

			for (const auto& curobj : AimModelArray) {
				patch(line, curobj);
			}
		}
	}

	void readConfig(const std::string& folder)
	{
		char skipChar = '/';

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
								PATCH::RecordFile("aimmodel", fullPath);
								std::string line;
								std::ifstream infile;
								std::list<patch_instruction> tokens;
								infile.open(fullPath);
								while (std::getline(infile, line)) {
									if (line.empty() || line[0] == skipChar) {
										continue;
									}

									PATCH::RecordRule("aimmodel");
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
}
