#include "Tools/UsdImportTool.h"
#include "Dom/JsonObject.h"

#if WITH_EDITOR
#include "Editor.h"
#include "AssetToolsModule.h"
#include "IAssetTools.h"
#include "Async/Async.h"
#include "EngineUtils.h"
#include "LevelEditor.h"
#include "Engine/StaticMesh.h"
#include "Components/StaticMeshComponent.h"
#include "Selection.h"
#include "Interfaces/IPluginManager.h"
#endif

#include "ToolRegistry.h"

// Minimal async job struct for USD imports
struct FUsdImportJob
{
    FString JobId;
    FString SourcePath;
    FString DestinationPath;
    FString Status; // pending, running, success, failed
    FString ResultAssetPath;
    FString Log;
};

static FCriticalSection GUsdImportJobsMutex;
static TMap<FString, TSharedPtr<FUsdImportJob>> GUsdImportJobs;

static FString GenerateUsdJobId()
{
    return FGuid::NewGuid().ToString();
}

#if WITH_EDITOR

bool PerformUsdImportSync(const FString& SourcePath, const FString& DestPath, FString& OutAssetPath, FString& OutLog)
{
    if (!FPaths::FileExists(SourcePath))
    {
        OutLog = TEXT("Source file not found");
        return false;
    }

    // Ensure USD plugin is enabled
    if (!IPluginManager::Get().FindEnabledPlugin(TEXT("USDImporter")).IsValid())
    {
        OutLog = TEXT("USD plugin is not enabled. Enable the 'USD' plugin in Edit->Plugins->Importers->USD and restart the editor.");
        return false;
    }

    // Use the USD importer by invoking the generic asset import with the USD file
    FAssetToolsModule& AssetToolsModule = FModuleManager::LoadModuleChecked<FAssetToolsModule>("AssetTools");
    IAssetTools& AssetTools = AssetToolsModule.Get();

    TArray<FString> Files;
    Files.Add(SourcePath);

    FString PackagePath = DestPath;
    if (PackagePath.IsEmpty()) PackagePath = TEXT("/Game/Imported/USD");

    TArray<UObject*> ImportedAssets = AssetTools.ImportAssets(Files, PackagePath);

    if (ImportedAssets.Num() == 0)
    {
        OutLog = TEXT("No assets were imported");
        return false;
    }

    UObject* First = ImportedAssets[0];
    if (!First)
    {
        OutLog = TEXT("Imported asset pointer invalid");
        return false;
    }

    OutAssetPath = First->GetPathName();
    OutLog = FString::Printf(TEXT("Imported %d assets from %s"), ImportedAssets.Num(), *SourcePath);
    return true;
}

void PerformUsdImportAsync(TSharedPtr<FUsdImportJob> Job)
{
    {
        FScopeLock Lock(&GUsdImportJobsMutex);
        Job->Status = TEXT("running");
    }

    Async(EAsyncExecution::ThreadPool, [Job]() {
        FString AssetPath, Log;
        bool bOk = PerformUsdImportSync(Job->SourcePath, Job->DestinationPath, AssetPath, Log);

        {
            FScopeLock Lock(&GUsdImportJobsMutex);
            Job->Status = bOk ? TEXT("success") : TEXT("failed");
            Job->ResultAssetPath = AssetPath;
            Job->Log = Log;
        }

        AsyncTask(ENamedThreads::GameThread, [Job]() {
            UE_LOG(LogTemp, Log, TEXT("UsdImport: Job %s completed with status %s: %s"), *Job->JobId, *Job->Status, *Job->Log);
            // TODO: Notify MCP clients via server notifications if a reference is available
        });
    });
}

#endif // WITH_EDITOR

bool FUsdImportTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
    OutResult->SetBoolField(TEXT("success"), false);
    OutResult->SetStringField(TEXT("error"), TEXT("USD import functionality is not available in this build."));
    return false;
}

