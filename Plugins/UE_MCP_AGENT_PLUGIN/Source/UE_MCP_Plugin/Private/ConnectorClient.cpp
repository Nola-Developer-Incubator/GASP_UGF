// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#include "ConnectorClient.h"
#include "WebSocketsModule.h"
#include "JsonObjectConverter.h"
#include "TimerManager.h"
#include "Engine/Engine.h"

FConnectorClient::FConnectorClient()
	: bIsConnected(false)
	, Protocol(TEXT("mcp"))
	, bTokenNeedsRefresh(false)
	, bAutoReconnect(false)
	, ReconnectInterval(5.0f)
	, NextRequestId(1)
{
}

FConnectorClient::~FConnectorClient()
{
	Disconnect();
}

bool FConnectorClient::Connect(const FString& URL, const FString& InProtocol)
{
	if (bIsConnected)
	{
		UE_LOG(LogTemp, Warning, TEXT("ConnectorClient: Already connected to %s"), *ServerURL);
		return false;
	}

	ServerURL = URL;
	Protocol = InProtocol;

	// Create WebSocket
	WebSocket = FWebSocketsModule::Get().CreateWebSocket(ServerURL, Protocol);

	if (!WebSocket.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ConnectorClient: Failed to create WebSocket for %s"), *ServerURL);
		return false;
	}

	// Bind delegates
	WebSocket->OnConnected().AddSP(this, &FConnectorClient::OnConnected);
	WebSocket->OnConnectionError().AddSP(this, &FConnectorClient::OnConnectionError);
	WebSocket->OnClosed().AddSP(this, &FConnectorClient::OnClosed);
	WebSocket->OnMessage().AddSP(this, &FConnectorClient::OnMessage);
	WebSocket->OnRawMessage().AddSP(this, &FConnectorClient::OnRawMessage);

	// Initiate connection
	WebSocket->Connect();
	
	UE_LOG(LogTemp, Log, TEXT("ConnectorClient: Connecting to %s"), *ServerURL);
	
	return true;
}

void FConnectorClient::Disconnect()
{
	if (!WebSocket.IsValid())
	{
		return;
	}

	ClearReconnectTimer();

	WebSocket->OnConnected().RemoveAll(this);
	WebSocket->OnConnectionError().RemoveAll(this);
	WebSocket->OnClosed().RemoveAll(this);
	WebSocket->OnMessage().RemoveAll(this);
	WebSocket->OnRawMessage().RemoveAll(this);

	if (bIsConnected)
	{
		WebSocket->Close();
	}

	WebSocket.Reset();
	bIsConnected = false;

	UE_LOG(LogTemp, Log, TEXT("ConnectorClient: Disconnected"));
}

bool FConnectorClient::SendMessage(const FString& Message)
{
	if (!bIsConnected || !WebSocket.IsValid())
	{
		UE_LOG(LogTemp, Warning, TEXT("ConnectorClient: Cannot send message, not connected"));
		return false;
	}

	WebSocket->Send(Message);
	UE_LOG(LogTemp, Verbose, TEXT("ConnectorClient: Sent message: %s"), *Message);
	
	return true;
}

bool FConnectorClient::SendJsonRpcRequest(const FString& Method, TSharedPtr<FJsonObject> Params, int32 RequestId)
{
	TSharedPtr<FJsonObject> RequestObject = MakeShareable(new FJsonObject());
	RequestObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	RequestObject->SetStringField(TEXT("method"), Method);
	RequestObject->SetNumberField(TEXT("id"), RequestId);

	if (Params.IsValid())
	{
		RequestObject->SetObjectField(TEXT("params"), Params);
	}

	// Add auth token if available
	if (!AuthToken.IsEmpty())
	{
		RequestObject->SetStringField(TEXT("token"), AuthToken);
	}

	FString RequestString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&RequestString);
	FJsonSerializer::Serialize(RequestObject.ToSharedRef(), Writer);

	return SendMessage(RequestString);
}

bool FConnectorClient::SendJsonRpcNotification(const FString& Method, TSharedPtr<FJsonObject> Params)
{
	TSharedPtr<FJsonObject> NotificationObject = MakeShareable(new FJsonObject());
	NotificationObject->SetStringField(TEXT("jsonrpc"), TEXT("2.0"));
	NotificationObject->SetStringField(TEXT("method"), Method);

	if (Params.IsValid())
	{
		NotificationObject->SetObjectField(TEXT("params"), Params);
	}

	// Add auth token if available
	if (!AuthToken.IsEmpty())
	{
		NotificationObject->SetStringField(TEXT("token"), AuthToken);
	}

	FString NotificationString;
	TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&NotificationString);
	FJsonSerializer::Serialize(NotificationObject.ToSharedRef(), Writer);

	return SendMessage(NotificationString);
}

