# RobCo Patcher VR

![RobCo Patcher VR](assets/robco.png)

RobCo Patcher VR lets mod authors and users modify Fallout 4 VR game records through INI rules, without creating an ESP or ESM patch. It is a native F4SEVR plugin for **Fallout 4 VR 1.2.72.0**.

**Current release: 1.2.1.** Download the installable ZIP from [GitHub Releases](https://github.com/brunocatani/Robco-Patcher-VR/releases/tag/v1.2.1). See [CHANGELOG.md](CHANGELOG.md) for the full version history.

## What's new in 1.2.1

Runtime settings now live exclusively at `Documents\My Games\Fallout4VR\Mods_Config\RobCo_Patcher\RobCo_Patcher.ini`. Move your existing INI there when upgrading; its values stay the same. The old settings locations are no longer read, and a missing INI is reported in the plugin log. See [Configuration](#configuration) below.

## What's new in 1.2.0

- Reworked VR patching for ingestible effects, object modifications (OMODs), leveled lists, projectiles, and explosions.
- Checked numeric updates for weapons, armor, and ammunition, plus fixes for crafting records, form lists, containers, and outfits.
- Improved dry-run previews, error isolation, and preservation of existing data when a replacement fails.
- Daytripper ESL provider detection accepts both `Daytripper4.dll` and `falloutvresl.dll`, with validation before use.

The release also includes the aim-model patching and expanded weapon, armor, and ammo fields introduced in 1.1.0.

## Requirements

- Fallout 4 VR, executable version **1.2.72.0**.
- **F4SEVR 0.6.21**.
- **VR Address Library for F4SEVR**, with the address database for Fallout 4 VR 1.2.72.0.
- For rules targeting ESL or ESL-flagged plugins: a compatible **Daytripper / Fallout VR ESL Support** provider exposing `GetCompiledFileCollectionExtern`.

This DLL is for Fallout 4 VR. Desktop Fallout 4 and its next-generation executable are unsupported.

## Installation and updates

1. Install the requirements and launch the game through F4SEVR.
2. Install `RobCo_Patcher_VR-v1.2.1.zip` with your mod manager. If prompted to select the game-data directory, choose the archive's `Data` folder. For manual installation, merge that folder into the game's `Data` folder.
3. Confirm the installed plugin path is `Data\F4SE\Plugins\RobCo_Patcher.dll`.
4. Keep your patch-rule mods installed. Their rules belong under `Data\F4SE\Plugins\RobCo_Patcher\`, in the appropriate category folders.
5. Configure the category switches described below, then restart the game to apply the rules.

When upgrading, replace the old DLL, move your settings INI as described below, and preserve your patch-rule files. The release archive contains the DLL, README, changelog, license, and project image; it does not supply or overwrite runtime INIs or patch rules. The GitHub ZIP is also the package intended for Nexus distribution.

## Configuration

The runtime settings file is:

```text
Documents\My Games\Fallout4VR\Mods_Config\RobCo_Patcher\RobCo_Patcher.ini
```

This is the only runtime settings location. When upgrading from 1.2.0 or earlier, create the `Mods_Config\RobCo_Patcher` folder and move your existing `RobCo_Patcher.ini` there, preserving its contents. Previous installations may have stored the file directly under `Documents\My Games\Fallout4VR` or under `Data\F4SE\Plugins`. Those locations are no longer read. If the new location already contains your settings, keep that file; settings are not merged or migrated automatically.

For a new installation, create the folder and settings file, then enable the categories your patch rules use under **`[Patcher]`**. Missing switches default to `0` (disabled), and the plugin does not create this file automatically. A missing settings file is reported in the plugin log. This example enables all supported categories:

```ini
[Patcher]
iEnableAmmoPatching=1
iEnableWeaponPatching=1
iEnableAimModelPatching=1
iEnableRacePatching=1
iEnableNPCPatching=1
iEnableArmorPatching=1
iEnableFormlistPatching=1
iEnableConstructibleObjectPatching=1
iEnableLeveledListPatching=1
iEnableObjectModificationPatching=1
iEnableMiscPatching=1
iEnableIngestiblePatching=1
iEnableProjectilePatching=1
iEnableExplosionPatching=1
iEnableOutfitPatching=1
```

Rule folders beneath `Data\F4SE\Plugins\RobCo_Patcher\` are:

| Record category | Folder |
| --- | --- |
| Ammunition | `ammo` |
| Weapons | `weapon` |
| Aim models | `aimModel` |
| Races | `race` |
| NPCs | `npc` |
| Armor | `armor` |
| Form lists | `formList` |
| Constructible objects | `constructibleObject` |
| Leveled lists | `leveledList` |
| Object modifications | `objectModification` |
| Miscellaneous objects | `misc` |
| Ingestibles | `ingestible` |
| Projectiles | `projectile` |
| Explosions | `explosion` |
| Outfits | `outfit` |

## Diagnostics

Add these optional sections to the same runtime INI when investigating a patch:

```ini
[Log]
iEnablelog=0

[Diagnostics]
iEnableDiagnostics=0
iWritePatchReport=0
iEnableDryRun=0
iStrictConfigValidation=0
iLogMatchedRecords=0
iReportInstanceData=0
```

Set `iEnableDiagnostics=1`, `iWritePatchReport=1`, and `iEnableDryRun=1` to preview enabled patch rules without applying their record changes. Restore `iEnableDryRun=0` and restart the game when you want the changes applied. Set `iEnablelog=1` for debug logging.

The plugin log is `Documents\My Games\Fallout4VR\F4SE\RobCo_Patcher.log`. When enabled, the patch report is written alongside it as `RobCo_Patcher.report.txt`.

## Compatibility and known limitations

- ESL resolution requires a loaded, compatible Daytripper provider. The patcher accepts either supported DLL name, but refuses two distinct providers loaded together. Missing exports or invalid plugin collections disable light-plugin resolution and produce a log entry. Detection of both filenames does not establish compatibility with every provider version or mod list.
- Reference InstanceData refresh (`iEnableREFRCreateNewInstanceData` under `[Features]`) is disabled because this VR implementation has no verified transactional replacement API. Requests are logged and skipped to preserve existing owned extra data.
- Patch behavior depends on the supplied rules and loaded records. Include the relevant rules and the current plugin log when reporting a problem.

## Support and original mod

This is a VR port of RobCo Patcher by **Zzyxzz**. Report VR-port issues on the VR mod's forum or the [VR repository issue tracker](https://github.com/brunocatani/Robco-Patcher-VR/issues).

Please do not report VR-port bugs on the original RobCo Patcher page or ask its original author to troubleshoot this port.

## Credits

Please send any and all credits, donations, and kudos to Zzyxzz.

## License

This project is licensed under the MIT License. See [LICENSE](LICENSE).
