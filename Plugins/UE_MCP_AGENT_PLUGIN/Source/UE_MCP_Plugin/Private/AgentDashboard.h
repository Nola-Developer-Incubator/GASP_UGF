#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"

/**
 * Simple dashboard showing registered MCP tools and a manual telemetry button.
 */
class SAgentDashboard : public SCompoundWidget
{
public:
    SLATE_BEGIN_ARGS(SAgentDashboard) {}
    SLATE_END_ARGS()

    void Construct(const FArguments& InArgs);

private:
    /** handle when user clicks "Refresh" */
    FReply OnRefreshClicked();
    /** handle when user clicks "Send Test Telemetry" */
    FReply OnSendTelemetryClicked();

    /** cached list of tool names */
    TArray<TSharedPtr<FString>> ToolNames;
};
