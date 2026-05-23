// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "IWebSocket.h"
#include "Dom/JsonObject.h"

/**
 * Delegate for when a message is received from the WebSocket
 */
DECLARE_DELEGATE_OneParam(FOnWebSocketMessage, const FString& /*Message*/);

/**
 * Delegate for connection state changes
 */
DECLARE_DELEGATE_OneParam(FOnConnectionStateChanged, bool /*bIsConnected*/);

/**
 * ConnectorClient handles WebSocket connection to MCP server
 * with automatic reconnection and token refresh
 */
class UE_MCP_PLUGIN_API FConnectorClient : public TSharedFromThis<FConnectorClient>
{
public:
	FConnectorClient();
	~FConnectorClient();

	/**
	 * Connect to WebSocket server
	 * @param URL - WebSocket URL (e.g., ws://localhost:3000)
	 * @param Protocol - WebSocket protocol (default: "mcp")
	 * @return true if connection initiated successfully
	 */
	bool Connect(const FString& URL, const FString& Protocol = TEXT("mcp"));

	/**
	 * Disconnect from WebSocket server
	 */
	void Disconnect();

	/**
	 * Check if connected to server
	 * @return true if connected
	 */
	bool IsConnected() const { return bIsConnected; }

	/**
	 * Send a JSON-RPC message to the server
	 * @param Message - JSON-RPC message as string
	 * @return true if message was sent successfully
	 */
	bool SendMessage(const FString& Message);

	/**
	 * Send a JSON-RPC request
	 * @param Method - Method name
	 * @param Params - Parameters as JSON object
	 * @param RequestId - Request ID
	 * @return true if sent successfully
	 */
	bool SendJsonRpcRequest(const FString& Method, TSharedPtr<FJsonObject> Params, int32 RequestId);

	/**
	 * Send a JSON-RPC notification (no response expected)
	 * @param Method - Method name
	 * @param Params - Parameters as JSON object
	 * @return true if sent successfully
	 */
	bool SendJsonRpcNotification(const FString& Method, TSharedPtr<FJsonObject> Params);

	/**
	 * Set authentication token
	 * @param Token - JWT token
	 */
	void SetAuthToken(const FString& Token);

	/**
	 * Enable/disable automatic reconnection
	 * @param bEnable - true to enable auto-reconnect
	 * @param RetryInterval - Seconds between retry attempts (default: 5)
	 */
	void SetAutoReconnect(bool bEnable, float RetryInterval = 5.0f);

	/**
	 * Bind delegate for incoming messages
	 */
	FOnWebSocketMessage OnMessageReceived;

	/**
	 * Bind delegate for connection state changes
	 */
	FOnConnectionStateChanged OnConnectionStateChanged;

private:
	/**
	 * Handle WebSocket connection established
	 */
	void OnConnected();

	/**
	 * Handle WebSocket connection closed
	 * @param StatusCode - Close status code
	 * @param Reason - Reason for closure
	 * @param bWasClean - Whether closure was clean
	 */
	void OnConnectionError(const FString& Error);

	/**
	 * Handle WebSocket connection closed
	 */
	void OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean);

	/**
	 * Handle incoming WebSocket message
	 * @param Message - Message content
	 */
	void OnMessage(const FString& Message);

	/**
	 * Handle incoming WebSocket binary message
	 * @param Data - Binary data
	 * @param Size - Data size
	 */
	void OnRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining);

	/**
	 * Attempt to reconnect to server
	 */
	void AttemptReconnect();

	/**
	 * Handle token refresh
	 */
	void RefreshToken();

	UWorld* ResolveTimerWorld() const;
	void ClearReconnectTimer();

	// WebSocket instance
	TSharedPtr<IWebSocket> WebSocket;

	// Connection state
	bool bIsConnected;
	FString ServerURL;
	FString Protocol;

	// Authentication
	FString AuthToken;
	bool bTokenNeedsRefresh;

	// Auto-reconnect settings
	bool bAutoReconnect;
	float ReconnectInterval;
	FTimerHandle ReconnectTimerHandle;
	TWeakObjectPtr<UWorld> ReconnectTimerWorld;

	// Request tracking
	int32 NextRequestId;
};
