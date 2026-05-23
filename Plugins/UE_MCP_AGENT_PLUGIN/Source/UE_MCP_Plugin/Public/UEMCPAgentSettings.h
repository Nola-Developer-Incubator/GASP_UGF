// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "UEMCPAgentSettings.generated.h"

UCLASS(config=Engine, defaultconfig, meta=(DisplayName="UE MCP Agent"))
class UE_MCP_PLUGIN_API UUEMCPAgentSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	UUEMCPAgentSettings();

	virtual FName GetCategoryName() const override;

	UPROPERTY(EditAnywhere, config, Category="Server", meta=(ClampMin="1", ClampMax="65535"))
	int32 ServerPort;

	UPROPERTY(EditAnywhere, config, Category="Connector")
	FString ConnectorUrl;

	UPROPERTY(EditAnywhere, config, Category="Connector")
	FString ConnectorProtocol;

	UPROPERTY(EditAnywhere, config, Category="Connector")
	bool bEnableConnectorAutoReconnect;

	UPROPERTY(EditAnywhere, config, Category="Connector", meta=(ClampMin="0.1"))
	float ConnectorReconnectInterval;

	UPROPERTY(EditAnywhere, config, Category="Security")
	FString JwksUrl;

	UPROPERTY(EditAnywhere, config, Category="Security", meta=(DisplayName="Require HTTP Authentication When JWKS Is Configured"))
	bool bRequireAuthenticationForHttpRequests;

	UPROPERTY(EditAnywhere, config, Category="Security")
	FString ExpectedTokenAudience;

	UPROPERTY(EditAnywhere, config, Category="Security")
	TArray<FString> AllowedTokenIssuers;

	UPROPERTY(EditAnywhere, config, Category="Security", meta=(ClampMin="0", ClampMax="300"))
	int32 TokenClockSkewSeconds;

	UPROPERTY(EditAnywhere, config, Category="Security", meta=(DisplayName="Enforce Tool Capability Claims"))
	bool bEnforceToolCapabilities;

	UPROPERTY(EditAnywhere, config, Category="Security", meta=(DisplayName="Allow Insecure Token Validation (Development Only)"))
	bool bAllowInsecureTokenValidation;

	UPROPERTY(EditAnywhere, config, Category="Telemetry")
	FString TelemetryBaseUrl;

	UPROPERTY(EditAnywhere, config, Category="Telemetry")
	FString TelemetryApiKey;
};
