#include "EngineAdapters.h"
#include "PCH.h"


#include "gameforms.h"
#include "object_aimModel.h"
#include "object_alch.h"
#include "object_ammos.h"
#include "object_armors.h"
#include "object_cobjs.h"
#include "object_explosion.h"
#include "object_formlist.h"
#include "object_leveledLists.h"
#include "object_misc.h"
#include "object_npcs.h"
#include "object_omod.h"
#include "object_outfit.h"
#include "object_projectile.h"
#include "object_races.h"
#include "object_weapons.h"
#include "utility.h"
#include "RuntimeConfig.h"

#include <spdlog/sinks/rotating_file_sink.h>

#include <array>
#include <atomic>
#include <cstdio>
#include <fstream>
#include <memory>
#include <stdexcept>
#include <string_view>

namespace
{
	constexpr std::string_view kSettingsSection = "Patcher";
	constexpr std::string_view kLogSection = "Log";
	constexpr std::string_view kFeaturesSection = "Features";
	constexpr std::string_view kDiagnosticsSection = "Diagnostics";
	constexpr std::string_view kRulesRoot = ".\\Data\\F4SE\\Plugins\\RobCo_Patcher\\";

	std::filesystem::path g_configPath;
	std::shared_ptr<spdlog::logger> g_log;
	std::atomic_bool g_patchStarted{ false };

	void ReportBoundaryFailure(const char* boundary, const char* detail) noexcept
	{
		char message[1024]{};
		std::snprintf(
			message,
			sizeof(message),
			"RobCo Patcher VR: %s failed%s%s\n",
			boundary ? boundary : "plugin boundary",
			detail ? ": " : "",
			detail ? detail : "");
		OutputDebugStringA(message);
		try {
			if (g_log) {
				g_log->critical("{}", message);
			}
		} catch (...) {
		}
	}

	bool InitializeLogger() noexcept
	{
		try {
			if (g_log) {
				return true;
			}

			auto directory = logger::log_directory();
			if (!directory) {
				return false;
			}
			constexpr std::string_view expectedPath = "Fallout4VR/F4SE";
			if (!directory->generic_string().ends_with(expectedPath)) {
				*directory = directory->parent_path() / expectedPath;
			}
			*directory /= fmt::format(FMT_STRING("{}.log"), Version::PROJECT);

			auto sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(
				directory->string(),
				5 * 1024 * 1024,
				3,
				true);
			g_log = std::make_shared<spdlog::logger>("RobCo_Patcher", std::move(sink));
			g_log->set_pattern("%Y-%m-%d %H:%M:%S.%e [%l] %v");
			g_log->set_level(spdlog::level::info);
			g_log->flush_on(spdlog::level::info);
			spdlog::set_default_logger(g_log);
			return true;
		} catch (const std::exception& error) {
			ReportBoundaryFailure("logger initialization", error.what());
			return false;
		} catch (...) {
			ReportBoundaryFailure("logger initialization", nullptr);
			return false;
		}
	}

	int ReadSetting(std::string_view section, std::string_view key, int defaultValue = 0)
	{
		return GetPrivateProfileIntW(
			std::wstring(section.begin(), section.end()).c_str(),
			std::wstring(key.begin(), key.end()).c_str(),
			defaultValue,
			g_configPath.c_str());
	}

	bool IsEnabled(std::string_view key)
	{
		return ReadSetting(kSettingsSection, key) != 0;
	}

