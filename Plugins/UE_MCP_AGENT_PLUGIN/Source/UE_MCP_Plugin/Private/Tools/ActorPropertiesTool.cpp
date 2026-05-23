#include "Tools/ActorPropertiesTool.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "GameFramework/Actor.h"

static AActor* FindActorByGuidOrNameForProps(UWorld* World, const FString& GuidOrName)
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

bool FActorPropertiesTool::ExecuteGet(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
	OutResult->SetBoolField(TEXT("success"), false);

	if (!Params.IsValid() || !Params->HasField(TEXT("query")))
	{
		OutResult->SetStringField(TEXT("error"), TEXT("query is required"));
		return false;
	}

	FString Query = Params->GetStringField(TEXT("query"));

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

	AActor* Actor = FindActorByGuidOrNameForProps(World, Query);
	if (!Actor)
	{
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

	TSharedPtr<FJsonObject> Rotation = MakeShareable(new FJsonObject());
	FRotator ActorRotation = Actor->GetActorRotation();
	Rotation->SetNumberField(TEXT("pitch"), ActorRotation.Pitch);
	Rotation->SetNumberField(TEXT("yaw"), ActorRotation.Yaw);
	Rotation->SetNumberField(TEXT("roll"), ActorRotation.Roll);
	ActorObject->SetObjectField(TEXT("rotation"), Rotation);

	TArray<TSharedPtr<FJsonValue>> TagsArray;
	for (const FName& Tag : Actor->Tags)
	{
		TagsArray.Add(MakeShareable(new FJsonValueString(Tag.ToString())));
	}
	ActorObject->SetArrayField(TEXT("tags"), TagsArray);

	OutResult->SetObjectField(TEXT("actor"), ActorObject);
	OutResult->SetBoolField(TEXT("success"), true);
	return true;
}

bool FActorPropertiesTool::ExecuteSet(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
	OutResult->SetBoolField(TEXT("success"), false);

	if (!Params.IsValid() || !Params->HasField(TEXT("query")))
	{
		OutResult->SetStringField(TEXT("error"), TEXT("query is required"));
		return false;
	}

	FString Query = Params->GetStringField(TEXT("query"));

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

	AActor* Actor = FindActorByGuidOrNameForProps(World, Query);
	if (!Actor)
	{
		OutResult->SetStringField(TEXT("error"), TEXT("Actor not found"));
		return false;
	}

	if (Params->HasField(TEXT("location")))
	{
		TSharedPtr<FJsonObject> Location = Params->GetObjectField(TEXT("location"));
		FVector NewLocation = Actor->GetActorLocation();
		if (Location->HasField(TEXT("x"))) NewLocation.X = Location->GetNumberField(TEXT("x"));
		if (Location->HasField(TEXT("y"))) NewLocation.Y = Location->GetNumberField(TEXT("y"));
		if (Location->HasField(TEXT("z"))) NewLocation.Z = Location->GetNumberField(TEXT("z"));
		Actor->SetActorLocation(NewLocation);
	}

	if (Params->HasField(TEXT("rotation")))
	{
		TSharedPtr<FJsonObject> Rotation = Params->GetObjectField(TEXT("rotation"));
		FRotator NewRotation = Actor->GetActorRotation();
		if (Rotation->HasField(TEXT("pitch"))) NewRotation.Pitch = Rotation->GetNumberField(TEXT("pitch"));
		if (Rotation->HasField(TEXT("yaw"))) NewRotation.Yaw = Rotation->GetNumberField(TEXT("yaw"));
		if (Rotation->HasField(TEXT("roll"))) NewRotation.Roll = Rotation->GetNumberField(TEXT("roll"));
		Actor->SetActorRotation(NewRotation);
	}

	if (Params->HasField(TEXT("label")))
	{
		FString NewLabel = Params->GetStringField(TEXT("label"));
		Actor->SetActorLabel(NewLabel);
	}

	OutResult->SetBoolField(TEXT("success"), true);
	return true;
}

TSharedPtr<FJsonObject> FActorPropertiesTool::GetInputSchemaGet()
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

TSharedPtr<FJsonObject> FActorPropertiesTool::GetInputSchemaSet()
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());

	TSharedPtr<FJsonObject> QueryProperty = MakeShareable(new FJsonObject());
	QueryProperty->SetStringField(TEXT("type"), TEXT("string"));
	QueryProperty->SetStringField(TEXT("description"), TEXT("Actor name, label, or GUID"));
	Properties->SetObjectField(TEXT("query"), QueryProperty);

	TSharedPtr<FJsonObject> Location = MakeShareable(new FJsonObject());
	Location->SetStringField(TEXT("type"), TEXT("object"));
	TSharedPtr<FJsonObject> LocationProperties = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> X = MakeShareable(new FJsonObject()); X->SetStringField(TEXT("type"), TEXT("number")); LocationProperties->SetObjectField(TEXT("x"), X);
	TSharedPtr<FJsonObject> Y = MakeShareable(new FJsonObject()); Y->SetStringField(TEXT("type"), TEXT("number")); LocationProperties->SetObjectField(TEXT("y"), Y);
	TSharedPtr<FJsonObject> Z = MakeShareable(new FJsonObject()); Z->SetStringField(TEXT("type"), TEXT("number")); LocationProperties->SetObjectField(TEXT("z"), Z);
	Location->SetObjectField(TEXT("properties"), LocationProperties);
	Properties->SetObjectField(TEXT("location"), Location);

	TSharedPtr<FJsonObject> Rotation = MakeShareable(new FJsonObject());
	Rotation->SetStringField(TEXT("type"), TEXT("object"));
	TSharedPtr<FJsonObject> RotationProperties = MakeShareable(new FJsonObject());
	TSharedPtr<FJsonObject> Pitch = MakeShareable(new FJsonObject()); Pitch->SetStringField(TEXT("type"), TEXT("number")); RotationProperties->SetObjectField(TEXT("pitch"), Pitch);
	TSharedPtr<FJsonObject> Yaw = MakeShareable(new FJsonObject()); Yaw->SetStringField(TEXT("type"), TEXT("number")); RotationProperties->SetObjectField(TEXT("yaw"), Yaw);
	TSharedPtr<FJsonObject> Roll = MakeShareable(new FJsonObject()); Roll->SetStringField(TEXT("type"), TEXT("number")); RotationProperties->SetObjectField(TEXT("roll"), Roll);
	Rotation->SetObjectField(TEXT("properties"), RotationProperties);
	Properties->SetObjectField(TEXT("rotation"), Rotation);

	TSharedPtr<FJsonObject> LabelProperty = MakeShareable(new FJsonObject());
	LabelProperty->SetStringField(TEXT("type"), TEXT("string"));
	Properties->SetObjectField(TEXT("label"), LabelProperty);

	Schema->SetObjectField(TEXT("properties"), Properties);
	return Schema;
}