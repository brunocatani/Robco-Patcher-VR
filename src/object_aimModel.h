#ifndef AIMMODEL_H
#define AIMMODEL_H

using namespace std;

#	include "PCH.h"
#	include "dirent.h"
#	include "gameforms.h"
#	include "utility.h"
#	include <fstream>
#	include <iostream>
#	include <regex>
#	include <string>
#	include <vector>

namespace AIMMODEL
{
	struct patch_instruction
	{
		std::vector<std::string> object;

		std::string aimModelMinConeDegrees;
		std::string aimModelMaxConeDegrees;
		std::string aimModelConeIncreasePerShot;
		std::string aimModelConeDecreasePerSec;
		std::string aimModelConeDecreaseDelayMs;
		std::string aimModelConeSneakMultiplier;
		std::string aimModelRecoilDiminishSpringForce;
		std::string aimModelRecoilDiminishSightsMult;
		std::string aimModelRecoilMaxDegPerShot;
		std::string aimModelRecoilMinDegPerShot;
		std::string aimModelRecoilHipMult;
		std::string aimModelRecoilShotsForRunaway;
		std::string aimModelRecoilArcDeg;
		std::string aimModelRecoilArcRotateDeg;
		std::string aimModelConeIronSightsMultiplier;
		std::string aimModelBaseStability;
	};

	struct patch_instruction create_patch_instruction(const std::string& line);
	void process_patch_instructions(const std::list<patch_instruction>& tokens);
	void readConfig(const std::string& folder);
	void patch(AIMMODEL::patch_instruction line, RE::BGSAimModel* curobj);
}

#endif
