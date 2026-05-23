// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#include "ToolRegistry.h"
#include "JsonObjectConverter.h"
#include "TokenManager.h"
#include "UE_MCP_Plugin.h"
#include "UE_MCP_Server.h"

// JSON-RPC Error Codes
#define JSONRPC_PARSE_ERROR -32700
#define JSONRPC_INVALID_REQUEST -32600
#define JSONRPC_METHOD_NOT_FOUND -32601
#define JSONRPC_INVALID_PARAMS -32602
#define JSONRPC_INTERNAL_ERROR -32603

FToolRegistry::FToolRegistry() {}

FToolRegistry::~FToolRegistry() { ClearAll(); }

bool FToolRegistry::RegisterTool(const FString &Name,
                                 const FString &Description,
                                 TSharedPtr<FJsonObject> InputSchema,
                                 FOnToolExecute ExecuteDelegate) {
  if (Name.IsEmpty()) {
    UE_LOG(LogTemp, Error,
           TEXT("ToolRegistry: Cannot register tool with empty name"));
    return false;
  }

  if (!ExecuteDelegate.IsBound()) {
    UE_LOG(
        LogTemp, Error,
        TEXT("ToolRegistry: Cannot register tool '%s' with unbound delegate"),
        *Name);
    return false;
  }

  if (Tools.Contains(Name)) {
    UE_LOG(LogTemp, Warning,
           TEXT("ToolRegistry: Tool '%s' already registered, overwriting"),
           *Name);
  }

  FMCPToolInfo ToolInfo(Name, Description);
  ToolInfo.InputSchema = InputSchema;
  ToolInfo.ExecuteDelegate = ExecuteDelegate;

  Tools.Add(Name, ToolInfo);

  UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Registered tool '%s' - %s"), *Name,
         *Description);

  // Forward newly registered tool to the MCP Server if available so HTTP calls
  // can reach it immediately
  {
    FUE_MCP_PluginModule *PluginModule =
        FModuleManager::GetModulePtr<FUE_MCP_PluginModule>(
            TEXT("UE_MCP_Plugin"));
    UE_LOG(LogTemp, Log, TEXT("ToolRegistry: PluginModule valid=%d"),
           PluginModule != nullptr);
    if (PluginModule) {
      UE_LOG(LogTemp, Log, TEXT("ToolRegistry: MCPServer valid=%d"),
             PluginModule->GetMCPServer() != nullptr);
    }
    if (PluginModule && PluginModule->GetMCPServer()) {
      FOnMCPToolExecute MCPDelegate;
      MCPDelegate.BindLambda([this](const FString &ToolName,
                                    TSharedPtr<FJsonObject> Params,
                                    TSharedPtr<FJsonObject> &OutResult) {
        this->ExecuteTool(ToolName, Params, OutResult);
      });

      PluginModule->GetMCPServer()->RegisterTool(Name, Description,
                     InputSchema,
                     MCPDelegate);
      UE_LOG(LogTemp, Log,
             TEXT("ToolRegistry: Forwarded tool '%s' to MCP Server"), *Name);
    }
  }

  return true;
}

bool FToolRegistry::UnregisterTool(const FString &Name) {
  if (Tools.Remove(Name) > 0) {
    UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Unregistered tool '%s'"), *Name);
    return true;
  }

  UE_LOG(LogTemp, Warning,
         TEXT("ToolRegistry: Tool '%s' not found for unregistration"), *Name);
  return false;
}

bool FToolRegistry::HasTool(const FString &Name) const {
  return Tools.Contains(Name);
}

const FMCPToolInfo *FToolRegistry::GetToolInfo(const FString &Name) const {
  return Tools.Find(Name);
}

