#include "PatchDiagnostics.h"

#include <algorithm>
#include <charconv>
#include <fstream>
#include <sstream>

namespace PATCH
{
	namespace
	{
		ReportState g_report;
		AliasRegistry g_aliases;

		std::string CleanCategory(const std::string& category)
		{
			auto trimmed = Trim(category);
			return trimmed.empty() ? "general" : ToLower(trimmed);
		}
	}

	std::string Trim(const std::string& value)
	{
		const auto begin = std::find_if_not(value.begin(), value.end(), [](unsigned char ch) {
			return std::isspace(ch) != 0;
		});
		const auto end = std::find_if_not(value.rbegin(), value.rend(), [](unsigned char ch) {
			return std::isspace(ch) != 0;
		}).base();

		if (begin >= end) {
			return {};
		}
		return std::string(begin, end);
	}

	std::string ToLower(std::string value)
	{
		std::transform(value.begin(), value.end(), value.begin(), [](unsigned char ch) {
			return static_cast<char>(std::tolower(ch));
		});
		return value;
	}

	std::vector<std::string> SplitList(const std::string& value, char delimiter)
	{
		std::vector<std::string> result;
		std::stringstream stream(value);
		std::string item;

		while (std::getline(stream, item, delimiter)) {
			auto trimmed = Trim(item);
			if (!trimmed.empty()) {
				result.push_back(trimmed);
			}
		}

		return result;
	}

	std::vector<KeyValue> ParseKeyValues(const std::string& line)
	{
		std::vector<KeyValue> result;
		for (const auto& token : SplitList(line, ':')) {
			const auto delimiter = token.find('=');
			if (delimiter == std::string::npos) {
				result.push_back({ Trim(token), {} });
				continue;
			}

			result.push_back({ Trim(token.substr(0, delimiter)), Trim(token.substr(delimiter + 1)) });
		}
		return result;
	}

	std::optional<std::string> GetValue(const std::vector<KeyValue>& values, const std::string& key)
	{
		const auto wanted = ToLower(key);
		for (const auto& value : values) {
			if (ToLower(value.key) == wanted) {
				return value.value;
			}
		}
		return std::nullopt;
	}

	bool TryParseInt(const std::string& value, int& result)
	{
		const auto trimmed = Trim(value);
		if (trimmed.empty()) {
			return false;
		}

		const auto* begin = trimmed.data();
		const auto* end = begin + trimmed.size();
		const auto parsed = std::from_chars(begin, end, result);
		return parsed.ec == std::errc{} && parsed.ptr == end;
	}

	bool TryParseUInt(const std::string& value, std::uint32_t& result)
	{
		const auto trimmed = Trim(value);
		if (trimmed.empty() || trimmed[0] == '-') {
			return false;
		}

		const auto* begin = trimmed.data();
		const auto* end = begin + trimmed.size();
		const auto parsed = std::from_chars(begin, end, result);
		return parsed.ec == std::errc{} && parsed.ptr == end;
	}

	bool TryParseFloat(const std::string& value, float& result)
	{
		const auto trimmed = Trim(value);
		if (trimmed.empty()) {
			return false;
		}

		char* parseEnd = nullptr;
		result = std::strtof(trimmed.c_str(), &parseEnd);
		return parseEnd && *parseEnd == '\0';
	}

	bool AliasRegistry::Add(const std::string& name, const std::string& target, const std::string& sourceFile, std::uint32_t line)
	{
		auto cleanName = Trim(name);
		if (!cleanName.empty() && cleanName.front() == '@') {
			cleanName.erase(cleanName.begin());
		}

		const auto cleanTarget = Trim(target);
		if (cleanName.empty() || cleanTarget.empty()) {
			return false;
		}

		aliases_[ToLower(cleanName)] = AliasEntry{ cleanName, cleanTarget, sourceFile, line };
		return true;
	}

	std::optional<std::string> AliasRegistry::Resolve(const std::string& value) const
	{
		const auto cleanValue = Trim(value);
		if (cleanValue.empty()) {
			return std::nullopt;
		}
		if (cleanValue.front() != '@') {
			return cleanValue;
		}

		auto name = cleanValue.substr(1);
		const auto found = aliases_.find(ToLower(name));
		if (found == aliases_.end()) {
			return std::nullopt;
		}
		return found->second.target;
	}

	std::vector<AliasEntry> AliasRegistry::Entries() const
	{
		std::vector<AliasEntry> entries;
		entries.reserve(aliases_.size());
		for (const auto& [_, entry] : aliases_) {
			entries.push_back(entry);
		}
		std::sort(entries.begin(), entries.end(), [](const AliasEntry& lhs, const AliasEntry& rhs) {
			return ToLower(lhs.name) < ToLower(rhs.name);
		});
		return entries;
	}

