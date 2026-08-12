#include "PatchDiagnostics.h"

#include <cmath>
#include <iostream>
#include <string>

namespace
{
	bool Expect(bool condition, const char* expression, int line)
	{
		if (!condition) {
			std::cerr << "Expectation failed at line " << line << ": " << expression << '\n';
		}
		return condition;
	}
}

#define EXPECT(expression)                  \
	do {                                      \
		if (!Expect((expression), #expression, __LINE__)) { \
			return 1;                              \
		}                                       \
	} while (false)

int main()
{
	using namespace PATCH;

	EXPECT(Trim("  Fallout4.esm|00012345 \t") == "Fallout4.esm|00012345");
	EXPECT(ToLower("FiLtErByProjectiles") == "filterbyprojectiles");

	const auto values = SplitList("one, two,three");
	EXPECT(values.size() == 3);
	EXPECT(values[0] == "one");
	EXPECT(values[1] == "two");
	EXPECT(values[2] == "three");

	const auto keyValues = ParseKeyValues("filterByProjectiles=Fallout4.esm|001: speed = 3500 : fullName=Test Projectile");
	EXPECT(keyValues.size() == 3);
	EXPECT(keyValues[0].key == "filterByProjectiles");
	EXPECT(keyValues[0].value == "Fallout4.esm|001");
	EXPECT(keyValues[1].key == "speed");
	EXPECT(keyValues[1].value == "3500");
	EXPECT(keyValues[2].value == "Test Projectile");

	int parsedInt = 0;
	EXPECT(TryParseInt(" -42 ", parsedInt));
	EXPECT(parsedInt == -42);
	EXPECT(!TryParseInt("42x", parsedInt));

	float parsedFloat = 0.0F;
	EXPECT(TryParseFloat(" 12.5 ", parsedFloat));
	EXPECT(std::fabs(parsedFloat - 12.5F) < 0.001F);
	EXPECT(!TryParseFloat("12.5x", parsedFloat));

	AliasRegistry aliases;
	EXPECT(aliases.Add("PipeGun", "Fallout4.esm|00024F55", "aliases.ini", 2));
	EXPECT(aliases.Resolve("@PipeGun").value() == "Fallout4.esm|00024F55");
	EXPECT(aliases.Resolve("Fallout4.esm|00000014").value() == "Fallout4.esm|00000014");
	EXPECT(!aliases.Resolve("@Missing").has_value());

	DiagnosticsConfig config;
	config.enableDiagnostics = true;
	config.writePatchReport = true;
	config.dryRun = true;
	config.strictConfigValidation = true;

	ReportState report;
	report.Configure(config);
	report.BeginLoad();
	report.RecordFile("projectile", "proj.ini");
	report.RecordRule("projectile");
	report.RecordMatch("projectile");
	report.RecordPatchCall("projectile", true);
	report.RecordMutation("projectile", "Fallout4.esm|001", "speed", "1000", "3500", true);

	const auto stats = report.GetStats("projectile");
	EXPECT(report.IsDryRun());
	EXPECT(report.IsStrict());
	EXPECT(stats.filesRead == 1);
	EXPECT(stats.rulesParsed == 1);
	EXPECT(stats.matchedRecords == 1);
	EXPECT(stats.patchCalls == 0);
	EXPECT(stats.wouldPatchRecords == 1);
	EXPECT(stats.wouldApplyMutations == 1);
	EXPECT(stats.appliedMutations == 0);

	return 0;
}
