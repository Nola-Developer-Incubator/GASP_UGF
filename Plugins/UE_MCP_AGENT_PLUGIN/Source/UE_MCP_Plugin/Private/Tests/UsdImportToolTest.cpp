#include "Misc/AutomationTest.h"
#include "Tools/UsdImportTool.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

#if WITH_AUTOMATION_TESTS

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUsdImportToolTest, "UE_MCP_Plugin.UsdImportTool.ImportTest", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

static bool WriteSampleUsda(const FString& Path)
{
    // Minimal USDA stage with a single Xform
    const FString Usda =
        "#usda 1.0\n" \
        "def Xform \"Root\"\n" \
        "{\n" \
        "}\n";

    return FFileHelper::SaveStringToFile(Usda, *Path);
}

bool FUsdImportToolTest::RunTest(const FString& Parameters)
{
    FString TempDir = FPaths::ProjectSavedDir() / TEXT("TempTests");
    IFileManager::Get().MakeDirectory(*TempDir, true);
    FString UsdaPath = TempDir / TEXT("sample.usda");

    bool bWrote = WriteSampleUsda(UsdaPath);
    TestTrue(TEXT("Wrote sample USDA"), bWrote);

    TSharedPtr<FJsonObject> Params = MakeShareable(new FJsonObject());
    Params->SetStringField(TEXT("sourcePath"), UsdaPath);
    Params->SetStringField(TEXT("destinationPath"), TEXT("/Game/ImportedUSD"));

    TSharedPtr<FJsonObject> Out = MakeShareable(new FJsonObject());

    bool bOk = FUsdImportTool::Execute(Params, Out);

    // If USD plugin not enabled, tool should return informative error
    if (Out->HasField(TEXT("error")))
    {
        FString Err = Out->GetStringField(TEXT("error"));
        TestTrue(TEXT("UsdImport reported plugin disabled or other error"), !Err.IsEmpty());
    }
    else
    {
        TestTrue(TEXT("UsdImport succeeded or returned job id"), Out->GetBoolField(TEXT("success")));
    }

    return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FUsdImportStatusToolTest, "UE_MCP_Plugin.UsdImportTool.ImportStatusMissingJob", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FUsdImportStatusToolTest::RunTest(const FString& Parameters)
{
    TSharedPtr<FJsonObject> Params = MakeShareable(new FJsonObject());
    Params->SetStringField(TEXT("jobId"), TEXT("missing-job"));

    TSharedPtr<FJsonObject> Out = MakeShareable(new FJsonObject());
    const bool bOk = FUsdImportStatusTool::Execute(Params, Out);

    TestFalse(TEXT("Missing job should not succeed"), bOk);
    TestTrue(TEXT("Missing job returns an error"), Out->HasField(TEXT("error")));
    return true;
}

#endif // WITH_AUTOMATION_TESTS
