#include "Tools/OnboardTool.h"
#include "Kismet2/KismetEditorUtilities.h"
#include "Engine/Blueprint.h"
#include "UObject/Package.h"
#include "AssetRegistry/AssetRegistryModule.h"

bool FOnboardTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
    // create package and blueprint asset
    const FString PackageName = TEXT("/Game/MCPExamples/BP_MCPExample");
    UPackage* Package = CreatePackage(*PackageName);
    UBlueprint* NewBP = FKismetEditorUtilities::CreateBlueprint(
        AActor::StaticClass(),
        Package,
        FName(*FPaths::GetBaseFilename(PackageName)),
        BPTYPE_Normal,
        UBlueprint::StaticClass(),
        UBlueprintGeneratedClass::StaticClass());

    if (!NewBP)
    {
        return false;
    }

    // mark new asset dirty so it appears in content browser
    NewBP->MarkPackageDirty();
    FAssetRegistryModule::AssetCreated(NewBP);

    // optionally add instructions to result
    OutResult = MakeShared<FJsonObject>();
    OutResult->SetStringField(TEXT("path"), PackageName);
    return true;
}

TSharedPtr<FJsonObject> FOnboardTool::GetInputSchema()
{
    // no input parameters
    return MakeShared<FJsonObject>();
}
