#pragma once

#include <cstdint>
#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace PATCH
{
	struct DiagnosticsConfig
	{
		bool enableDiagnostics{ false };
		bool writePatchReport{ false };
		bool dryRun{ false };
		bool strictConfigValidation{ false };
		bool logMatchedRecords{ false };
		bool reportInstanceData{ false };
	};

	struct KeyValue
	{
		std::string key;
		std::string value;
	};

	struct CategoryStats
	{
		std::uint32_t filesRead{ 0 };
		std::uint32_t rulesParsed{ 0 };
		std::uint32_t invalidRules{ 0 };
		std::uint32_t matchedRecords{ 0 };
		std::uint32_t appliedMutations{ 0 };
		std::uint32_t wouldApplyMutations{ 0 };
		std::uint32_t skippedMutations{ 0 };
		std::uint32_t unresolvedForms{ 0 };
		std::uint32_t wrongTypes{ 0 };
		std::uint32_t warnings{ 0 };
	};

	struct AliasEntry
	{
		std::string name;
		std::string target;
		std::string sourceFile;
		std::uint32_t line{ 0 };
	};

	struct ActiveRule
	{
		std::string category;
		std::string sourceFile;
		std::uint32_t line{ 0 };
		std::string text;
	};

	std::string Trim(const std::string& value);
	std::string ToLower(std::string value);
	std::vector<std::string> SplitList(const std::string& value, char delimiter = ',');
	std::vector<KeyValue> ParseKeyValues(const std::string& line);
	std::optional<std::string> GetValue(const std::vector<KeyValue>& values, const std::string& key);
	bool TryParseInt(const std::string& value, int& result);
	bool TryParseUInt(const std::string& value, std::uint32_t& result);
	bool TryParseFloat(const std::string& value, float& result);

	class AliasRegistry
	{
	public:
		bool Add(const std::string& name, const std::string& target, const std::string& sourceFile, std::uint32_t line);
		std::optional<std::string> Resolve(const std::string& value) const;
		std::vector<AliasEntry> Entries() const;
		void Clear();

	private:
		std::unordered_map<std::string, AliasEntry> aliases_;
	};

	class ReportState
	{
	public:
		void Configure(const DiagnosticsConfig& config);
		void BeginLoad();
		void EndLoad();
		void SetActiveRule(const ActiveRule& rule);
		void ClearActiveRule();

		[[nodiscard]] bool IsDiagnosticsEnabled() const;
		[[nodiscard]] bool ShouldWriteReport() const;
		[[nodiscard]] bool IsDryRun() const;
		[[nodiscard]] bool IsStrict() const;
		[[nodiscard]] bool ShouldLogMatchedRecords() const;
		[[nodiscard]] bool ShouldReportInstanceData() const;
		[[nodiscard]] const DiagnosticsConfig& Config() const;
		[[nodiscard]] ActiveRule Active() const;
		[[nodiscard]] CategoryStats GetStats(const std::string& category) const;

		void RecordFile(const std::string& category, const std::string& file);
		void RecordRule(const std::string& category);
		void RecordInvalidRule(const std::string& category, const std::string& message);
		void RecordMatch(const std::string& category, const std::string& target = {});
		void RecordMutation(
			const std::string& category,
			const std::string& target,
			const std::string& field,
			const std::string& oldValue,
			const std::string& newValue,
			bool dryRun);
		void RecordSkippedMutation(const std::string& category, const std::string& target, const std::string& reason);
		void RecordUnresolvedForm(const std::string& category, const std::string& identifier, const std::string& reason);
		void RecordWrongType(const std::string& category, const std::string& identifier, const std::string& expectedType);
		void RecordWarning(const std::string& category, const std::string& message);
		bool WriteReport(const std::filesystem::path& path) const;

	private:
		CategoryStats& MutableStats(const std::string& category);
		std::string FormatSource() const;

		DiagnosticsConfig config_;
		ActiveRule active_;
		bool loadStarted_{ false };
		std::unordered_map<std::string, CategoryStats> stats_;
		std::vector<std::string> files_;
		std::vector<std::string> issues_;
		std::vector<std::string> mutations_;
	};

	ReportState& Report();
	AliasRegistry& Aliases();

	void ConfigureDiagnostics(const DiagnosticsConfig& config);
	void BeginLoad();
	void EndLoad();
	void SetActiveRule(const std::string& category, const std::string& sourceFile, std::uint32_t line, const std::string& text);
	void ClearActiveRule();
	bool IsDryRun();
	bool IsStrict();
	bool IsDiagnosticsEnabled();
	bool ShouldReportInstanceData();
	bool WriteReport(const std::filesystem::path& path);
	std::string ActiveCategoryOr(const std::string& fallback);
	std::optional<std::string> ResolveAlias(const std::string& value);

	void RecordFile(const std::string& category, const std::string& file);
	void RecordRule(const std::string& category);
	void RecordInvalidRule(const std::string& category, const std::string& message);
	void RecordMatch(const std::string& category, const std::string& target = {});
	void RecordMutation(
		const std::string& category,
		const std::string& target,
		const std::string& field,
		const std::string& oldValue,
		const std::string& newValue);
	void RecordSkippedMutation(const std::string& category, const std::string& target, const std::string& reason);
	void RecordUnresolvedForm(const std::string& category, const std::string& identifier, const std::string& reason);
	void RecordWrongType(const std::string& category, const std::string& identifier, const std::string& expectedType);
	void RecordWarning(const std::string& category, const std::string& message);

	template <class T>
	bool TryResolveTypedForm(const std::string& identifier, T* form, const std::string& expectedType)
	{
		const auto category = ActiveCategoryOr("forms");
		if (!form) {
			RecordUnresolvedForm(category, identifier, "form lookup returned null");
			return false;
		}
		if (!form->template As<T>()) {
			RecordWrongType(category, identifier, expectedType);
			return false;
		}
		return true;
	}
}