	void AliasRegistry::Clear()
	{
		aliases_.clear();
	}

	void ReportState::Configure(const DiagnosticsConfig& config)
	{
		config_ = config;
	}

	void ReportState::BeginLoad()
	{
		loadStarted_ = true;
		stats_.clear();
		files_.clear();
		issues_.clear();
		mutations_.clear();
		active_ = {};
	}

	void ReportState::EndLoad()
	{
		loadStarted_ = false;
		active_ = {};
	}

	void ReportState::SetActiveRule(const ActiveRule& rule)
	{
		active_ = rule;
		active_.category = CleanCategory(active_.category);
	}

	void ReportState::ClearActiveRule()
	{
		active_ = {};
	}

	bool ReportState::IsDiagnosticsEnabled() const
	{
		return config_.enableDiagnostics || config_.writePatchReport || config_.dryRun ||
			config_.strictConfigValidation || config_.logMatchedRecords || config_.reportInstanceData;
	}

	bool ReportState::ShouldWriteReport() const
	{
		return config_.writePatchReport || config_.dryRun || config_.strictConfigValidation;
	}

	bool ReportState::IsDryRun() const
	{
		return config_.dryRun;
	}

	bool ReportState::IsStrict() const
	{
		return config_.strictConfigValidation;
	}

	bool ReportState::ShouldLogMatchedRecords() const
	{
		return config_.logMatchedRecords;
	}

	bool ReportState::ShouldReportInstanceData() const
	{
		return config_.reportInstanceData;
	}

	const DiagnosticsConfig& ReportState::Config() const
	{
		return config_;
	}

	ActiveRule ReportState::Active() const
	{
		return active_;
	}

	CategoryStats ReportState::GetStats(const std::string& category) const
	{
		const auto found = stats_.find(CleanCategory(category));
		return found == stats_.end() ? CategoryStats{} : found->second;
	}

	void ReportState::RecordFile(const std::string& category, const std::string& file)
	{
		++MutableStats(category).filesRead;
		files_.push_back(CleanCategory(category) + ": " + file);
	}

	void ReportState::RecordRule(const std::string& category)
	{
		++MutableStats(category).rulesParsed;
	}

	void ReportState::RecordInvalidRule(const std::string& category, const std::string& message)
	{
		++MutableStats(category).invalidRules;
		issues_.push_back("invalid " + CleanCategory(category) + " rule " + FormatSource() + ": " + message);
	}

	void ReportState::RecordMatch(const std::string& category, const std::string& target)
	{
		++MutableStats(category).matchedRecords;
		if (config_.logMatchedRecords && !target.empty()) {
			issues_.push_back("matched " + CleanCategory(category) + " " + target + " " + FormatSource());
		}
	}

	void ReportState::RecordMutation(
		const std::string& category,
		const std::string& target,
		const std::string& field,
		const std::string& oldValue,
		const std::string& newValue,
		bool dryRun)
	{
		auto& stats = MutableStats(category);
		if (dryRun) {
			++stats.wouldApplyMutations;
		} else {
			++stats.appliedMutations;
		}

		if (IsDiagnosticsEnabled()) {
			mutations_.push_back(
				std::string(dryRun ? "wouldApply " : "applied ") + CleanCategory(category) + " " + target +
				" " + field + ": " + oldValue + " -> " + newValue + " " + FormatSource());
		}
	}

	void ReportState::RecordSkippedMutation(const std::string& category, const std::string& target, const std::string& reason)
	{
		++MutableStats(category).skippedMutations;
		issues_.push_back("skipped " + CleanCategory(category) + " " + target + ": " + reason + " " + FormatSource());
	}

	void ReportState::RecordUnresolvedForm(const std::string& category, const std::string& identifier, const std::string& reason)
	{
		++MutableStats(category).unresolvedForms;
		issues_.push_back("unresolved form " + identifier + " in " + CleanCategory(category) + ": " + reason + " " + FormatSource());
	}

	void ReportState::RecordWrongType(const std::string& category, const std::string& identifier, const std::string& expectedType)
	{
		++MutableStats(category).wrongTypes;
		issues_.push_back("wrong form type " + identifier + " in " + CleanCategory(category) + ", expected " + expectedType + " " + FormatSource());
	}

	void ReportState::RecordWarning(const std::string& category, const std::string& message)
	{
		++MutableStats(category).warnings;
		issues_.push_back("warning " + CleanCategory(category) + ": " + message + " " + FormatSource());
	}