	void ConfigureRuntime()
	{
		g_configPath = RuntimeConfig::ResolvePath();
		logger::info(FMT_STRING("Runtime INI: {}"), g_configPath.string());
		if (RuntimeConfig::EnsureExists(g_configPath)) {
			logger::info("Created missing runtime INI from compiled defaults; patch categories are enabled");
		}

		if (ReadSetting(kLogSection, "iEnablelog") != 0) {
			g_log->set_level(spdlog::level::debug);
			g_log->flush_on(spdlog::level::debug);
		}

		PATCH::DiagnosticsConfig diagnostics;
		diagnostics.enableDiagnostics = ReadSetting(kDiagnosticsSection, "iEnableDiagnostics") != 0;
		diagnostics.writePatchReport = ReadSetting(kDiagnosticsSection, "iWritePatchReport") != 0;
		diagnostics.dryRun = ReadSetting(kDiagnosticsSection, "iEnableDryRun") != 0;
		diagnostics.strictConfigValidation = ReadSetting(kDiagnosticsSection, "iStrictConfigValidation") != 0;
		diagnostics.logMatchedRecords = ReadSetting(kDiagnosticsSection, "iLogMatchedRecords") != 0;
		diagnostics.reportInstanceData = ReadSetting(kDiagnosticsSection, "iReportInstanceData") != 0;
		PATCH::ConfigureDiagnostics(diagnostics);
		if (diagnostics.dryRun) {
			logger::info("Diagnostics dry-run enabled; patch rules will not mutate records");
		}
	}

	std::string RulePath(std::string_view category)
	{
		return fmt::format(FMT_STRING("{}{}\\"), kRulesRoot, category);
	}

	void LoadAliases()
	{
		PATCH::Aliases().Clear();
		const auto aliasPath = fmt::format(FMT_STRING("{}aliases.ini"), kRulesRoot);
		std::ifstream input(aliasPath);
		if (!input) {
			return;
		}

		PATCH::RecordFile("aliases", aliasPath);
		std::string line;
		std::uint32_t lineNumber = 0;
		while (std::getline(input, line)) {
			++lineNumber;
			const auto trimmed = PATCH::Trim(line);
			if (trimmed.empty() || trimmed[0] == '#' || trimmed[0] == ';' || trimmed[0] == '[') {
				continue;
			}

			PATCH::SetActiveRule("aliases", aliasPath, lineNumber, line);
			PATCH::RecordRule("aliases");
			const auto delimiter = trimmed.find('=');
			if (delimiter == std::string::npos ||
				!PATCH::Aliases().Add(trimmed.substr(0, delimiter), trimmed.substr(delimiter + 1), aliasPath, lineNumber)) {
				PATCH::RecordInvalidRule("aliases", "expected AliasName=Plugin.esp|FormID");
			}
			PATCH::ClearActiveRule();
		}
	}

	void WriteDiagnosticsReport()
	{
		if (!PATCH::Report().ShouldWriteReport()) {
			return;
		}
		auto path = logger::log_directory();
		if (!path) {
			PATCH::RecordWarning("diagnostics", "could not resolve F4SE log directory");
			return;
		}
		constexpr std::string_view expectedPath = "Fallout4VR/F4SE";
		if (!path->generic_string().ends_with(expectedPath)) {
			*path = path->parent_path() / expectedPath;
		}
		*path /= fmt::format(FMT_STRING("{}.report.txt"), Version::PROJECT);
		if (!PATCH::WriteReport(*path)) {
			logger::warn(FMT_STRING("Could not write diagnostics report to {}"), path->string());
		}
	}

	template <class Function>
	void RunCategory(const char* category, Function&& function)
	{
		try {
			function();
		} catch (const std::exception& error) {
			PATCH::RecordInvalidRule(category, error.what());
			logger::error(FMT_STRING("Category {} failed: {}"), category, error.what());
		} catch (...) {
			PATCH::RecordInvalidRule(category, "unknown exception");
			logger::error(FMT_STRING("Category {} failed with an unknown exception"), category);
		}
	}

