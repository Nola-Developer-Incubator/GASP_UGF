// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Modules/ModuleManager.h"

/**
 * MCP Server Plugin Module
 * Enterprise-grade Unreal Engine plugin implementing the Model Context Protocol (MCP)
 * Provides secure AI-assisted automation inside the Unreal Editor using WebSocket,
 * JSON-RPC, JWT authentication, and capability-based policy enforcement.
 */
class FMCPServerPluginModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};
