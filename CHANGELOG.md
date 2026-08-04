# Changelog

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
