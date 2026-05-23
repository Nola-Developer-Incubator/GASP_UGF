#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GASP_StateTreeComponent.generated.h"

class AGASP_MoverCharacter;

USTRUCT(BlueprintType)
struct FGASPTrajectorySample
{
    GENERATED_BODY()

    UPROPERTY(BlueprintReadOnly, Category="GASP|Locomotion")
    FVector Position = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="GASP|Locomotion")
    FVector Velocity = FVector::ZeroVector;

    UPROPERTY(BlueprintReadOnly, Category="GASP|Locomotion")
    float TimeOffset = 0.f;
};

UCLASS(ClassGroup=(GASP), meta=(BlueprintSpawnableComponent))
class GASP_API UGASP_StateTreeComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGASP_StateTreeComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GASP|Locomotion")
    int32 TrajectoryHistorySize = 30;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="GASP|Locomotion")
    float TrajectorySampleInterval = 0.0333f;

    UFUNCTION(BlueprintCallable, Category="GASP|Locomotion")
    TArray<FGASPTrajectorySample> GetTrajectoryHistory() const;

    UFUNCTION(BlueprintCallable, Category="GASP|Locomotion")
    bool GetTrajectorySample(int32 Index, FGASPTrajectorySample& OutSample) const;

    UFUNCTION(BlueprintCallable, Category="GASP|Locomotion")
    bool GetPredictedTrajectorySample(float PredictionTime, FGASPTrajectorySample& OutSample) const;

    UFUNCTION(BlueprintCallable, Category="GASP|Locomotion")
    TArray<FGASPTrajectorySample> GetPredictedTrajectoryHistory(int32 NumSamples, float SecondsPerSample) const;

    UFUNCTION(BlueprintPure, Category="GASP|Locomotion")
    int32 GetTrajectoryHistoryCount() const;

    UFUNCTION(BlueprintCallable, Category="GASP|Locomotion")
    void ResetTrajectoryHistory();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
    float DecisionInterval = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
    float AggressiveIntentThreshold = 60.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
    float DefensiveIntentThreshold = 40.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
    float CandidateSearchDistance = 800.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="State")
    int32 CandidateCount = 8;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
    FGameplayTagContainer CurrentIntentTags;

    UFUNCTION(BlueprintCallable, Category="State")
    void EvaluateMindState();

    UFUNCTION(BlueprintCallable, Category="State")
    void ApplyCurrentIntentToPawn();

    UFUNCTION(BlueprintCallable, Category="State")
    void SetIntentTags(const FGameplayTagContainer& Tags);

    UFUNCTION(BlueprintCallable, Category="State")
    void ClearBrainDrivenIntent();

private:
    float TimeSinceLastDecision = 0.f;
    AGASP_MoverCharacter* OwningMoverPawn = nullptr;
    bool bHasBrainDrivenIntent = false;

    float TrajectorySampleAccumulator = 0.f;
    FVector LastTrajectoryLocation = FVector::ZeroVector;
    TArray<FGASPTrajectorySample> TrajectoryHistory;

    void CacheTrajectorySample(float DeltaTime);

    TArray<FVector> GenerateCandidatePositions() const;
    FVector ChooseTargetPosition(const TArray<FVector>& Candidates) const;
};

