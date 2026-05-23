// Copyright 2026 Nola Development Incubator. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

class UE_MCP_PLUGIN_API FLevelListTool
{
public:
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchema();
	static FString GetName() { return TEXT("level.list"); }
	static FString GetDescription() { return TEXT("Lists the persistent and streaming levels in the current world"); }
};

class UE_MCP_PLUGIN_API FLevelOpenTool
{
public:
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchema();
	static FString GetName() { return TEXT("level.open"); }
	static FString GetDescription() { return TEXT("Opens a level in the Unreal Editor"); }
};

class UE_MCP_PLUGIN_API FPIETool
{
public:
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
	static TSharedPtr<FJsonObject> GetInputSchema();
	static FString GetName() { return TEXT("pie.control"); }
	static FString GetDescription() { return TEXT("Starts or stops Play-In-Editor"); }
};