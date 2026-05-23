#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Onboarding tool that creates a sample blueprint asset and wiring.
 */
class UE_MCP_PLUGIN_API FOnboardTool
{
public:
    static bool Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);
    static TSharedPtr<FJsonObject> GetInputSchema();
    static FString GetName() { return TEXT("onboard.createExample"); }
    static FString GetDescription() { return TEXT("Creates an example actor Blueprint pre‑wired with telemetry"); }
};
