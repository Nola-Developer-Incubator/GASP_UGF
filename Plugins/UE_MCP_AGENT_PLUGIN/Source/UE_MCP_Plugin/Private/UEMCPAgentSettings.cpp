// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#include "UEMCPAgentSettings.h"

UUEMCPAgentSettings::UUEMCPAgentSettings()
	: ServerPort(4020)
	, ConnectorProtocol(TEXT("mcp"))
	, bEnableConnectorAutoReconnect(false)
	, ConnectorReconnectInterval(5.0f)
	, bRequireAuthenticationForHttpRequests(false)
	, ExpectedTokenAudience(TEXT("ue-mcp-agent"))
	, TokenClockSkewSeconds(60)
	, bEnforceToolCapabilities(false)
	, bAllowInsecureTokenValidation(false)
{
}

FName UUEMCPAgentSettings::GetCategoryName() const
{
	return TEXT("Plugins");
}
