#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "Components/SceneComponent.h"
#include "MoverComponent.h"
#include "MyMoverCharacter.generated.h"

UCLASS()
class GASP_API AMyMoverCharacter : public APawn
{
	GENERATED_BODY()

public:
	AMyMoverCharacter();

	// A root component is required for Mover to move the actor
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	USceneComponent* RootSceneComponent;

	// The core component that makes "Mover" work
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Movement")
	UMoverComponent* MoverComponent;
};

