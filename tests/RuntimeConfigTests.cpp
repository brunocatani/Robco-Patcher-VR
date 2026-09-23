#include "RuntimeConfig.h"
#include "LoadedFileIndex.h"
#include <Windows.h>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <iterator>

int main()
{
	const auto directory = std::filesystem::temp_directory_path() /
		(L"RobCoConfigTests-" + std::to_wstring(GetCurrentProcessId()) + L"-" + std::to_wstring(GetTickCount64()));
	const auto path = directory / L"Config-\u00e9" / L"RobCo_Patcher.ini";
	try {
		using LoadedFiles::Classify;
		using LoadedFiles::Domain;
		using LoadedFiles::FormID;
		if (FormID(0x04, 0, false, 0x123456) != 0x04123456u ||
			FormID(0xFE, 0xABC, true, 0xFED) != 0xFEABCFEDu ||
			FormID(0xFE, 0xFFF, true, 0xFFF) != 0xFEFFFFFFu ||
			FormID(0xFF, 0, false, 1) || FormID(0xFE, 0, false, 1) ||
			FormID(0xFE, 0x1000, true, 1) || FormID(0xFE, 0, true, 0x1000) ||
			FormID(0x04, 0, false, 0x1000000) ||
			Classify(0xFF, 0, true) != Domain::inactive || Classify(0x04, 0, true) != Domain::invalid) {
			throw std::runtime_error("Full/light plugin domain or FormID composition failed");
		}
		if (!RuntimeConfig::EnsureExists(path)) throw std::runtime_error("Missing INI was not created");
		const auto read = [&](const wchar_t* section, const wchar_t* key) {
			return GetPrivateProfileIntW(section, key, 99, path.c_str());
		};
		for (const auto* key : {L"iEnableAmmoPatching", L"iEnableWeaponPatching", L"iEnableObjectModificationPatching",
			L"iEnableAimModelPatching", L"iEnableArmorPatching", L"iEnableConstructibleObjectPatching",
			L"iEnableExplosionPatching", L"iEnableFormlistPatching", L"iEnableIngestiblePatching",
			L"iEnableLeveledListPatching", L"iEnableMiscPatching", L"iEnableNPCPatching",
			L"iEnableProjectilePatching", L"iEnableRacePatching"}) {
			if (read(L"Patcher", key) != 1) throw std::runtime_error("User-supplied patcher default was lost");
		}
		if (read(L"Patcher", L"iEnableOutfitPatching") != 0 || read(L"Log", L"iEnablelog") != 0 ||
			read(L"Diagnostics", L"iEnableDryRun") != 0 || read(L"Diagnostics", L"iEnableDiagnostics") != 0 ||
			read(L"Features", L"iEnableReCalculateStatsWithSaveLoad") != 1 ||
			read(L"Features", L"iEnableREFRCreateNewInstanceData") != 1) throw std::runtime_error("Feature/diagnostic defaults changed");
		const std::string custom = "; keep my formatting\r\n[Patcher]\r\niEnableAmmoPatching=0\r\n[Custom]\r\nValue=preserve\r\n";
		{ std::ofstream file(path, std::ios::binary | std::ios::trunc); file << custom; }
		if (RuntimeConfig::EnsureExists(path)) throw std::runtime_error("Existing INI was recreated");
		std::ifstream file(path, std::ios::binary);
		const std::string actual{std::istreambuf_iterator<char>(file), {}};
		file.close();
		if (actual != custom) throw std::runtime_error("Existing user INI was modified");
		std::filesystem::remove(path);
		if (!RuntimeConfig::EnsureExists(path)) throw std::runtime_error("Deleted INI was not recreated");
		bool failed = false;
		try { RuntimeConfig::EnsureExists(path / L"not-a-directory.ini"); }
		catch (const std::exception&) { failed = true; }
		if (!failed) throw std::runtime_error("Creation failure was silently accepted");
		std::filesystem::remove(path);
		std::filesystem::remove(path.parent_path());
		std::filesystem::remove(directory);
		std::cout << "First-run defaults, preservation, recreation, and full/light FormIDs passed\n";
		return 0;
	} catch (const std::exception& error) {
		std::cerr << error.what() << '\n';
		std::error_code ignored;
		std::filesystem::remove(path, ignored);
		std::filesystem::remove(path.parent_path(), ignored);
		std::filesystem::remove(directory, ignored);
		return 1;
	}
}
