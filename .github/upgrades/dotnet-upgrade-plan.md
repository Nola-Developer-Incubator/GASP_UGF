# .NET 8.0 Upgrade Plan

## Execution Steps

Execute steps below sequentially one by one in the order they are listed.

1. Validate that an .NET 8.0 SDK required for this upgrade is installed on the machine and if not, help to get it installed.
2. Ensure that the SDK version specified in global.json files is compatible with the .NET 8.0 upgrade.

3. Upgrade `Intermediate\Build\BuildRulesProjects\GASPModuleRules\GASPModuleRules.csproj`

## Settings

This section contains settings and data used by execution steps.

### Excluded projects

Table below contains projects that belong to the solution but are intentionally excluded from the .NET upgrade because they are native Unreal Engine C++ projects.

| Project name                                   | Description                                                       |
|:-----------------------------------------------|:------------------------------------------------------------------|
| `Intermediate\ProjectFiles\UE5.vcxproj`       | Native UE C++ project — excluded from .NET SDK-style conversion    |
| `Intermediate\ProjectFiles\LiveLinkHub.vcxproj` | Native UE C++ project — excluded from .NET SDK-style conversion    |
| `Intermediate\ProjectFiles\IoStoreOnDemandTests.vcxproj` | Native UE C++ test project — excluded from .NET SDK-style conversion |
| `Intermediate\ProjectFiles\GASP.vcxproj`      | Native UE C++ project — excluded from .NET SDK-style conversion    |
| `Intermediate\ProjectFiles\EventLoopUnitTests.vcxproj` | Native UE C++ test project — excluded from .NET SDK-style conversion |
| `Intermediate\ProjectFiles\DotNetPerforceLib.vcxproj` | Native/interop project — excluded from automated .NET conversion   |

### Project upgrade details
This section contains details about each project upgrade and modifications that need to be done in the project.

#### Intermediate\ProjectFiles\UE5.vcxproj (excluded)

This is a native Unreal Engine C++ project generated under `Intermediate\ProjectFiles`. It is excluded from the .NET upgrade plan. No SDK-style conversion will be attempted.

Action items (manual, optional):
  - Keep project as native `.vcxproj` and manage with Unreal Build Tool (UBT).
  - If you later want managed components upgraded, move them into separate `.csproj` managed projects.

#### Intermediate\ProjectFiles\LiveLinkHub.vcxproj (excluded)

Native Unreal Engine C++ project. Excluded from automated .NET upgrade.

Action items (manual, optional):
  - Use UE toolchain and follow manual troubleshooting steps if build errors occur.

#### Intermediate\ProjectFiles\IoStoreOnDemandTests.vcxproj (excluded)

Native Unreal Engine test project. Excluded from automated .NET upgrade.

Action items (manual, optional):
  - Run `Build.bat` for this target manually and follow UBT logs to fix native compile issues.

#### Intermediate\ProjectFiles\GASP.vcxproj (excluded)

Native Unreal Engine C++ project. Excluded from automated .NET upgrade.

Action items (manual, optional):
  - Use UE tools to debug and fix native build issues.

#### Intermediate\ProjectFiles\EventLoopUnitTests.vcxproj (excluded)

Native Unreal Engine test project. Excluded from automated .NET upgrade.

Action items (manual, optional):
  - Run and inspect UBT logs for first error and fix native issues.

#### Intermediate\ProjectFiles\DotNetPerforceLib.vcxproj (excluded)

Native or interop project. Excluded from automated .NET upgrade.

Action items (manual, optional):
  - If this project contains managed wrapper code that should be upgraded, extract managed parts into a separate `.csproj` and upgrade that.

#### Intermediate\Build\BuildRulesProjects\GASPModuleRules\GASPModuleRules.csproj modifications

Project properties changes:
  - Confirmed upgraded to `net8.0` during execution. No further automated changes required.

NuGet packages changes:
  - No specific NuGet package updates were identified by analysis for this project file; verify package references and update to compatible versions for `net8.0` as needed.

Other changes:
  - Validate build rules integration after changes.




