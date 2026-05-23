#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UE_MCP_PLUGIN_API FActorListTool
{
public:
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchema();
	static FString GetName() { return TEXT("actors.list"); }
	static FString GetDescription() { return TEXT("Lists actors in the current world with optional filters"); }
};