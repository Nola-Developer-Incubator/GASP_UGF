#include "AgentDashboard.h"
#include "ToolRegistry.h"
#include "UE_MCP_Plugin.h"
#include "Modules/ModuleManager.h"
#include "Styling/AppStyle.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Layout/SScrollBox.h"
#include "Widgets/Layout/SBox.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SEditableTextBox.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Views/SListView.h"
#include "McpTelemetryLibrary.h"

namespace
{
FToolRegistry* GetActiveToolRegistry()
{
    FUE_MCP_PluginModule* Module = FModuleManager::GetModulePtr<FUE_MCP_PluginModule>(TEXT("UE_MCP_Plugin"));
    return Module ? Module->GetToolRegistry() : nullptr;
}
}

void SAgentDashboard::Construct(const FArguments& InArgs)
{
    // populate initial tool list
    if (FToolRegistry* Reg = GetActiveToolRegistry())
    {
        TArray<FString> Names = Reg->GetToolNames();
        for (auto& Name : Names)
        {
            ToolNames.Add(MakeShared<FString>(Name));
        }
    }

    ChildSlot
    [
        SNew(SBorder)
        .Padding(8)
        .BorderImage(FAppStyle::GetBrush("ToolPanel.GroupBorder"))
        [
            SNew(SVerticalBox)

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(2)
            [
                SNew(SButton)
                .Text(FText::FromString("Refresh Tools"))
                .OnClicked(this, &SAgentDashboard::OnRefreshClicked)
            ]

            + SVerticalBox::Slot()
            .AutoHeight()
            .Padding(2)
            [
                SNew(SButton)
                .Text(FText::FromString("Send Test Telemetry"))
                .OnClicked(this, &SAgentDashboard::OnSendTelemetryClicked)
            ]

            + SVerticalBox::Slot()
            .FillHeight(1.0f)
            .Padding(2)
            [
                SNew(SListView<TSharedPtr<FString>>)
                .ListItemsSource(&ToolNames)
                .OnGenerateRow_Lambda([](TSharedPtr<FString> Item, const TSharedRef<STableViewBase>& Owner)
                {
                    return SNew(STableRow<TSharedPtr<FString>>, Owner)
                        [ SNew(STextBlock).Text(FText::FromString(*Item)) ];
                })
            ]
        ]
    ];
}

FReply SAgentDashboard::OnRefreshClicked()
{
    ToolNames.Empty();
    if (FToolRegistry* Reg = GetActiveToolRegistry())
    {
        TArray<FString> Names = Reg->GetToolNames();
        for (auto& Name : Names)
        {
            ToolNames.Add(MakeShared<FString>(Name));
        }
    }
    return FReply::Handled();
}

FReply SAgentDashboard::OnSendTelemetryClicked()
{
    // fire an example telemetry event
    UMcpTelemetryLibrary::ReportTelemetry(TEXT("{\"example\":true}"));
    return FReply::Handled();
}
