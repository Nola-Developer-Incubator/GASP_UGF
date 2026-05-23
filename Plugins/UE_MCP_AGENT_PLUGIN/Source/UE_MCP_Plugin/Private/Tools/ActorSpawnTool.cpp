// Copyright © 2026 Nola Development Incubator. All Rights Reserved.

#include "Tools/ActorSpawnTool.h"
#include "Engine/World.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "UObject/UObjectGlobals.h"
#include "JsonObjectConverter.h"

bool FActorSpawnTool::Execute(TSharedPtr<FJsonObject> Params, TSharedPtr<FJsonObject>& OutResult)
{
	if (!Params.IsValid())
	{
		UE_LOG(LogTemp, Error, TEXT("ActorSpawnTool: Invalid parameters"));
		OutResult->SetBoolField(TEXT("success"), false);
		OutResult->SetStringField(TEXT("error"), TEXT("Invalid parameters"));
		return false;
	}

	// Extract parameters
	FString ClassName = Params->GetStringField(TEXT("className"));
	
	// Parse location
	FVector Location = FVector::ZeroVector;
	if (Params->HasField(TEXT("location")))
	{
		TSharedPtr<FJsonObject> LocationObj = Params->GetObjectField(TEXT("location"));
		Location.X = LocationObj->GetNumberField(TEXT("x"));
		Location.Y = LocationObj->GetNumberField(TEXT("y"));
		Location.Z = LocationObj->GetNumberField(TEXT("z"));
	}

	// Parse rotation
	FRotator Rotation = FRotator::ZeroRotator;
	if (Params->HasField(TEXT("rotation")))
	{
		TSharedPtr<FJsonObject> RotationObj = Params->GetObjectField(TEXT("rotation"));
		Rotation.Pitch = RotationObj->GetNumberField(TEXT("pitch"));
		Rotation.Yaw = RotationObj->GetNumberField(TEXT("yaw"));
		Rotation.Roll = RotationObj->GetNumberField(TEXT("roll"));
	}

	// Get the current world
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
		UE_LOG(LogTemp, Error, TEXT("ActorSpawnTool: No valid world found"));
		OutResult->SetBoolField(TEXT("success"), false);
		OutResult->SetStringField(TEXT("error"), TEXT("No valid world found"));
		return false;
	}

	// Load the actor class
	UClass* ActorClass = FindObject<UClass>(nullptr, *ClassName);
	if (!ActorClass)
	{
		// Try loading by path
		ActorClass = LoadObject<UClass>(nullptr, *ClassName);
	}

	if (!ActorClass)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorSpawnTool: Failed to find class '%s'"), *ClassName);
		OutResult->SetBoolField(TEXT("success"), false);
		OutResult->SetStringField(TEXT("error"), FString::Printf(TEXT("Class not found: %s"), *ClassName));
		return false;
	}

	// Spawn the actor
	FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;

	AActor* SpawnedActor = World->SpawnActor<AActor>(ActorClass, Location, Rotation, SpawnParams);

	if (!SpawnedActor)
	{
		UE_LOG(LogTemp, Error, TEXT("ActorSpawnTool: Failed to spawn actor of class '%s'"), *ClassName);
		OutResult->SetBoolField(TEXT("success"), false);
		OutResult->SetStringField(TEXT("error"), TEXT("Failed to spawn actor"));
		return false;
	}

	// Get actor GUID
	FString ActorGuid = SpawnedActor->GetActorGuid().ToString();

	// Build result
	OutResult->SetBoolField(TEXT("success"), true);
	OutResult->SetStringField(TEXT("actorGuid"), ActorGuid);
	OutResult->SetStringField(TEXT("actorName"), SpawnedActor->GetName());
	OutResult->SetStringField(TEXT("actorLabel"), SpawnedActor->GetActorLabel());

	// Add location info
	TSharedPtr<FJsonObject> LocationResult = MakeShareable(new FJsonObject());
	LocationResult->SetNumberField(TEXT("x"), SpawnedActor->GetActorLocation().X);
	LocationResult->SetNumberField(TEXT("y"), SpawnedActor->GetActorLocation().Y);
	LocationResult->SetNumberField(TEXT("z"), SpawnedActor->GetActorLocation().Z);
	OutResult->SetObjectField(TEXT("location"), LocationResult);

	UE_LOG(LogTemp, Log, TEXT("ActorSpawnTool: Successfully spawned actor '%s' (GUID: %s) at location (%s)"),
		*SpawnedActor->GetName(), *ActorGuid, *Location.ToString());

	return true;
}

