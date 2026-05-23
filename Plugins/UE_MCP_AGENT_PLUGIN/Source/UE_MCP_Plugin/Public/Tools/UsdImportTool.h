#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UE_MCP_PLUGIN_API FUsdImportTool
{
public:
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchema();
	static FString GetName() { return TEXT("usd.import"); }
	static FString GetDescription() { return TEXT("Import a USD file (usda/usdc/usdz) into the project content. Supports async import with job status notifications."); }
};

class UE_MCP_PLUGIN_API FUsdImportStatusTool
{
public:
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchema();
	static FString GetName() { return TEXT("usd.import_status"); }
	static FString GetDescription() { return TEXT("Query the status of an async USD import job by jobId"); }
};

class UE_MCP_PLUGIN_API FUsdExportTool
{
public:
    static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
    static TSharedPtr<FJsonObject> GetInputSchema();
    static FString GetName() { return TEXT("usd.export"); }
    static FString GetDescription() { return TEXT("Export selected actors/levels to a USD file (usda/usdc/usdz)"); }
};
