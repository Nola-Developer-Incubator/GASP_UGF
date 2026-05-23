#include "Tools/LevelTools.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "Engine/LevelStreaming.h"
#include "Engine/Level.h"

#if WITH_EDITOR
#include "Editor.h"
#include "FileHelpers.h"
#include "Editor/EditorEngine.h"
#include "LevelEditor.h"
#endif

bool FLevelListTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
    OutResult->SetBoolField(TEXT("success"), false);

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

    TArray<TSharedPtr<FJsonValue>> LevelsArray;

    // Main persistent level
    if (World->PersistentLevel)
    {
        TSharedPtr<FJsonObject> LObj = MakeShareable(new FJsonObject());
        LObj->SetStringField(TEXT("name"), World->PersistentLevel->GetOutermost()->GetName());
        LevelsArray.Add(MakeShareable(new FJsonValueObject(LObj)));
    }

    // Streaming levels
    for (ULevelStreaming* StreamingLevel : World->GetStreamingLevels())
    {
        if (!StreamingLevel) continue;
        TSharedPtr<FJsonObject> LObj = MakeShareable(new FJsonObject());
        LObj->SetStringField(TEXT("package"), StreamingLevel->GetWorldAssetPackageName());
        LObj->SetBoolField(TEXT("loaded"), StreamingLevel->IsLevelLoaded());
        LevelsArray.Add(MakeShareable(new FJsonValueObject(LObj)));
    }

    OutResult->SetArrayField(TEXT("levels"), LevelsArray);
    OutResult->SetBoolField(TEXT("success"), true);
    return true;
}

bool FLevelOpenTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
    OutResult->SetBoolField(TEXT("success"), false);

    if (!Params.IsValid() || !Params->HasField(TEXT("levelPath")))
    {
        OutResult->SetStringField(TEXT("error"), TEXT("levelPath is required"));
        return false;
    }

    FString Path = Params->GetStringField(TEXT("levelPath"));

#if WITH_EDITOR
    if (!GEditor)
    {
        OutResult->SetStringField(TEXT("error"), TEXT("Editor context not available"));
        return false;
    }

    bool bLoaded = FEditorFileUtils::LoadMap(Path, false, true);
    OutResult->SetBoolField(TEXT("success"), bLoaded);
    if (!bLoaded) OutResult->SetStringField(TEXT("error"), TEXT("Failed to open level"));
    return bLoaded;
#else
    OutResult->SetStringField(TEXT("error"), TEXT("Level open is only supported in editor builds"));
    return false;
#endif
}

bool FPIETool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
    OutResult->SetBoolField(TEXT("success"), false);

    if (!Params.IsValid() || !Params->HasField(TEXT("action")))
    {
        OutResult->SetStringField(TEXT("error"), TEXT("action is required (start|stop)"));
        return false;
    }

    FString Action = Params->GetStringField(TEXT("action"));

#if WITH_EDITOR
    if (!GEditor)
    {
        OutResult->SetStringField(TEXT("error"), TEXT("Editor context not available"));
        return false;
    }

    if (Action.Equals(TEXT("start"), ESearchCase::IgnoreCase))
    {
        UWorld* World = nullptr;
        for (const FWorldContext& Context : GEngine->GetWorldContexts())
        {
            if (Context.WorldType == EWorldType::Editor)
            {
                World = Context.World();
                break;
            }
        }
        if (!World)
        {
            OutResult->SetStringField(TEXT("error"), TEXT("No editor world found"));
            return false;
        }

        FRequestPlaySessionParams PlaySessionParams;
        FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>(TEXT("LevelEditor"));
        PlaySessionParams.DestinationSlateViewport = LevelEditorModule.GetFirstActiveViewport();
        GEditor->RequestPlaySession(PlaySessionParams);
        OutResult->SetBoolField(TEXT("success"), true);
        return true;
    }
    else if (Action.Equals(TEXT("stop"), ESearchCase::IgnoreCase))
    {
        GEditor->RequestEndPlayMap();
        OutResult->SetBoolField(TEXT("success"), true);
        return true;
    }
    else
    {
        OutResult->SetStringField(TEXT("error"), TEXT("Unknown action"));
        return false;
    }
#else
    OutResult->SetStringField(TEXT("error"), TEXT("PIE control only supported in editor builds"));
    return false;
#endif
}

TSharedPtr<FJsonObject> FLevelListTool::GetInputSchema()
{
    TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
    Schema->SetStringField(TEXT("type"), TEXT("object"));
    return Schema;
}

TSharedPtr<FJsonObject> FLevelOpenTool::GetInputSchema()
{
    TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
    Schema->SetStringField(TEXT("type"), TEXT("object"));
    TSharedPtr<FJsonObject> Props = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> PathProp = MakeShareable(new FJsonObject());
    PathProp->SetStringField(TEXT("type"), TEXT("string"));
    PathProp->SetStringField(TEXT("description"), TEXT("Path to map asset (e.g. /Game/Maps/MyMap)"));
    Props->SetObjectField(TEXT("levelPath"), PathProp);
    Schema->SetObjectField(TEXT("properties"), Props);
    return Schema;
}

TSharedPtr<FJsonObject> FPIETool::GetInputSchema()
{
    TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
    Schema->SetStringField(TEXT("type"), TEXT("object"));
    TSharedPtr<FJsonObject> Props = MakeShareable(new FJsonObject());
    TSharedPtr<FJsonObject> ActionProp = MakeShareable(new FJsonObject());
    ActionProp->SetStringField(TEXT("type"), TEXT("string"));
    ActionProp->SetStringField(TEXT("description"), TEXT("action: start or stop"));
    Props->SetObjectField(TEXT("action"), ActionProp);
    Schema->SetObjectField(TEXT("properties"), Props);
    return Schema;
}
