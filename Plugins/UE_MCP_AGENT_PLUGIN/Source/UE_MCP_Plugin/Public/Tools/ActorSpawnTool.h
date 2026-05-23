// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Actor Spawn Tool
 * Spawns an actor in the current level with specified class, location, and rotation
 */
class UE_MCP_PLUGIN_API FActorSpawnTool
{
public:
	/**
	 * Execute the actor spawn tool
	 * @param Params - JSON parameters containing className, location, rotation
	 * @param OutResult - Result JSON containing actorGuid
	 * @return true if actor spawned successfully
	 */
	static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);

	/**
	 * Get the tool's input schema
	 * @return JSON schema for tool inputs
	 */
	static TSharedPtr<FJsonObject> GetInputSchema();

	/**
	 * Get tool name
	 */
	static FString GetName() { return TEXT("actors.spawn"); }

	/**
	 * Get tool description
	 */
	static FString GetDescription() { return TEXT("Spawns an actor in the current level with specified class name, location, and rotation"); }
};
