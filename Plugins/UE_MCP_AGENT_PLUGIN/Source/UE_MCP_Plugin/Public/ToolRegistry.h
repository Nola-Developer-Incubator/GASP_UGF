// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Dom/JsonObject.h"

/**
 * Delegate for tool execution
 * @param Params - Tool parameters as JSON object
 * @param OutResult - Result object to populate
 * @return true if execution successful
 */
DECLARE_DELEGATE_RetVal_TwoParams(bool, FOnToolExecute, TSharedPtr<FJsonObject> /*Params*/, TSharedPtr<FJsonObject>& /*OutResult*/);

/**
 * Represents a registered tool
 */
struct FMCPToolInfo
{
	FString Name;
	FString Description;
	TSharedPtr<FJsonObject> InputSchema;
	FOnToolExecute ExecuteDelegate;

	FMCPToolInfo()
	{}

	FMCPToolInfo(const FString& InName, const FString& InDescription)
		: Name(InName)
		, Description(InDescription)
	{}
};

/**
 * ToolRegistry manages MCP tools
 */
class UE_MCP_PLUGIN_API FToolRegistry
{
public:
	FToolRegistry();
	~FToolRegistry();

	/**
	 * Register a tool
	 * @param Name - Tool name
	 * @param Description - Tool description
	 * @param InputSchema - JSON schema for input parameters
	 * @param ExecuteDelegate - Delegate to execute the tool
	 * @return true if registered successfully
	 */
	bool RegisterTool(const FString& Name, const FString& Description, TSharedPtr<FJsonObject> InputSchema, FOnToolExecute ExecuteDelegate);

	/**
	 * Unregister a tool
	 * @param Name - Tool name
	 * @return true if unregistered successfully
	 */
	bool UnregisterTool(const FString& Name);

	/**
	 * Check if a tool is registered
	 * @param Name - Tool name
	 * @return true if tool exists
	 */
	bool HasTool(const FString& Name) const;

	/**
	 * Get tool information
	 * @param Name - Tool name
	 * @return Pointer to tool info if found
	 */
	const FMCPToolInfo* GetToolInfo(const FString& Name) const;

	/**
	 * Execute a tool with JSON parameters
	 * @param Name - Tool name
	 * @param Params - Parameters as JSON object
	 * @param OutResult - Result as JSON object
	 * @return true if execution successful
	 */
	bool ExecuteTool(const FString& Name, TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult);

	/**
	 * Create JSON-RPC response for tool execution
	 * @param Name - Tool name
	 * @param Params - Parameters
	 * @param RequestId - Request ID
	 * @return JSON-RPC response as string
	 */
	FString ExecuteToolJsonRpc(const FString& Name, TSharedPtr<FJsonObject> Params, int32 RequestId);

	/**
	 * Get list of all registered tools
	 * @return Array of tool names
	 */
	TArray<FString> GetToolNames() const;

	/**
	 * Get all registered tools as JSON array
	 * @return JSON array of tool information
	 */
	TArray<TSharedPtr<FJsonValue>> GetToolsAsJsonArray() const;

	/**
	 * Clear all registered tools
	 */
	void ClearAll();

private:
	/**
	 * Create JSON-RPC error response
	 * @param Code - Error code
	 * @param Message - Error message
	 * @param RequestId - Request ID
	 * @return JSON-RPC response as string
	 */
	FString CreateErrorResponse(int32 Code, const FString& Message, int32 RequestId);

	/**
	 * Create JSON-RPC success response
	 * @param Result - Result object
	 * @param RequestId - Request ID
	 * @return JSON-RPC response as string
	 */
	FString CreateSuccessResponse(TSharedPtr<FJsonObject> Result, int32 RequestId);

	// Map of tool name to tool info
	TMap<FString, FMCPToolInfo> Tools;
};
