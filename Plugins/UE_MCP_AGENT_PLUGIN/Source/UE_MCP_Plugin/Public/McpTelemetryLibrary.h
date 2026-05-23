#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "McpTelemetryLibrary.generated.h"

/**
 * Blueprint helper functions for reporting telemetry to an MCP endpoint.
 * This lives in the MCP agent plugin so that any project using the plugin
 * can send telemetry without depending on the Adventure game code.
 */
UCLASS()
class UE_MCP_PLUGIN_API UMcpTelemetryLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    /**
     * Send a telemetry payload (arbitrary JSON string) to the configured
     * MCP telemetry endpoint. This function is non-blocking; use the
     * "OnTelemetrySent" delegate to get completion.
     */
    UFUNCTION(BlueprintCallable, Category="MCP|Telemetry")
    static void ReportTelemetry(const FString& PayloadJson);

    /**
     * Delegate called when telemetry has been sent (success or failure).
     */
    DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTelemetrySent, bool, bWasSuccessful);

    /**
     * Broadcast when telemetry is sent.
     *
     * Note: BlueprintAssignable cannot be used on static members, so this
     * delegate is not exposed to Blueprint binding.
     */
    static FOnTelemetrySent TelemetrySentEvent;
};