TSharedPtr<FJsonObject> FUsdImportTool::GetInputSchema()
{
    TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
    Schema->SetStringField(TEXT("type"), TEXT("object"));
    return Schema;
}

bool FUsdImportStatusTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
    OutResult->SetBoolField(TEXT("success"), false);

    if (!Params.IsValid() || !Params->HasField(TEXT("jobId")))
    {
        OutResult->SetStringField(TEXT("error"), TEXT("jobId is required"));
        return false;
    }

    const FString JobId = Params->GetStringField(TEXT("jobId"));

    TSharedPtr<FUsdImportJob> Job;
    {
        FScopeLock Lock(&GUsdImportJobsMutex);
        if (!GUsdImportJobs.Contains(JobId))
        {
            OutResult->SetStringField(TEXT("error"), TEXT("job not found"));
            return false;
        }

        Job = GUsdImportJobs[JobId];
    }

    OutResult->SetStringField(TEXT("jobId"), Job->JobId);
    OutResult->SetStringField(TEXT("status"), Job->Status);
    OutResult->SetStringField(TEXT("assetPath"), Job->ResultAssetPath);
    OutResult->SetStringField(TEXT("log"), Job->Log);
    OutResult->SetBoolField(TEXT("success"), true);
    return true;
}

TSharedPtr<FJsonObject> FUsdImportStatusTool::GetInputSchema()
{
    TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
    Schema->SetStringField(TEXT("type"), TEXT("object"));
    TSharedPtr<FJsonObject> Props = MakeShareable(new FJsonObject());

    TSharedPtr<FJsonObject> JobProp = MakeShareable(new FJsonObject());
    JobProp->SetStringField(TEXT("type"), TEXT("string"));
    Props->SetObjectField(TEXT("jobId"), JobProp);

    Schema->SetObjectField(TEXT("properties"), Props);
    return Schema;
}