	void ApplyPatches()
	{
		if (g_patchStarted.exchange(true)) {
			logger::warn("Ignoring duplicate GameDataReady patch request");
			return;
		}

		PATCH::BeginLoad();
		struct EndLoadGuard
		{
			~EndLoadGuard()
			{
				PATCH::ClearActiveRule();
				PATCH::EndLoad();
			}
		} endLoadGuard;

		LoadAliases();
		if (!InitializeFormResolver()) {
			logger::error("Patch pass skipped: loaded-file resolution failed validation");
			return;
		}
		GAMEFORMS::DefineGameForms();

		if (IsEnabled("iEnableAmmoPatching")) RunCategory("ammo", [] { AMMOS::readConfig(RulePath("ammo")); });
		if (IsEnabled("iEnableWeaponPatching")) RunCategory("weapon", [] { WEAPONS::readConfig(RulePath("weapon")); });
		if (IsEnabled("iEnableAimModelPatching")) RunCategory("aimmodel", [] { AIMMODEL::readConfig(RulePath("aimModel")); });
		if (IsEnabled("iEnableRacePatching")) RunCategory("race", [] { RACES::readConfig(RulePath("race")); });
		if (IsEnabled("iEnableNPCPatching")) RunCategory("npc", [] { NPCS::readConfig(RulePath("npc")); });
		if (IsEnabled("iEnableArmorPatching")) RunCategory("armor", [] { ARMORS::readConfig(RulePath("armor")); });
		if (IsEnabled("iEnableFormlistPatching")) RunCategory("formlist", [] { FORMLIST::readConfig(RulePath("formList")); });
		if (IsEnabled("iEnableConstructibleObjectPatching")) RunCategory("constructibleobject", [] { COBJ::readConfig(RulePath("constructibleObject")); });
		if (IsEnabled("iEnableLeveledListPatching")) RunCategory("leveledlist", [] { LEVELEDLISTS::readConfig(RulePath("leveledList")); });
		if (IsEnabled("iEnableObjectModificationPatching")) RunCategory("objectmodification", [] { OMOD::readConfig(RulePath("objectModification")); });
		if (IsEnabled("iEnableMiscPatching")) RunCategory("misc", [] { MISC::readConfig(RulePath("misc")); });
		if (IsEnabled("iEnableIngestiblePatching")) RunCategory("ingestible", [] { ALCH::readConfig(RulePath("ingestible")); });
		if (IsEnabled("iEnableProjectilePatching")) RunCategory("projectile", [] { PROJECTILE::readConfig(RulePath("projectile")); });
		if (IsEnabled("iEnableExplosionPatching")) RunCategory("explosion", [] { EXPLOSION::readConfig(RulePath("explosion")); });
		if (IsEnabled("iEnableOutfitPatching")) RunCategory("outfit", [] { OUTFIT::readConfig(RulePath("outfit")); });

		if (ReadSetting(kFeaturesSection, "iEnableREFRCreateNewInstanceData") != 0) {
			PATCH::RecordWarning(
				"instancedata",
				"reference InstanceData refresh was requested but is disabled: FO4VR exposes no verified transactional replacement API");
			logger::critical("Reference InstanceData refresh skipped to preserve existing owned extra data");
		}

		WriteDiagnosticsReport();
		logger::info("RobCo patch pass completed");
	}

	void RecalculateActorStats()
	{
		if (ReadSetting(kFeaturesSection, "iEnableReCalculateStatsWithSaveLoad") == 0) {
			return;
		}

		auto* enduranceForm = GetFormFromIdentifier("Fallout4.esm|000002C4");
		auto* endurance = enduranceForm ? enduranceForm->As<RE::ActorValueInfo>() : nullptr;
		if (!endurance) {
			logger::warn("Actor stat recalculation skipped: Endurance actor value is unavailable");
			return;
		}

		if (auto* playerForm = GetFormFromIdentifier("Fallout4.esm|00000014")) {
			if (auto* player = playerForm->As<RE::Actor>()) {
				player->SetBaseActorValue(*endurance, player->GetBaseActorValue(*endurance));
			}
		}

		const auto* processLists = RE::ProcessLists::GetSingleton();
		if (!processLists) {
			return;
		}
		std::uint32_t updated = 0;
		for (const auto& handle : processLists->highActorHandles) {
			const auto actorPointer = handle.get();
			if (auto* actor = actorPointer.get(); actor && actor->race && actor->data.objectReference) {
				actor->SetBaseActorValue(*endurance, actor->GetBaseActorValue(*endurance));
				++updated;
			}
		}
		logger::debug(FMT_STRING("Recalculated {} active actors"), updated);
	}

