// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#include "UE_MCP_Plugin.h"
#include "AgentDashboard.h"
#include "ConnectorClient.h"
#include "McpTelemetryLibrary.h"
#include "TokenManager.h"
#include "ToolRegistry.h"
#include "UE_MCP_Server.h"
#include "UEMCPAgentSettings.h"
#include "Framework/Docking/TabManager.h"
#include "Tools/ActorDeleteTool.h"
#include "Tools/ActorFindTool.h"
#include "Tools/ActorListTool.h"
#include "Tools/ActorPropertiesTool.h"
#include "Tools/ActorSpawnTool.h"
#include "Tools/LevelTools.h"
#include "Tools/OnboardTool.h"
#include "Tools/UsdImportTool.h"

#define LOCTEXT_NAMESPACE "FUE_MCP_PluginModule"

namespace
{
constexpr TCHAR DashboardTabId[] = TEXT("MCPAgentDashboard");

void RegisterServerTool(
    const TSharedPtr<FUE_MCP_Server>& Server,
    const TSharedPtr<FToolRegistry>& Registry,
    const FString& ToolName,
    const FString& Description,
    const TArray<FString>& RequiredCapabilities = TArray<FString>())
{
    if (!Server.IsValid() || !Registry.IsValid())
    {
        return;
    }

    const TWeakPtr<FToolRegistry> WeakRegistry = Registry;
    FOnMCPToolExecute Delegate;
    Delegate.BindLambda([WeakRegistry](const FString& RequestedToolName, TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
    {
        const TSharedPtr<FToolRegistry> PinnedRegistry = WeakRegistry.Pin();
        if (!PinnedRegistry.IsValid())
        {
            OutResult = MakeShared<FJsonObject>();
            OutResult->SetBoolField(TEXT("success"), false);
            OutResult->SetStringField(TEXT("error"), TEXT("Tool registry unavailable during shutdown"));
            return;
        }

        PinnedRegistry->ExecuteTool(RequestedToolName, Params, OutResult);
    });

    Server->RegisterTool(ToolName, Description, Delegate, RequiredCapabilities);
}

TArray<FString> GetRequiredCapabilitiesForTool(const FString& ToolName)
{
    if (ToolName == TEXT("actors.delete") || ToolName == TEXT("delete_actor"))
    {
        return { TEXT("tools.actors.delete") };
    }

    if (ToolName == TEXT("actor.set_properties") || ToolName == TEXT("set_actor_properties") || ToolName == TEXT("actors.spawn"))
    {
        return { TEXT("tools.actors.write") };
    }

    if (ToolName == TEXT("level.open") || ToolName == TEXT("open_level"))
    {
        return { TEXT("tools.level.write") };
    }

    if (ToolName == TEXT("pie") || ToolName == TEXT("pie.start") || ToolName == TEXT("pie.stop"))
    {
        return { TEXT("tools.pie.control") };
    }

    if (ToolName == TEXT("asset.import") || ToolName == TEXT("usd.import"))
    {
        return { TEXT("tools.assets.import") };
    }

    if (ToolName == TEXT("usd.export"))
    {
        return { TEXT("tools.assets.export") };
    }

    return {};
}
}

void FUE_MCP_PluginModule::StartupModule()
{
    UE_LOG(LogTemp, Log, TEXT("UE_MCP_Plugin: Module Starting"));

    if (IsRunningCommandlet())
    {
        UE_LOG(LogTemp, Log, TEXT("UE_MCP_Plugin: Running in commandlet mode; skipping MCP service startup"));
        return;
    }

    InitializeComponents();
    RegisterBuiltInTools();

    FGlobalTabmanager::Get()->RegisterNomadTabSpawner(
        DashboardTabId,
        FOnSpawnTab::CreateRaw(this, &FUE_MCP_PluginModule::SpawnDashboardTab))
        .SetDisplayName(LOCTEXT("MCPDashboardTitle", "MCP Agent Dashboard"))
        .SetMenuType(ETabSpawnerMenuType::Enabled);

    UE_LOG(LogTemp, Log, TEXT("UE_MCP_Plugin: Module Started Successfully"));
}

void FUE_MCP_PluginModule::ShutdownModule()
{
    UE_LOG(LogTemp, Log, TEXT("UE_MCP_Plugin: Module Shutting Down"));

    if (!IsRunningCommandlet())
    {
        FGlobalTabmanager::Get()->UnregisterNomadTabSpawner(DashboardTabId);
    }

    ShutdownComponents();
    UE_LOG(LogTemp, Log, TEXT("UE_MCP_Plugin: Module Shutdown Complete"));
}

void FUE_MCP_PluginModule::InitializeComponents()
{
    MCPServer = MakeShared<FUE_MCP_Server>();
    ConnectorClient = MakeShared<FConnectorClient>();
    TokenManager = MakeShared<FTokenManager>();
    ToolRegistry = MakeShared<FToolRegistry>();

    const UUEMCPAgentSettings* Settings = GetDefault<UUEMCPAgentSettings>();
    const int32 ServerPort = Settings ? Settings->ServerPort : 4020;
    bool bRequireHttpAuthentication = false;

    if (Settings && !Settings->JwksUrl.IsEmpty())
    {
        if (!TokenManager->Initialize(
            Settings->JwksUrl,
            Settings->bAllowInsecureTokenValidation,
            Settings->ExpectedTokenAudience,
            Settings->AllowedTokenIssuers,
            Settings->TokenClockSkewSeconds))
        {
            UE_LOG(LogTemp, Error, TEXT("UE_MCP_Plugin: Token manager failed to initialize with configured JWKS source; refusing to expose an unauthenticated MCP endpoint"));
            return;
        }

        bRequireHttpAuthentication = true;
    }
    else if (Settings && Settings->bRequireAuthenticationForHttpRequests)
    {
        UE_LOG(LogTemp, Error, TEXT("UE_MCP_Plugin: HTTP authentication was required, but no JWKS URL is configured"));
        return;
    }

    if (MCPServer.IsValid())
    {
        MCPServer->ConfigureSecurity(TokenManager, bRequireHttpAuthentication, Settings ? Settings->bEnforceToolCapabilities : false);
        MCPServer->StartServer(ServerPort);
    }

    if (Settings && ConnectorClient.IsValid() && !Settings->ConnectorUrl.IsEmpty())
    {
        ConnectorClient->SetAutoReconnect(Settings->bEnableConnectorAutoReconnect, Settings->ConnectorReconnectInterval);

        if (Settings->ConnectorUrl.StartsWith(TEXT("ws://")) || Settings->ConnectorUrl.StartsWith(TEXT("wss://")))
        {
            ConnectorClient->Connect(
                Settings->ConnectorUrl,
                Settings->ConnectorProtocol.IsEmpty() ? TEXT("mcp") : Settings->ConnectorProtocol);
        }
        else
        {
            UE_LOG(LogTemp, Warning, TEXT("UE_MCP_Plugin: ConnectorClient only supports WebSocket URLs. Skipping '%s'"), *Settings->ConnectorUrl);
        }
    }
}

void FUE_MCP_PluginModule::RegisterBuiltInTools()
{
    if (!ToolRegistry.IsValid())
    {
        UE_LOG(LogTemp, Error, TEXT("UE_MCP_Plugin: Tool registry is not valid"));
        return;
    }

    auto RegisterTool = [this](const FString& Name, const FString& Description, TSharedPtr<FJsonObject> Schema, FOnToolExecute ExecuteDelegate)
    {
        ToolRegistry->RegisterTool(Name, Description, Schema, ExecuteDelegate);
        RegisterServerTool(MCPServer, ToolRegistry, Name, Description, GetRequiredCapabilitiesForTool(Name));
        UE_LOG(LogTemp, Log, TEXT("UE_MCP_Plugin: Registered tool '%s'"), *Name);
    };

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorSpawnTool::Execute(Params, OutResult);
        });
        RegisterTool(FActorSpawnTool::GetName(), FActorSpawnTool::GetDescription(), FActorSpawnTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorListTool::Execute(Params, OutResult);
        });
        RegisterTool(FActorListTool::GetName(), FActorListTool::GetDescription(), FActorListTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorFindTool::Execute(Params, OutResult);
        });
        RegisterTool(FActorFindTool::GetName(), FActorFindTool::GetDescription(), FActorFindTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorDeleteTool::Execute(Params, OutResult);
        });
        RegisterTool(FActorDeleteTool::GetName(), FActorDeleteTool::GetDescription(), FActorDeleteTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorPropertiesTool::ExecuteGet(Params, OutResult);
        });
        RegisterTool(FActorPropertiesTool::GetNameGet(), FActorPropertiesTool::GetDescriptionGet(), FActorPropertiesTool::GetInputSchemaGet(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorPropertiesTool::ExecuteSet(Params, OutResult);
        });
        RegisterTool(FActorPropertiesTool::GetNameSet(), FActorPropertiesTool::GetDescriptionSet(), FActorPropertiesTool::GetInputSchemaSet(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FLevelListTool::Execute(Params, OutResult);
        });
        RegisterTool(FLevelListTool::GetName(), FLevelListTool::GetDescription(), FLevelListTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FLevelOpenTool::Execute(Params, OutResult);
        });
        RegisterTool(FLevelOpenTool::GetName(), FLevelOpenTool::GetDescription(), FLevelOpenTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FPIETool::Execute(Params, OutResult);
        });
        RegisterTool(FPIETool::GetName(), FPIETool::GetDescription(), FPIETool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            TSharedPtr<FJsonObject> StartParams = MakeShared<FJsonObject>();
            StartParams->SetStringField(TEXT("action"), TEXT("start"));
            return FPIETool::Execute(StartParams, OutResult);
        });
        RegisterTool(TEXT("pie.start"), TEXT("Start Play-In-Editor (compat)"), FPIETool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            TSharedPtr<FJsonObject> StopParams = MakeShared<FJsonObject>();
            StopParams->SetStringField(TEXT("action"), TEXT("stop"));
            return FPIETool::Execute(StopParams, OutResult);
        });
        RegisterTool(TEXT("pie.stop"), TEXT("Stop Play-In-Editor (compat)"), FPIETool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            FString Payload;
            if (!Params.IsValid() || !Params->TryGetStringField(TEXT("payload"), Payload))
            {
                return false;
            }

            UMcpTelemetryLibrary::ReportTelemetry(Payload);
            OutResult = MakeShared<FJsonObject>();
            OutResult->SetBoolField(TEXT("success"), true);
            return true;
        });
        RegisterTool(TEXT("telemetry.report"), TEXT("Send a JSON telemetry payload to the MCP telemetry endpoint"), MakeShared<FJsonObject>(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FOnboardTool::Execute(Params, OutResult);
        });
        RegisterTool(FOnboardTool::GetName(), FOnboardTool::GetDescription(), FOnboardTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FUsdImportTool::Execute(Params, OutResult);
        });
        RegisterTool(FUsdImportTool::GetName(), FUsdImportTool::GetDescription(), FUsdImportTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FUsdImportStatusTool::Execute(Params, OutResult);
        });
        RegisterTool(FUsdImportStatusTool::GetName(), FUsdImportStatusTool::GetDescription(), FUsdImportStatusTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FUsdImportStatusTool::Execute(Params, OutResult);
        });
        RegisterTool(FUsdImportStatusTool::GetName(), FUsdImportStatusTool::GetDescription(), FUsdImportStatusTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FUsdExportTool::Execute(Params, OutResult);
        });
        RegisterTool(FUsdExportTool::GetName(), FUsdExportTool::GetDescription(), FUsdExportTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorListTool::Execute(Params, OutResult);
        });
        RegisterTool(TEXT("list_actors"), TEXT("Alias for actors.list"), FActorListTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorFindTool::Execute(Params, OutResult);
        });
        RegisterTool(TEXT("find_actor"), TEXT("Alias for actors.find"), FActorFindTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorDeleteTool::Execute(Params, OutResult);
        });
        RegisterTool(TEXT("delete_actor"), TEXT("Alias for actors.delete"), FActorDeleteTool::GetInputSchema(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorPropertiesTool::ExecuteGet(Params, OutResult);
        });
        RegisterTool(TEXT("get_actor_properties"), TEXT("Alias for actor.get_properties"), FActorPropertiesTool::GetInputSchemaGet(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FActorPropertiesTool::ExecuteSet(Params, OutResult);
        });
        RegisterTool(TEXT("set_actor_properties"), TEXT("Alias for actor.set_properties"), FActorPropertiesTool::GetInputSchemaSet(), Delegate);
    }

    {
        FOnToolExecute Delegate;
        Delegate.BindLambda([](TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult) -> bool
        {
            return FLevelOpenTool::Execute(Params, OutResult);
        });
        RegisterTool(TEXT("open_level"), TEXT("Alias for level.open"), FLevelOpenTool::GetInputSchema(), Delegate);
    }
}

TSharedRef<SDockTab> FUE_MCP_PluginModule::SpawnDashboardTab(const FSpawnTabArgs& Args)
{
    return SNew(SDockTab)
        .TabRole(ETabRole::NomadTab)
        [
            SNew(SAgentDashboard)
        ];
}

void FUE_MCP_PluginModule::ShutdownComponents()
{
    if (ConnectorClient.IsValid())
    {
        ConnectorClient->Disconnect();
        ConnectorClient.Reset();
    }

    if (MCPServer.IsValid())
    {
        MCPServer->StopServer();
        MCPServer.Reset();
    }

    if (TokenManager.IsValid())
    {
        TokenManager.Reset();
    }

    if (ToolRegistry.IsValid())
    {
        ToolRegistry.Reset();
    }
}

#undef LOCTEXT_NAMESPACE

IMPLEMENT_MODULE(FUE_MCP_PluginModule, UE_MCP_Plugin)
