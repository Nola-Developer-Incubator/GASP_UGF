#include "Tools/ActorListTool.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

bool FActorListTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
	OutResult->SetBoolField(TEXT("success"), false);

	FString ClassNameFilter;
	FString NameFilter;
	bool bIncludeComponents = false;
	bool bIncludeTags = false;

	if (Params.IsValid())
	{
		if (Params->HasField(TEXT("className"))) ClassNameFilter = Params->GetStringField(TEXT("className"));
		if (Params->HasField(TEXT("name"))) NameFilter = Params->GetStringField(TEXT("name"));
		if (Params->HasField(TEXT("includeComponents"))) bIncludeComponents = Params->GetBoolField(TEXT("includeComponents"));
		if (Params->HasField(TEXT("includeTags"))) bIncludeTags = Params->GetBoolField(TEXT("includeTags"));
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

	TArray<TSharedPtr<FJsonValue>> ActorsArray;

	for (TActorIterator<AActor> It(World); It; ++It)
	{
		AActor* Actor = *It;
		if (!Actor) continue;

		if (!ClassNameFilter.IsEmpty())
		{
			FString ActorClassPath = Actor->GetClass()->GetPathName();
			if (!ActorClassPath.Contains(ClassNameFilter)) continue;
		}

		if (!NameFilter.IsEmpty())
		{
			if (!Actor->GetName().Contains(NameFilter)) continue;
		}

		TSharedPtr<FJsonObject> ActorObj = MakeShareable(new FJsonObject());
		ActorObj->SetStringField(TEXT("name"), Actor->GetName());
		ActorObj->SetStringField(TEXT("label"), Actor->GetActorLabel());
		ActorObj->SetStringField(TEXT("class"), Actor->GetClass()->GetPathName());
		ActorObj->SetStringField(TEXT("actorGuid"), Actor->GetActorGuid().ToString());

		TSharedPtr<FJsonObject> Loc = MakeShareable(new FJsonObject());
		FVector LocV = Actor->GetActorLocation();
		Loc->SetNumberField(TEXT("x"), LocV.X);
		Loc->SetNumberField(TEXT("y"), LocV.Y);
		Loc->SetNumberField(TEXT("z"), LocV.Z);
		ActorObj->SetObjectField(TEXT("location"), Loc);

		TSharedPtr<FJsonObject> Rot = MakeShareable(new FJsonObject());
		FRotator Rotation = Actor->GetActorRotation();
		Rot->SetNumberField(TEXT("pitch"), Rotation.Pitch);
		Rot->SetNumberField(TEXT("yaw"), Rotation.Yaw);
		Rot->SetNumberField(TEXT("roll"), Rotation.Roll);
		ActorObj->SetObjectField(TEXT("rotation"), Rot);

		if (bIncludeTags)
		{
			TArray<TSharedPtr<FJsonValue>> TagsArray;
			for (const FName& Tag : Actor->Tags)
			{
				TagsArray.Add(MakeShareable(new FJsonValueString(Tag.ToString())));
			}
			ActorObj->SetArrayField(TEXT("tags"), TagsArray);
		}

		if (bIncludeComponents)
		{
			TArray<TSharedPtr<FJsonValue>> ComponentArray;
			TArray<UActorComponent*> Components = Actor->GetComponents().Array();
			for (UActorComponent* Component : Components)
			{
				if (!Component) continue;
				TSharedPtr<FJsonObject> ComponentObject = MakeShareable(new FJsonObject());
				ComponentObject->SetStringField(TEXT("name"), Component->GetName());
				if (Component->GetClass()) ComponentObject->SetStringField(TEXT("class"), Component->GetClass()->GetPathName());
				ComponentArray.Add(MakeShareable(new FJsonValueObject(ComponentObject)));
			}
			ActorObj->SetArrayField(TEXT("components"), ComponentArray);
		}

		ActorsArray.Add(MakeShareable(new FJsonValueObject(ActorObj)));
	}

	OutResult->SetArrayField(TEXT("actors"), ActorsArray);
	OutResult->SetBoolField(TEXT("success"), true);
	return true;
}

TSharedPtr<FJsonObject> FActorListTool::GetInputSchema()
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());

	TSharedPtr<FJsonObject> ClassNameProperty = MakeShareable(new FJsonObject());
	ClassNameProperty->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("className"), ClassNameProperty);

	TSharedPtr<FJsonObject> NameProperty = MakeShareable(new FJsonObject());
	NameProperty->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("name"), NameProperty);

	TSharedPtr<FJsonObject> IncludeComponentsProperty = MakeShareable(new FJsonObject());
	IncludeComponentsProperty->SetStringField(TEXT("type"), TEXT("boolean"));
	Properties->SetObjectField(TEXT("includeComponents"), IncludeComponentsProperty);

	TSharedPtr<FJsonObject> IncludeTagsProperty = MakeShareable(new FJsonObject());
	IncludeTagsProperty->SetStringField(TEXT("type"), TEXT("boolean"));
	Properties->SetObjectField(TEXT("includeTags"), IncludeTagsProperty);

	Schema->SetObjectField(TEXT("properties"), Properties);
	return Schema;
}