using UnrealBuildTool;

public class UE_MCP_Plugin : ModuleRules
{
    public UE_MCP_Plugin(ReadOnlyTargetRules Target) : base(Target)
    {
        PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

        PublicDependencyModuleNames.AddRange(new string[] {
            "Core",
            "CoreUObject",
            "Engine",
            "Json",
            "JsonUtilities",
            "HTTP",
            "HTTPServer",
            "Sockets",
            "WebSockets",
            "Slate",
            "SlateCore",
            "Projects",
            "DeveloperSettings"
        });

        PrivateDependencyModuleNames.AddRange(new string[] {
            "InputCore"
        });

        if (Target.bBuildEditor)
        {
            PrivateDependencyModuleNames.AddRange(new string[] {
                "EditorStyle",
                "UnrealEd",
                "LevelEditor",
                "AssetTools",
                "AssetRegistry",
                "Kismet",
                "KismetCompiler",
                "KismetWidgets",
                "WorkspaceMenuStructure"
            });
        }

        PublicAdditionalLibraries.Add("bcrypt.lib");
    }
}
