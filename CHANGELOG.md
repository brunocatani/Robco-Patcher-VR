# Changelog

## 1.2.0 - 2026-09-11

### Patching improvements

- Reworked ingestible magic-effect additions, removals, clearing, and modification with VR engine allocation and cleanup.
- Reworked OMOD property additions, changes, and removals for VR property storage.
- Fixed leveled-list additions, removals, replacements, and bulk edits, including preservation of entry data.
- Expanded projectile and explosion patching across their supported property sets.
- Added checked numeric updates for weapon, armor, and ammo values to reject invalid or overflowing results.
- Corrected constructible-object categories and workbenches, form lists, containers, and outfits.
- Improved mutation failure handling to preserve existing data when a replacement cannot be completed.

### Diagnostics and compatibility

- Reworked dry-run handling so enabled patch rules can be previewed without applying their record changes.
- Isolated rule and category failures so an error does not abort every remaining patch category.
- Added runtime and live engine-signature checks around VR-specific engine adapters.
- Accept Daytripper's ESL provider as either `Daytripper4.dll` or `falloutvresl.dll` and validate its compiled-file collection before using it.
- Reject missing, invalid, or ambiguous ESL providers; light-plugin lookups are skipped when no usable provider is available.
- Prefer `Documents\My Games\Fallout4VR\RobCo_Patcher.ini`, with the legacy `Data\F4SE\Plugins\RobCo_Patcher.ini` used when the preferred file is absent.
- Document the `[Patcher]` category switches, diagnostics, installation, and upgrade procedure in the README.

### Known limitation

- Reference InstanceData refresh (`iEnableREFRCreateNewInstanceData`) is disabled in this VR implementation. A request for it is logged and skipped to preserve existing owned extra data.

## 1.1.0 - 2026-08-04

- Added aim-model patching. Enable it with `iEnableAimModelPatching=1` under
  `[Patcher]`; rules are read from
  `Data\F4SE\Plugins\RobCo_Patcher\aimModel\`.
- Added ammo `value` and `valueMult` patch fields.
- Added armor `damageResistMult`, `damageResistToAdd`, `healthMult`, and
  `instanceNamingRule` patch fields.
- Added weapon `attackDamageMult`, `attackDamageToAdd`, `maxRange`, `minRange`,
  `changeDamageTypesByMult`, and `instanceNamingRule` patch fields.
- An explicit `instanceNamingRule=none` clears an armor or weapon naming rule;
  omitting the field leaves the existing rule unchanged.
- Added the MIT license and RobCo Patcher VR project branding/documentation.

## 1.0.0

- Initial Fallout 4 VR release.
