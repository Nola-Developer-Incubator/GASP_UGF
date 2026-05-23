#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UE_MCP_PLUGIN_API FActorFindTool
{
public:
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchema();
	static FString GetName() { return TEXT("actors.find"); }
	static FString GetDescription() { return TEXT("Find an actor by name or GUID"); }
};