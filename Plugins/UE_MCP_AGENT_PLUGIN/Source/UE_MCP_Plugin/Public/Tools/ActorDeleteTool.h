#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UE_MCP_PLUGIN_API FActorDeleteTool
{
public:
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchema();
	static FString GetName() { return TEXT("actors.delete"); }
	static FString GetDescription() { return TEXT("Deletes an actor by GUID or name"); }
};