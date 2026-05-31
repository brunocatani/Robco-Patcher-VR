#include "PatchDiagnostics.h"

#include <cassert>
#include <cmath>
#include <string>

int main()
{
	using namespace PATCH;

	assert(Trim("  Fallout4.esm|00012345 \t") == "Fallout4.esm|00012345");
	assert(ToLower("FiLtErByProjectiles") == "filterbyprojectiles");

	const auto values = SplitList("one, two,three");
	assert(values.size() == 3);
	assert(values[0] == "one");
	assert(values[1] == "two");
	assert(values[2] == "three");

	const auto keyValues = ParseKeyValues("filterByProjectiles=Fallout4.esm|001: speed = 3500 : fullName=Test Projectile");
	assert(keyValues.size() == 3);
	assert(keyValues[0].key == "filterByProjectiles");
	assert(keyValues[0].value == "Fallout4.esm|001");
	assert(keyValues[1].key == "speed");
	assert(keyValues[1].value == "3500");
	assert(keyValues[2].value == "Test Projectile");

	int parsedInt = 0;
	assert(TryParseInt(" -42 ", parsedInt));
	assert(parsedInt == -42);
	assert(!TryParseInt("42x", parsedInt));

	float parsedFloat = 0.0F;
	assert(TryParseFloat(" 12.5 ", parsedFloat));
	assert(std::fabs(parsedFloat - 12.5F) < 0.001F);
	assert(!TryParseFloat("12.5x", parsedFloat));

	AliasRegistry aliases;
	assert(aliases.Add("PipeGun", "Fallout4.esm|00024F55", "aliases.ini", 2));
	assert(aliases.Resolve("@PipeGun").value() == "Fallout4.esm|00024F55");
	assert(aliases.Resolve("Fallout4.esm|00000014").value() == "Fallout4.esm|00000014");
	assert(!aliases.Resolve("@Missing").has_value());

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
	report.RecordMutation("projectile", "Fallout4.esm|001", "speed", "1000", "3500", true);

	const auto stats = report.GetStats("projectile");
	assert(report.IsDryRun());
	assert(report.IsStrict());
	assert(stats.filesRead == 1);
	assert(stats.rulesParsed == 1);
	assert(stats.matchedRecords == 1);
	assert(stats.wouldApplyMutations == 1);
	assert(stats.appliedMutations == 0);

	return 0;
}
