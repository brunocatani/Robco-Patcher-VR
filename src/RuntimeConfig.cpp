#include "RuntimeConfig.h"

#include <Windows.h>
#include <array>
#include <string>
#include <string_view>
#include <system_error>

namespace RuntimeConfig
{
	namespace
	{
		struct Setting { std::string_view section; std::string_view key; int value; };
		// User-supplied RobCo_Patcher.ini, 2026-09-23. Unsupported/unsupplied
		// categories and diagnostics retain their previous disabled defaults.
		constexpr std::array settings{
			Setting{"Patcher", "iEnableAimModelPatching", 1},
			Setting{"Patcher", "iEnableAmmoPatching", 1},
			Setting{"Patcher", "iEnableArmorPatching", 1},
			Setting{"Patcher", "iEnableConstructibleObjectPatching", 1},
			Setting{"Patcher", "iEnableExplosionPatching", 1},
			Setting{"Patcher", "iEnableFormlistPatching", 1},
			Setting{"Patcher", "iEnableIngestiblePatching", 1},
			Setting{"Patcher", "iEnableLeveledListPatching", 1},
			Setting{"Patcher", "iEnableMiscPatching", 1},
			Setting{"Patcher", "iEnableNPCPatching", 1},
			Setting{"Patcher", "iEnableObjectModificationPatching", 1},
			Setting{"Patcher", "iEnableProjectilePatching", 1},
			Setting{"Patcher", "iEnableRacePatching", 1},
			Setting{"Patcher", "iEnableWeaponPatching", 1},
			Setting{"Patcher", "iEnableOutfitPatching", 0},
			Setting{"Log", "iEnablelog", 0},
			Setting{"Features", "iEnableReCalculateStatsWithSaveLoad", 1},
			Setting{"Features", "iEnableREFRCreateNewInstanceData", 1},
			Setting{"Diagnostics", "iEnableDiagnostics", 0},
			Setting{"Diagnostics", "iWritePatchReport", 0},
			Setting{"Diagnostics", "iEnableDryRun", 0},
			Setting{"Diagnostics", "iStrictConfigValidation", 0},
			Setting{"Diagnostics", "iLogMatchedRecords", 0},
			Setting{"Diagnostics", "iReportInstanceData", 0}
		};

		struct FileHandle
		{
			explicit FileHandle(HANDLE file) : value(file) {}
			FileHandle(const FileHandle&) = delete;
			FileHandle& operator=(const FileHandle&) = delete;
			HANDLE value;
			~FileHandle() { CloseHandle(value); }
		};

		std::string DefaultText()
		{
			std::string text;
			std::string_view section;
			for (const auto& setting : settings) {
				if (section != setting.section) {
					if (!text.empty()) text += "\r\n";
					section = setting.section;
					text += "[";
					text += section;
					text += "]\r\n";
				}
				text += setting.key;
				text += "=" + std::to_string(setting.value) + "\r\n";
			}
			return text;
		}
	}

	std::filesystem::path PathFromGameDirectory(const std::filesystem::path& gameDirectory)
	{
		return gameDirectory / "Data" / "F4SE" / "Plugins" / "RobCo_Patcher.ini";
	}

	std::filesystem::path ResolvePath()
	{
		std::array<wchar_t, 32768> executable{};
		const auto length = GetModuleFileNameW(nullptr, executable.data(), static_cast<DWORD>(executable.size()));
		if (length == 0 || length == executable.size()) {
			throw std::runtime_error("Cannot resolve the game directory for RobCo Patcher configuration");
		}
		return PathFromGameDirectory(std::filesystem::path(executable.data()).parent_path());
	}

	bool EnsureExists(const std::filesystem::path& path)
	{
		if (std::filesystem::is_regular_file(path)) return false;
		std::filesystem::create_directories(path.parent_path());
		const auto text = DefaultText();
		const auto file = CreateFileW(path.c_str(), GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
		if (file == INVALID_HANDLE_VALUE) {
			const auto error = GetLastError();
			if ((error == ERROR_FILE_EXISTS || error == ERROR_ALREADY_EXISTS) && std::filesystem::is_regular_file(path)) return false;
			throw std::system_error(static_cast<int>(error), std::system_category(), "Creating RobCo runtime INI");
		}
		DWORD written{};
		DWORD error{};
		{
			FileHandle handle{file};
			if (!WriteFile(file, text.data(), static_cast<DWORD>(text.size()), &written, nullptr) ||
				written != text.size() || !FlushFileBuffers(file)) {
				error = GetLastError();
				if (!error) error = ERROR_WRITE_FAULT;
			}
		}
		if (error) {
			// This call alone created the file. Do not leave partial defaults behind.
			if (!DeleteFileW(path.c_str())) {
				throw std::system_error(static_cast<int>(GetLastError()), std::system_category(),
					"Runtime INI write failed and partial-file cleanup failed");
			}
			throw std::system_error(static_cast<int>(error), std::system_category(), "Writing RobCo runtime INI");
		}
		return true;
	}
}