TSharedPtr<FJsonObject> FActorSpawnTool::GetInputSchema()
{
	TSharedPtr<FJsonObject> Schema = MakeShareable(new FJsonObject());
	Schema->SetStringField(TEXT("type"), TEXT("object"));

	// Properties
	TSharedPtr<FJsonObject> Properties = MakeShareable(new FJsonObject());

	// className property
	TSharedPtr<FJsonObject> ClassNameProp = MakeShareable(new FJsonObject());
	ClassNameProp->SetStringField(TEXT("type"), TEXT("string"));
	ClassNameProp->SetStringField(TEXT("description"), TEXT("Fully qualified class name or path to actor class"));
	Properties->SetObjectField(TEXT("className"), ClassNameProp);

	// location property
	TSharedPtr<FJsonObject> LocationProp = MakeShareable(new FJsonObject());
	LocationProp->SetStringField(TEXT("type"), TEXT("object"));
	TSharedPtr<FJsonObject> LocationProps = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> XProp = MakeShareable(new FJsonObject());
	XProp->SetStringField(TEXT("type"), TEXT("number"));
	LocationProps->SetObjectField(TEXT("x"), XProp);
	
	TSharedPtr<FJsonObject> YProp = MakeShareable(new FJsonObject());
	YProp->SetStringField(TEXT("type"), TEXT("number"));
	LocationProps->SetObjectField(TEXT("y"), YProp);
	
	TSharedPtr<FJsonObject> ZProp = MakeShareable(new FJsonObject());
	ZProp->SetStringField(TEXT("type"), TEXT("number"));
	LocationProps->SetObjectField(TEXT("z"), ZProp);
	
	LocationProp->SetObjectField(TEXT("properties"), LocationProps);
	Properties->SetObjectField(TEXT("location"), LocationProp);

	// rotation property
	TSharedPtr<FJsonObject> RotationProp = MakeShareable(new FJsonObject());
	RotationProp->SetStringField(TEXT("type"), TEXT("object"));
	TSharedPtr<FJsonObject> RotationProps = MakeShareable(new FJsonObject());
	
	TSharedPtr<FJsonObject> PitchProp = MakeShareable(new FJsonObject());
	PitchProp->SetStringField(TEXT("type"), TEXT("number"));
	RotationProps->SetObjectField(TEXT("pitch"), PitchProp);
	
	TSharedPtr<FJsonObject> YawProp = MakeShareable(new FJsonObject());
	YawProp->SetStringField(TEXT("type"), TEXT("number"));
	RotationProps->SetObjectField(TEXT("yaw"), YawProp);
	
	TSharedPtr<FJsonObject> RollProp = MakeShareable(new FJsonObject());
	RollProp->SetStringField(TEXT("type"), TEXT("number"));
	RotationProps->SetObjectField(TEXT("roll"), RollProp);
	
	RotationProp->SetObjectField(TEXT("properties"), RotationProps);
	Properties->SetObjectField(TEXT("rotation"), RotationProp);

	Schema->SetObjectField(TEXT("properties"), Properties);

	// Required fields
	TArray<TSharedPtr<FJsonValue>> Required;
	Required.Add(MakeShareable(new FJsonValueString(TEXT("className"))));
	Schema->SetArrayField(TEXT("required"), Required);

	return Schema;
}
