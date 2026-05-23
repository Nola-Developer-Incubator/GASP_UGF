#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UE_MCP_PLUGIN_API FActorPropertiesTool
{
public:
	static bool ExecuteGet(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static bool ExecuteSet(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchemaGet();
	static TSharedPtr<FJsonObject> GetInputSchemaSet();
	static FString GetNameGet() { return TEXT("actor.get_properties"); }
	static FString GetNameSet() { return TEXT("actor.set_properties"); }
	static FString GetDescriptionGet() { return TEXT("Get properties for an actor"); }
	static FString GetDescriptionSet() { return TEXT("Set properties for an actor (location, rotation, label)"); }
};