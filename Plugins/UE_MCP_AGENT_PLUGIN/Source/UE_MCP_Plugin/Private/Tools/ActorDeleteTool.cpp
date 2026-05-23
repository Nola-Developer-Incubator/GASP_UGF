#include "Tools/ActorDeleteTool.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

static AActor* FindActorByGuidOrNameLocal(UWorld* World, const FString& GuidOrName)
{
	if (!World) return nullptr;
	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor) continue;
		if (Actor->GetActorGuid().ToString() == GuidOrName) return Actor;
		if (Actor->GetName() == GuidOrName) return Actor;
		if (Actor->GetActorLabel() == GuidOrName) return Actor;
	}
	return nullptr;
}

bool FActorDeleteTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
	OutResult->SetBoolField(TEXT("success"), false);

	if (!Params.IsValid())
	{
		OutResult->SetStringField(TEXT("error"), TEXT("Invalid params"));
		return false;
	}

	FString Query;
	if (Params->HasField(TEXT("query"))) Query = Params->GetStringField(TEXT("query"));

	if (Query.IsEmpty())
	{
		OutResult->SetStringField(TEXT("error"), TEXT("query is required"));
		return false;
	}

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

	AActor* Actor = FindActorByGuidOrNameLocal(World, Query);
	if (!Actor)
	{
		OutResult->SetStringField(TEXT("error"), TEXT("Actor not found"));
		return false;
	}

	const FString ActorName = Actor->GetName();
	bool bDestroyed = World->DestroyActor(Actor);

	OutResult->SetBoolField(TEXT("success"), bDestroyed);
	if (!bDestroyed)
	{
		OutResult->SetStringField(TEXT("error"), TEXT("Failed to destroy actor"));
	}
	else
	{
		OutResult->SetStringField(TEXT("deleted"), ActorName);
	}

	return bDestroyed;
}

TSharedPtr<FJsonObject> FActorDeleteTool::GetInputSchema()
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());

	TSharedPtr<FJsonObject> QueryProperty = MakeShareable(new FJsonObject());
	QueryProperty->SetStringField(TEXT("type"), TEXT("string"));
	QueryProperty->SetStringField(TEXT("description"), TEXT("Actor name, label, or GUID to delete"));
	Properties->SetObjectField(TEXT("query"), QueryProperty);

	Schema->SetObjectField(TEXT("properties"), Properties);
	return Schema;
}