	bool ReportState::WriteReport(const std::filesystem::path& path) const
	{
		std::ofstream output(path, std::ios::trunc);
		if (!output) {
			return false;
		}

		output << "RobCo Patcher report\n";
		output << "dryRun=" << (config_.dryRun ? "1" : "0") << "\n";
		output << "strictConfigValidation=" << (config_.strictConfigValidation ? "1" : "0") << "\n\n";

		output << "[Summary]\n";
		std::vector<std::string> categories;
		categories.reserve(stats_.size());
		for (const auto& [category, _] : stats_) {
			categories.push_back(category);
		}
		std::sort(categories.begin(), categories.end());
		for (const auto& category : categories) {
			const auto stats = GetStats(category);
			output << category
				   << " files=" << stats.filesRead
				   << " rules=" << stats.rulesParsed
				   << " invalid=" << stats.invalidRules
				   << " matched=" << stats.matchedRecords
				   << " applied=" << stats.appliedMutations
				   << " wouldApply=" << stats.wouldApplyMutations
				   << " skipped=" << stats.skippedMutations
				   << " unresolved=" << stats.unresolvedForms
				   << " wrongType=" << stats.wrongTypes
				   << " warnings=" << stats.warnings
				   << "\n";
		}

		if (!files_.empty()) {
			output << "\n[Files]\n";
			for (const auto& file : files_) {
				output << file << "\n";
			}
		}

		if (!issues_.empty()) {
			output << "\n[Issues]\n";
			for (const auto& issue : issues_) {
				output << issue << "\n";
			}
		}

		if (!mutations_.empty()) {
			output << "\n[Mutations]\n";
			for (const auto& mutation : mutations_) {
				output << mutation << "\n";
			}
		}

		return true;
	}

	CategoryStats& ReportState::MutableStats(const std::string& category)
	{
		return stats_[CleanCategory(category)];
	}

	std::string ReportState::FormatSource() const
	{
		if (active_.sourceFile.empty()) {
			return {};
		}

		return "(" + active_.sourceFile + ":" + std::to_string(active_.line) + ")";
	}

	ReportState& Report()
	{
		return g_report;
	}

	AliasRegistry& Aliases()
	{
		return g_aliases;
	}

	void ConfigureDiagnostics(const DiagnosticsConfig& config)
	{
		Report().Configure(config);
	}

	void BeginLoad()
	{
		Report().BeginLoad();
	}

	void EndLoad()
	{
		Report().EndLoad();
	}

	void SetActiveRule(const std::string& category, const std::string& sourceFile, std::uint32_t line, const std::string& text)
	{
		Report().SetActiveRule({ category, sourceFile, line, text });
	}

	void ClearActiveRule()
	{
		Report().ClearActiveRule();
	}

	bool IsDryRun()
	{
		return Report().IsDryRun();
	}

	bool IsStrict()
	{
		return Report().IsStrict();
	}

	bool IsDiagnosticsEnabled()
	{
		return Report().IsDiagnosticsEnabled();
	}

	bool ShouldReportInstanceData()
	{
		return Report().ShouldReportInstanceData();
	}

	bool WriteReport(const std::filesystem::path& path)
	{
		return Report().WriteReport(path);
	}

	std::string ActiveCategoryOr(const std::string& fallback)
	{
		auto active = Report().Active();
		return active.category.empty() ? CleanCategory(fallback) : active.category;
	}

	std::optional<std::string> ResolveAlias(const std::string& value)
	{
		auto resolved = Aliases().Resolve(value);
		if (!resolved && !Trim(value).empty() && Trim(value).front() == '@') {
			RecordUnresolvedForm(ActiveCategoryOr("aliases"), value, "alias not found");
		}
		return resolved;
	}

	void RecordFile(const std::string& category, const std::string& file)
	{
		Report().RecordFile(category, file);
	}

	void RecordRule(const std::string& category)
	{
		Report().RecordRule(category);
	}

	void RecordInvalidRule(const std::string& category, const std::string& message)
	{
		Report().RecordInvalidRule(category, message);
	}

	void RecordMatch(const std::string& category, const std::string& target)
	{
		Report().RecordMatch(category, target);
	}

	void RecordMutation(
		const std::string& category,
		const std::string& target,
		const std::string& field,
		const std::string& oldValue,
		const std::string& newValue)
	{
		Report().RecordMutation(category, target, field, oldValue, newValue, IsDryRun());
	}

	void RecordSkippedMutation(const std::string& category, const std::string& target, const std::string& reason)
	{
		Report().RecordSkippedMutation(category, target, reason);
	}

	void RecordUnresolvedForm(const std::string& category, const std::string& identifier, const std::string& reason)
	{
		Report().RecordUnresolvedForm(category, identifier, reason);
	}

	void RecordWrongType(const std::string& category, const std::string& identifier, const std::string& expectedType)
	{
		Report().RecordWrongType(category, identifier, expectedType);
	}

	void RecordWarning(const std::string& category, const std::string& message)
	{
		Report().RecordWarning(category, message);
	}
}
