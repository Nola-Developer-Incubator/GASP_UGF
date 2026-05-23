#include "Tools/ActorFindTool.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

static AActor* FindActorByGuidOrName(UWorld* World, const FString& GuidOrName)
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

bool FActorFindTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
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

	AActor* Actor = FindActorByGuidOrName(World, Query);
	if (!Actor)
	{
		OutResult->SetBoolField(TEXT("success"), false);
		OutResult->SetStringField(TEXT("error"), TEXT("Actor not found"));
		return false;
	}

	TSharedPtr<FJsonObject> ActorObject = MakeShareable(new FJsonObject());
	ActorObject->SetStringField(TEXT("name"), Actor->GetName());
	ActorObject->SetStringField(TEXT("label"), Actor->GetActorLabel());
	ActorObject->SetStringField(TEXT("class"), Actor->GetClass()->GetPathName());
	ActorObject->SetStringField(TEXT("actorGuid"), Actor->GetActorGuid().ToString());

	TSharedPtr<FJsonObject> Location = MakeShareable(new FJsonObject());
	FVector ActorLocation = Actor->GetActorLocation();
	Location->SetNumberField(TEXT("x"), ActorLocation.X);
	Location->SetNumberField(TEXT("y"), ActorLocation.Y);
	Location->SetNumberField(TEXT("z"), ActorLocation.Z);
	ActorObject->SetObjectField(TEXT("location"), Location);

	OutResult->SetObjectField(TEXT("actor"), ActorObject);
	OutResult->SetBoolField(TEXT("success"), true);
	return true;
}

TSharedPtr<FJsonObject> FActorFindTool::GetInputSchema()
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());

	TSharedPtr<FJsonObject> QueryProperty = MakeShareable(new FJsonObject());
	QueryProperty->SetStringField(TEXT("type"), TEXT("string"));
	QueryProperty->SetStringField(TEXT("description"), TEXT("Actor name, label, or GUID"));
	Properties->SetObjectField(TEXT("query"), QueryProperty);

	Schema->SetObjectField(TEXT("properties"), Properties);
	return Schema;
}