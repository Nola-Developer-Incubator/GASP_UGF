#include "McpTelemetryLibrary.h"
#include "HttpModule.h"
#include "Interfaces/IHttpRequest.h"
#include "Interfaces/IHttpResponse.h"
#include "UEMCPAgentSettings.h"
#include "Engine/World.h"
#include "TimerManager.h"

UMcpTelemetryLibrary::FOnTelemetrySent UMcpTelemetryLibrary::TelemetrySentEvent;

void UMcpTelemetryLibrary::ReportTelemetry(const FString& PayloadJson)
{
    FString BaseUrl;
    FString ApiKey;
    const UUEMCPAgentSettings* Settings = GetDefault<UUEMCPAgentSettings>();
    if (Settings)
    {
        BaseUrl = Settings->TelemetryBaseUrl;
        ApiKey = Settings->TelemetryApiKey;
    }

    if (BaseUrl.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("McpTelemetryLibrary: BaseUrl not configured"));
    }

    if (BaseUrl.IsEmpty())
    {
        UE_LOG(LogTemp, Warning, TEXT("McpTelemetryLibrary: telemetry BaseUrl is empty; payload queued"));
        // queue for later retry
        static TArray<FString> Pending;
        Pending.Add(PayloadJson);
        TelemetrySentEvent.Broadcast(false);
        return;
    }

    // construct full telemetry endpoint
    FString Url = BaseUrl;
    if (!Url.EndsWith("/"))
    {
        Url += TEXT("/");
    }
    Url += TEXT("telemetry");

    auto SendRequest = [&](const FString& Body, int32 Attempt) -> void
    {
        TSharedRef<IHttpRequest> Request = FHttpModule::Get().CreateRequest();
        Request->SetURL(Url);
        Request->SetVerb(TEXT("POST"));
        Request->SetHeader(TEXT("Content-Type"), TEXT("application/json"));
        if (!ApiKey.IsEmpty())
        {
            Request->SetHeader(TEXT("x-api-key"), ApiKey);
        }
        Request->SetContentAsString(Body);

        Request->OnProcessRequestComplete().BindLambda([Body, Attempt](FHttpRequestPtr Req, FHttpResponsePtr Resp, bool bSuccess)
        {
            bool bWasOk = bSuccess && Resp.IsValid() && EHttpResponseCodes::IsOk(Resp->GetResponseCode());
            if (!bWasOk && Attempt < 3)
            {
                // simple backoff: delay = 2^Attempt seconds
                FTimerHandle Timer;
                if (GWorld)
                {
                    GWorld->GetTimerManager().SetTimer(Timer, [Body]() { UMcpTelemetryLibrary::ReportTelemetry(Body); }, FMath::Pow(2.0f, Attempt), false);
                }
            }
            TelemetrySentEvent.Broadcast(bWasOk);
        });

        Request->ProcessRequest();
    };

    // immediately send
    SendRequest(PayloadJson, 0);
}