	void F4SEAPI MessageHandler(F4SE::MessagingInterface::Message* message) noexcept
	{
		try {
			if (!message) {
				return;
			}
			switch (message->type) {
			case F4SE::MessagingInterface::kGameDataReady:
				ApplyPatches();
				break;
			case F4SE::MessagingInterface::kPostLoadGame:
				RecalculateActorStats();
				break;
			default:
				break;
			}
		} catch (const std::exception& error) {
			ReportBoundaryFailure("F4SE message handler", error.what());
		} catch (...) {
			ReportBoundaryFailure("F4SE message handler", nullptr);
		}
	}
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Query(
	const F4SE::QueryInterface* a_f4se,
	F4SE::PluginInfo* a_info) noexcept
{
	try {
		if (!a_f4se || !a_info || !InitializeLogger()) {
			return false;
		}

		a_info->infoVersion = F4SE::PluginInfo::kVersion;
		a_info->name = Version::PROJECT.data();
		a_info->version = Version::MAJOR * 10000 + Version::MINOR * 100 + Version::PATCH;

		if (a_f4se->IsEditor()) {
			logger::critical("Editor runtime is unsupported");
			return false;
		}
		if (!REL::Module::IsVR()) {
			logger::critical("Fallout 4 VR is required");
			return false;
		}

		const auto requiredRuntime = F4SE::RUNTIME_1_10_138;
		if (a_f4se->RuntimeVersion() < requiredRuntime) {
			logger::critical(
				FMT_STRING("Unsupported F4SE compatibility runtime {} (need >= {})"),
				a_f4se->RuntimeVersion().string(),
				requiredRuntime.string());
			return false;
		}

		const auto executableVersion = REL::Module::get().version();
		if (executableVersion != F4SE::RUNTIME_VR_1_2_72) {
			logger::critical(FMT_STRING("Unsupported Fallout4VR.exe version {}"), executableVersion.string());
			return false;
		}

		logger::info(
			FMT_STRING("{} v{} Query passed: executable {}, F4SE compatibility runtime {}"),
			Version::PROJECT,
			Version::NAME,
			executableVersion.string(),
			a_f4se->RuntimeVersion().string());
		return true;
	} catch (const std::exception& error) {
		ReportBoundaryFailure("F4SEPlugin_Query", error.what());
		return false;
	} catch (...) {
		ReportBoundaryFailure("F4SEPlugin_Query", nullptr);
		return false;
	}
}

extern "C" DLLEXPORT bool F4SEAPI F4SEPlugin_Load(const F4SE::LoadInterface* a_f4se) noexcept
{
	try {
		if (!a_f4se || !InitializeLogger()) {
			return false;
		}
		F4SE::Init(a_f4se, false);
		if (!EngineAdapters::IsSupportedRuntime()) {
			logger::critical("FO4VR executable identity changed between Query and Load");
			return false;
		}

		ConfigureRuntime();
		const auto* messaging = F4SE::GetMessagingInterface();
		if (!messaging || !messaging->RegisterListener(MessageHandler)) {
			logger::critical("Could not register the F4SE message listener");
			return false;
		}

		logger::info("RobCo Patcher VR loaded; awaiting GameDataReady");
		return true;
	} catch (const std::exception& error) {
		ReportBoundaryFailure("F4SEPlugin_Load", error.what());
		return false;
	} catch (...) {
		ReportBoundaryFailure("F4SEPlugin_Load", nullptr);
		return false;
	}
}