void FConnectorClient::SetAuthToken(const FString& Token)
{
	AuthToken = Token;
	bTokenNeedsRefresh = false;
	UE_LOG(LogTemp, Log, TEXT("ConnectorClient: Auth token set"));
}

void FConnectorClient::SetAutoReconnect(bool bEnable, float RetryInterval)
{
	bAutoReconnect = bEnable;
	ReconnectInterval = RetryInterval;
	UE_LOG(LogTemp, Log, TEXT("ConnectorClient: Auto-reconnect %s (interval: %.1fs)"), 
		bEnable ? TEXT("enabled") : TEXT("disabled"), RetryInterval);
}

void FConnectorClient::OnConnected()
{
	bIsConnected = true;
	UE_LOG(LogTemp, Log, TEXT("ConnectorClient: Connected to %s"), *ServerURL);

	// Notify listeners
	OnConnectionStateChanged.ExecuteIfBound(true);

	// Clear any pending reconnect timers
	ClearReconnectTimer();
}

void FConnectorClient::OnConnectionError(const FString& Error)
{
	UE_LOG(LogTemp, Error, TEXT("ConnectorClient: Connection error - %s"), *Error);
	
	bIsConnected = false;
	OnConnectionStateChanged.ExecuteIfBound(false);

	if (bAutoReconnect)
	{
		AttemptReconnect();
	}
}

void FConnectorClient::OnClosed(int32 StatusCode, const FString& Reason, bool bWasClean)
{
	UE_LOG(LogTemp, Log, TEXT("ConnectorClient: Connection closed - Code: %d, Reason: %s, Clean: %d"), 
		StatusCode, *Reason, bWasClean);

	bIsConnected = false;
	OnConnectionStateChanged.ExecuteIfBound(false);

	if (bAutoReconnect && !bWasClean)
	{
		AttemptReconnect();
	}
}

void FConnectorClient::OnMessage(const FString& Message)
{
	UE_LOG(LogTemp, Verbose, TEXT("ConnectorClient: Received message: %s"), *Message);
	
	// Notify listeners
	OnMessageReceived.ExecuteIfBound(Message);
}

void FConnectorClient::OnRawMessage(const void* Data, SIZE_T Size, SIZE_T BytesRemaining)
{
	// Convert binary data to string if needed
	FString Message = FString::FromBlob(static_cast<const uint8*>(Data), Size);
	OnMessage(Message);
}

void FConnectorClient::AttemptReconnect()
{
	if (!bAutoReconnect)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ConnectorClient: Attempting reconnect in %.1f seconds..."), ReconnectInterval);

	if (UWorld* World = ResolveTimerWorld())
	{
		ReconnectTimerWorld = World;
		World->GetTimerManager().SetTimer(
			ReconnectTimerHandle,
			[this]()
			{
				Disconnect();
				Connect(ServerURL, Protocol);
			},
			ReconnectInterval,
			false
		);
	}
}

void FConnectorClient::RefreshToken()
{
	if (!bTokenNeedsRefresh)
	{
		return;
	}

	UE_LOG(LogTemp, Log, TEXT("ConnectorClient: Token refresh needed"));
	
	// Send token refresh request
	TSharedPtr<FJsonObject> Params = MakeShareable(new FJsonObject());
	SendJsonRpcRequest(TEXT("auth/refresh"), Params, NextRequestId++);
}

UWorld* FConnectorClient::ResolveTimerWorld() const
{
	if (ReconnectTimerWorld.IsValid())
	{
		return ReconnectTimerWorld.Get();
	}

	if (GEngine)
	{
		for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
		{
			if (UWorld* World = WorldContext.World())
			{
				return World;
			}
		}
	}

	return nullptr;
}

void FConnectorClient::ClearReconnectTimer()
{
	if (UWorld* World = ResolveTimerWorld())
	{
		World->GetTimerManager().ClearTimer(ReconnectTimerHandle);
	}

	ReconnectTimerHandle.Invalidate();
	ReconnectTimerWorld.Reset();
}