bool FUsdExportTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
    OutResult->SetBoolField(TEXT("success"), false);

    if (!Params.IsValid())
    {
        OutResult->SetStringField(TEXT("error"), TEXT("Invalid params"));
        return false;
    }

    if (!Params->HasField(TEXT("destinationPath")))
    {
        OutResult->SetStringField(TEXT("error"), TEXT("destinationPath is required"));
        return false;
    }

    FString Dest = Params->GetStringField(TEXT("destinationPath"));

    // If USD plugin unavailable, return helpful error
    if (!IPluginManager::Get().FindEnabledPlugin(TEXT("USDImporter")).IsValid())
    {
        OutResult->SetStringField(TEXT("error"), TEXT("USD plugin is not enabled. Enable the 'USD' plugin in Edit->Plugins->Importers->USD and restart the editor."));
        return false;
    }

    // Find world
    UWorld* World = nullptr;
    if (GEngine)
    {
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor || Context.WorldType == EWorldType::PIE)
            {
                World = Context.World();
                break;
            }
        }
    }

    if (!World)
    {
        OutResult->SetStringField(TEXT("error"), TEXT("No valid world found"));
        return false;
    }

    // Collect actors: either by GUIDs array or by current editor selection
    TArray<FString> RequestedGuids;
    if (Params->HasField(TEXT("actorGuids")))
    {
        const TArray<TSharedPtr<FJsonValue>>* Arr;
        if (Params->TryGetArrayField(TEXT("actorGuids"), Arr))
        {
            for (const TSharedPtr<FJsonValue>& V : *Arr)
            {
                if (V.IsValid() && V->Type == EJson::String)
                {
                    RequestedGuids.Add(V->AsString());
                }
            }
        }
    }

    TArray<AActor*> ActorsToExport;

    if (RequestedGuids.Num() > 0)
    {
        // Iterate actors and match GUIDs
        for (TActorIterator<AActor> It(World); It; ++It)
        {
            AActor* A = *It;
            if (!A) continue;
            FString ActorGuid = A->GetActorGuid().ToString();
            if (RequestedGuids.Contains(ActorGuid))
            {
                ActorsToExport.Add(A);
            }
        }
    }
    else
    {
        // Use editor selection
        if (GEditor)
        {
            for (FSelectionIterator It(GEditor->GetSelectedActorIterator()); It; ++It)
            {
                AActor* Actor = Cast<AActor>(*It);
                if (Actor) ActorsToExport.Add(Actor);
            }
        }
    }

    if (ActorsToExport.Num() == 0)
    {
        OutResult->SetStringField(TEXT("error"), TEXT("No actors to export (none selected or matching GUIDs)"));
        return false;
    }

    // Build a USDA stage with actor transforms and minimal metadata. Keep the
    // exporter stable on UE 5.7 instead of depending on private mesh internals.
    FString UsdContent;
    UsdContent += TEXT("#usda 1.0\n\n");
    UsdContent += TEXT("def Xform \"Root\"\n{\n}\n\n");

    auto SanitizeName = [](const FString& Name)->FString {
        FString S = Name;
        S.ReplaceInline(TEXT(" "), TEXT("_"));
        S.ReplaceInline(TEXT("\""), TEXT(""));
        return S;
    };

    for (AActor* Actor : ActorsToExport)
    {
        FString PrimName = SanitizeName(Actor->GetName());
        FVector Loc = Actor->GetActorLocation();
        FRotator Rot = Actor->GetActorRotation();
        FVector Scale = Actor->GetActorScale3D();

        UsdContent += FString::Printf(TEXT("def Xform \"%s\"\n{\n    double3 xformOp:translate = (%f, %f, %f)\n    float3 xformOp:rotateXYZ = (%f, %f, %f)\n    double3 xformOp:scale = (%f, %f, %f)\n"), *PrimName, Loc.X, Loc.Y, Loc.Z, Rot.Pitch, Rot.Yaw, Rot.Roll, Scale.X, Scale.Y, Scale.Z);

        // Add metadata attributes
        UsdContent += FString::Printf(TEXT("    string unreal:actorGuid = \"%s\"\n"), *Actor->GetActorGuid().ToString());
        UsdContent += FString::Printf(TEXT("    string unreal:actorClass = \"%s\"\n"), *Actor->GetClass()->GetPathName());

        // Attach simple metadata for static meshes without exporting raw render data.
        UStaticMeshComponent* SMC = Actor->FindComponentByClass<UStaticMeshComponent>();
        if (SMC && SMC->GetStaticMesh())
        {
            UsdContent += FString::Printf(TEXT("    string unreal:staticMesh = \"%s\"\n"), *SMC->GetStaticMesh()->GetPathName());

            if (SMC->GetMaterial(0))
            {
                UsdContent += FString::Printf(TEXT("    string unreal:material0 = \"%s\"\n"), *SMC->GetMaterial(0)->GetPathName());
            }
        }

        UsdContent += TEXT("}\n\n");
    }

    // Ensure directory exists for destination
    FString DestDir = FPaths::GetPath(Dest);
    if (!IFileManager::Get().DirectoryExists(*DestDir))
    {
        IFileManager::Get().MakeDirectory(*DestDir, true);
    }

    bool bWrote = FFileHelper::SaveStringToFile(UsdContent, *Dest);
    if (!bWrote)
    {
        OutResult->SetStringField(TEXT("error"), TEXT("Failed to write USD file"));
        return false;
    }

    OutResult->SetBoolField(TEXT("success"), true);
    OutResult->SetStringField(TEXT("exportPath"), Dest);
    OutResult->SetNumberField(TEXT("actorCount"), ActorsToExport.Num());
    return true;
}

TSharedPtr<FJsonObject> FUsdExportTool::GetInputSchema()
{
    TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
    Schema->SetStringField(TEXT("type"), TEXT("object"));
    return Schema;
}
