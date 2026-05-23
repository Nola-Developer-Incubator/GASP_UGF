#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GameplayTagContainer.h"
#include "GASP_TacticalLibrary.generated.h"

UCLASS()
class GASP_API UGASP_TacticalLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintPure, Category="GASP|Tactical")
    static float EvaluateTacticalDistance(const FVector& From, const FVector& To);

    UFUNCTION(BlueprintPure, Category="GASP|Tactical")
    static float EvaluateCombatIntentScore(const FGameplayTagContainer& IntentTags, const FVector& From, const FVector& To);

    UFUNCTION(BlueprintCallable, Category="GASP|Tactical")
    static TArray<FVector> RankNavigationCandidates(const UObject* WorldContextObject, const FVector& Origin, const TArray<FVector>& Candidates, float MaxDistance = 2000.f);
};

