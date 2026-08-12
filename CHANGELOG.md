# Changelog

## Unreleased - Major Update

- Ingestibles can now safely add, remove, clear, and modify magic effects.
- OMOD property additions, changes, and removals now function reliably in VR.
- Leveled-list additions, removals, replacements, and bulk edits are now reliable.
- Projectiles and explosions can now be patched across their full supported property sets.
- Weapon, armor, and ammo values can be set, added to, or multiplied without overflow corruption.
- Constructible-object categories, workbenches, form lists, containers, and outfits patch correctly.
- Dry-run mode now previews all intended patches without changing game records.
- A malformed rule or failed category no longer prevents the remaining patches from running.
- Failed patches preserve the original game data instead of leaving partially modified records.

## 1.1.0 - 2026-08-04

- Added aim-model patching. Enable it with `iEnableAimModelPatching=1` under
  `[Settings]`; rules are read from
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