bool FToolRegistry::ExecuteTool(const FString &Name,
                                TSharedPtr<FJsonObject> Params,
                                TSharedPtr<FJsonObject> &OutResult) {
  const FMCPToolInfo *ToolInfo = GetToolInfo(Name);
  if (!ToolInfo) {
    UE_LOG(LogTemp, Error, TEXT("ToolRegistry: Tool '%s' not found"), *Name);
    return false;
  }

  if (!ToolInfo->ExecuteDelegate.IsBound()) {
    UE_LOG(LogTemp, Error,
           TEXT("ToolRegistry: Tool '%s' has no bound delegate"), *Name);
    return false;
  }

  UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Executing tool '%s'"), *Name);

  // Simple capability enforcement for certain sensitive tool namespaces
  if (Name.StartsWith(TEXT("usd."))) {
    // Expect params to contain a token
    FString TokenStr;
    if (Params.IsValid() && Params->HasField(TEXT("token"))) {
      TokenStr = Params->GetStringField(TEXT("token"));
    }

    if (TokenStr.IsEmpty()) {
      UE_LOG(LogTemp, Warning,
             TEXT("ToolRegistry: Missing token for USD tool '%s'"), *Name);
      if (OutResult.IsValid()) {
        OutResult->SetBoolField(TEXT("success"), false);
        OutResult->SetStringField(TEXT("error"),
                                  TEXT("Missing token for USD operation"));
      }
      return false;
    }

    // Validate token via plugin's TokenManager if available
    FUE_MCP_PluginModule *PluginModule =
        FModuleManager::GetModulePtr<FUE_MCP_PluginModule>(
            TEXT("UE_MCP_Plugin"));
    if (!PluginModule || !PluginModule->GetTokenManager()) {
      UE_LOG(LogTemp, Warning,
             TEXT("ToolRegistry: TokenManager unavailable; cannot validate "
                  "token for '%s'"),
             *Name);
      if (OutResult.IsValid()) {
        OutResult->SetBoolField(TEXT("success"), false);
        OutResult->SetStringField(
            TEXT("error"), TEXT("Server token validation not configured"));
      }
      return false;
    }

    FJWTToken ValidatedToken;
    if (!PluginModule->GetTokenManager()->ValidateToken(TokenStr,
                                                        ValidatedToken)) {
      UE_LOG(LogTemp, Warning,
             TEXT("ToolRegistry: Token validation failed for '%s'"), *Name);
      if (OutResult.IsValid()) {
        OutResult->SetBoolField(TEXT("success"), false);
        OutResult->SetStringField(TEXT("error"), TEXT("Invalid token"));
      }
      return false;
    }

    // Check capability
    TArray<FString> Caps =
        PluginModule->GetTokenManager()->GetCapabilities(ValidatedToken);
    FString RequiredCap =
        Name; // require exact capability match (e.g., usd.import)
    bool bHasCap = Caps.Contains(RequiredCap) || Caps.Contains(TEXT("usd.*")) ||
                   Caps.Contains(TEXT("*"));
    if (!bHasCap) {
      UE_LOG(LogTemp, Warning,
             TEXT("ToolRegistry: Token missing required capability '%s' for "
                  "tool '%s'"),
             *RequiredCap, *Name);
      if (OutResult.IsValid()) {
        OutResult->SetBoolField(TEXT("success"), false);
        OutResult->SetStringField(TEXT("error"),
                                  TEXT("Token lacks required capability"));
      }
      return false;
    }
  }

  // Execute the tool
  bool bSuccess = ToolInfo->ExecuteDelegate.Execute(Params, OutResult);

  if (bSuccess) {
    UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Tool '%s' executed successfully"),
           *Name);
  } else {
    UE_LOG(LogTemp, Error, TEXT("ToolRegistry: Tool '%s' execution failed"),
           *Name);
  }

  return bSuccess;
}

FString FToolRegistry::ExecuteToolJsonRpc(const FString &Name,
                                          TSharedPtr<FJsonObject> Params,
                                          int32 RequestId) {
  TSharedPtr<FJsonObject> Result = MakeShareable(new FJsonObject());

  if (!HasTool(Name)) {
    return CreateErrorResponse(
        JSONRPC_METHOD_NOT_FOUND,
        FString::Printf(TEXT("Tool not found: %s"), *Name), RequestId);
  }

  bool bSuccess = ExecuteTool(Name, Params, Result);

  if (bSuccess) {
    return CreateSuccessResponse(Result, RequestId);
  } else {
    return CreateErrorResponse(
        JSONRPC_INTERNAL_ERROR,
        FString::Printf(TEXT("Tool execution failed: %s"), *Name), RequestId);
  }
}

TArray<FString> FToolRegistry::GetToolNames() const {
  TArray<FString> Names;
  Tools.GetKeys(Names);
  return Names;
}

TArray<TSharedPtr<FJsonValue>> FToolRegistry::GetToolsAsJsonArray() const {
  TArray<TSharedPtr<FJsonValue>> ToolsArray;

  for (const auto &ToolPair : Tools) {
    const FMCPToolInfo &Tool = ToolPair.Value;

    TSharedPtr<FJsonObject> ToolObject = MakeShareable(new FJsonObject());
    ToolObject->SetStringField(TEXT("name"), Tool.Name);
    ToolObject->SetStringField(TEXT("description"), Tool.Description);

    if (Tool.InputSchema.IsValid()) {
      ToolObject->SetObjectField(TEXT("inputSchema"), Tool.InputSchema);
    }

    ToolsArray.Add(MakeShareable(new FJsonValueObject(ToolObject)));
  }

  return ToolsArray;
}

void FToolRegistry::ClearAll() {
  int32 Count = Tools.Num();
  Tools.Empty();
  UE_LOG(LogTemp, Log, TEXT("ToolRegistry: Cleared %d tools"), Count);
}

FString FToolRegistry::CreateErrorResponse(int32 Code, const FString &Message,
                                           int32 RequestId) {
  TSharedPtr<FJsonObject> ErrorObject = MakeShareable(new FJsonObject());
  ErrorObject->SetNumberField(TEXT("code"), Code);
  ErrorObject->SetStringField(TEXT("message"), Message);

  TSharedPtr<FJsonObject> ResponseObject = MakeShareable(new FJsonObject());
  ResponseObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
  ResponseObject->SetObjectField(TEXT("error"), ErrorObject);
  ResponseObject->SetNumberField(TEXT("id"), RequestId);

  FString ResponseString;
  TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&ResponseString);
  FJsonSerializer::Serialize(ResponseObject.ToSharedRef(), Writer);

  return ResponseString;
}

FString FToolRegistry::CreateSuccessResponse(TSharedPtr<FJsonObject> Result,
                                             int32 RequestId) {
  TSharedPtr<FJsonObject> ResponseObject = MakeShareable(new FJsonObject());
  ResponseObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
  ResponseObject->SetObjectField(TEXT("result"), Result);
  ResponseObject->SetNumberField(TEXT("id"), RequestId);

  FString ResponseString;
  TSharedRef<TJsonWriter<>> Writer =
      TJsonWriterFactory<>::Create(&ResponseString);
  FJsonSerializer::Serialize(ResponseObject.ToSharedRef(), Writer);

  return ResponseString;
}
