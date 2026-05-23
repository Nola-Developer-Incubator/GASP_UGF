#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "GameplayTagContainer.h"
#include "I_GASPBodyInterface.generated.h"

USTRUCT(BlueprintType)
struct FCombatMoverInputCmd
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mover Input")
    float ForwardValue = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mover Input")
    float RightValue = 0.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mover Input")
    bool bJumpRequested = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mover Input")
    bool bSprintRequested = false;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Mover Input")
    FGameplayTagContainer IntentTags;
};

UINTERFACE(BlueprintType)
class GASP_API UGASPBodyInterface : public UInterface
{
    GENERATED_BODY()
};

class GASP_API IGASPBodyInterface
{
    GENERATED_BODY()

public:
    virtual void SubmitCombatMoverInput(const FCombatMoverInputCmd& InputCmd) = 0;
    virtual void TickBodySimulation(float DeltaSeconds) = 0;
    virtual void OnReceivedReplicatedBodyState() = 0;
    virtual FVector GetPredictedBodyLocation() const = 0;
};

