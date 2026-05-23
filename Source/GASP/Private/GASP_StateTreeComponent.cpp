#include "GASP_StateTreeComponent.h"
#include "GASP_MoverCharacter.h"
#include "GASP_CombatComponent.h"
#include "GASP_TacticalLibrary.h"
#include "GameplayTagsManager.h"
#include "Kismet/GameplayStatics.h"
#include "MoverComponent.h"
#include "MoverSimulationTypes.h"
#include "MoveLibrary/MovementUtils.h"
#include "Engine/SkeletalMesh.h"
#include "Animation/Skeleton.h"

UGASP_StateTreeComponent::UGASP_StateTreeComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGASP_StateTreeComponent::BeginPlay()
{
    Super::BeginPlay();
    OwningMoverPawn = Cast<AGASP_MoverCharacter>(GetOwner());
    ResetTrajectoryHistory();
    EvaluateMindState();
}

void UGASP_StateTreeComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwningMoverPawn)
    {
        return;
    }

    CacheTrajectorySample(DeltaTime);

    TimeSinceLastDecision += DeltaTime;
    if (TimeSinceLastDecision >= DecisionInterval)
    {
        TimeSinceLastDecision = 0.f;
        EvaluateMindState();
        ApplyCurrentIntentToPawn();
    }
}

void UGASP_StateTreeComponent::SetIntentTags(const FGameplayTagContainer& Tags)
{
    CurrentIntentTags = Tags;
    bHasBrainDrivenIntent = true;
}

void UGASP_StateTreeComponent::ResetTrajectoryHistory()
{
    TrajectoryHistory.Empty();
    TrajectorySampleAccumulator = 0.f;
    LastTrajectoryLocation = OwningMoverPawn ? OwningMoverPawn->GetActorLocation() : FVector::ZeroVector;
}

TArray<FGASPTrajectorySample> UGASP_StateTreeComponent::GetTrajectoryHistory() const
{
    return TrajectoryHistory;
}

bool UGASP_StateTreeComponent::GetTrajectorySample(int32 Index, FGASPTrajectorySample& OutSample) const
{
    if (TrajectoryHistory.IsValidIndex(Index))
    {
        OutSample = TrajectoryHistory[Index];
        return true;
    }
    return false;
}

bool UGASP_StateTreeComponent::GetPredictedTrajectorySample(float PredictionTime, FGASPTrajectorySample& OutSample) const
{
    if (!OwningMoverPawn || !OwningMoverPawn->MoverComponent || PredictionTime <= 0.f)
    {
        return false;
    }

    FMoverPredictTrajectoryParams Params;
    Params.NumPredictionSamples = 2;
    Params.SecondsPerSample = PredictionTime;
    Params.bUseVisualComponentRoot = false;
    Params.bDisableGravity = false;

    const TArray<FTrajectorySampleInfo> Predicted = OwningMoverPawn->MoverComponent->GetPredictedTrajectory(Params);
    if (Predicted.Num() < 2)
    {
        return false;
    }

    const FTrajectorySampleInfo& Sample = Predicted[1];
    OutSample.Position = Sample.Transform.GetLocation();
    OutSample.Velocity = Sample.LinearVelocity;
    OutSample.TimeOffset = PredictionTime;
    return true;
}

TArray<FGASPTrajectorySample> UGASP_StateTreeComponent::GetPredictedTrajectoryHistory(int32 NumSamples, float SecondsPerSample) const
{
    TArray<FGASPTrajectorySample> Result;
    if (!OwningMoverPawn || !OwningMoverPawn->MoverComponent || NumSamples <= 0 || SecondsPerSample <= 0.f)
    {
        return Result;
    }

    FMoverPredictTrajectoryParams Params;
    Params.NumPredictionSamples = NumSamples;
    Params.SecondsPerSample = SecondsPerSample;
    Params.bUseVisualComponentRoot = false;
    Params.bDisableGravity = false;

    const TArray<FTrajectorySampleInfo> Predicted = OwningMoverPawn->MoverComponent->GetPredictedTrajectory(Params);
    Result.Reserve(Predicted.Num());

    for (int32 Index = 0; Index < Predicted.Num(); ++Index)
    {
        const FTrajectorySampleInfo& Sample = Predicted[Index];
        FGASPTrajectorySample TrajectorySample;
        TrajectorySample.Position = Sample.Transform.GetLocation();
        TrajectorySample.Velocity = Sample.LinearVelocity;
        TrajectorySample.TimeOffset = Index * SecondsPerSample;
        Result.Add(TrajectorySample);
    }

    return Result;
}

FGASPLocomotionSchemaProfile UGASP_StateTreeComponent::GetLocomotionSchemaProfile() const
{
    return FGASPLocomotionSchemaProfile();
}

bool UGASP_StateTreeComponent::ValidateMotionMatchingSkeleton(USkeletalMesh* SkeletalMesh, TArray<FName>& OutMissingBoneNames) const
{
    OutMissingBoneNames.Reset();
    if (!SkeletalMesh)
    {
        return false;
    }

    const USkeleton* Skeleton = SkeletalMesh->GetSkeleton();
    if (!Skeleton)
    {
        return false;
    }

    const FReferenceSkeleton& RefSkeleton = Skeleton->GetReferenceSkeleton();
    const FGASPLocomotionSchemaProfile Profile = GetLocomotionSchemaProfile();

    for (const FGASPBoneTrackingMetric& BoneMetric : Profile.CoreBones)
    {
        if (BoneMetric.BoneName.IsNone())
        {
            continue;
        }

        if (RefSkeleton.FindBoneIndex(BoneMetric.BoneName) == INDEX_NONE)
        {
            OutMissingBoneNames.Add(BoneMetric.BoneName);
        }
    }

    return OutMissingBoneNames.Num() == 0;
}

TArray<FName> UGASP_StateTreeComponent::GetLocomotionSchemaBoneNames() const
{
    TArray<FName> BoneNames;
    const FGASPLocomotionSchemaProfile Profile = GetLocomotionSchemaProfile();
    BoneNames.Reserve(Profile.CoreBones.Num());

    for (const FGASPBoneTrackingMetric& BoneMetric : Profile.CoreBones)
    {
        if (!BoneMetric.BoneName.IsNone())
        {
            BoneNames.Add(BoneMetric.BoneName);
        }
    }

    return BoneNames;
}

int32 UGASP_StateTreeComponent::GetTrajectoryHistoryCount() const
{
    return TrajectoryHistory.Num();
}

void UGASP_StateTreeComponent::CacheTrajectorySample(float DeltaTime)
{
    if (!OwningMoverPawn || TrajectorySampleInterval <= 0.f)
    {
        return;
    }

    TrajectorySampleAccumulator += DeltaTime;
    while (TrajectorySampleAccumulator >= TrajectorySampleInterval)
    {
        const FVector CurrentLocation = OwningMoverPawn->GetActorLocation();
        const FVector Velocity = OwningMoverPawn->MoverComponent
            ? OwningMoverPawn->MoverComponent->GetVelocity()
            : (TrajectoryHistory.Num() > 0 ? (CurrentLocation - LastTrajectoryLocation) / TrajectorySampleInterval : FVector::ZeroVector);

        for (FGASPTrajectorySample& Sample : TrajectoryHistory)
        {
            Sample.TimeOffset -= TrajectorySampleInterval;
        }

        FGASPTrajectorySample NewSample;
        NewSample.Position = CurrentLocation;
        NewSample.Velocity = Velocity;
        NewSample.TimeOffset = 0.f;
        TrajectoryHistory.Insert(NewSample, 0);

        if (TrajectoryHistory.Num() > TrajectoryHistorySize)
        {
            TrajectoryHistory.RemoveAt(TrajectoryHistorySize, TrajectoryHistory.Num() - TrajectoryHistorySize);
        }

        LastTrajectoryLocation = CurrentLocation;
        TrajectorySampleAccumulator -= TrajectorySampleInterval;
    }
}

void UGASP_StateTreeComponent::ClearBrainDrivenIntent()
{
    bHasBrainDrivenIntent = false;
}

void UGASP_StateTreeComponent::EvaluateMindState()
{
    if (bHasBrainDrivenIntent)
    {
        return;
    }

    CurrentIntentTags.Reset();

    if (!OwningMoverPawn || !OwningMoverPawn->CombatComponent)
    {
        return;
    }

    const float HealthPercent = OwningMoverPawn->CombatComponent->Attributes.MaxHealth > 0.f
        ? OwningMoverPawn->CombatComponent->Health / OwningMoverPawn->CombatComponent->Attributes.MaxHealth
        : 1.f;

    const float StaminaPercent = OwningMoverPawn->CombatComponent->Attributes.MaxStamina > 0.f
        ? OwningMoverPawn->CombatComponent->Stamina / OwningMoverPawn->CombatComponent->Attributes.MaxStamina
        : 1.f;

    const float MomentumPercent = OwningMoverPawn->CombatComponent->Attributes.MaxMomentum > 0.f
        ? OwningMoverPawn->CombatComponent->Momentum / OwningMoverPawn->CombatComponent->Attributes.MaxMomentum
        : 0.f;

    float IntentScore = 50.f;
    if (HealthPercent < 0.25f)
    {
        IntentScore -= 30.f;
    }

    if (StaminaPercent > 0.8f)
    {
        IntentScore += 10.f;
    }

    if (MomentumPercent > 0.6f)
    {
        IntentScore += 10.f;
    }

    if (OwningMoverPawn->CombatComponent->Poise <= 0.f)
    {
        IntentScore -= 20.f;
    }

    if (IntentScore >= AggressiveIntentThreshold)
    {
        CurrentIntentTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Aggressive"), false));
    }
    else if (IntentScore <= DefensiveIntentThreshold)
    {
        CurrentIntentTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Defensive"), false));
    }
    else
    {
        CurrentIntentTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Idle"), false));
    }
}

void UGASP_StateTreeComponent::ApplyCurrentIntentToPawn()
{
    if (!OwningMoverPawn)
    {
        return;
    }

    FCombatMoverInputCmd InputCmd;
    InputCmd.ForwardValue = 0.f;
    InputCmd.RightValue = 0.f;
    InputCmd.bJumpRequested = false;
    InputCmd.IntentTags = CurrentIntentTags;

    const FVector PawnLocation = OwningMoverPawn->GetActorLocation();
    const TArray<FVector> Candidates = GenerateCandidatePositions();
    const FVector Destination = ChooseTargetPosition(Candidates);
    const FVector DesiredMovement = (Destination - PawnLocation).GetSafeNormal2D();

    if (!DesiredMovement.IsNearlyZero())
    {
        InputCmd.ForwardValue = DesiredMovement.X;
        InputCmd.RightValue = DesiredMovement.Y;
    }

    InputCmd.bSprintRequested = CurrentIntentTags.HasTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Aggressive"), false));
    OwningMoverPawn->SubmitCombatMoverInput(InputCmd);
}

TArray<FVector> UGASP_StateTreeComponent::GenerateCandidatePositions() const
{
    TArray<FVector> Candidates;
    if (!OwningMoverPawn)
    {
        return Candidates;
    }

    const FVector Origin = OwningMoverPawn->GetActorLocation();
    const float AngleStep = 2.f * PI / FMath::Max(1, CandidateCount);

    for (int32 Index = 0; Index < CandidateCount; ++Index)
    {
        const float Angle = Index * AngleStep;
        const FVector Offset = FVector(FMath::Cos(Angle), FMath::Sin(Angle), 0.f) * CandidateSearchDistance;
        Candidates.Add(Origin + Offset);
    }

    return Candidates;
}

FVector UGASP_StateTreeComponent::ChooseTargetPosition(const TArray<FVector>& Candidates) const
{
    if (!OwningMoverPawn || Candidates.Num() == 0)
    {
        return OwningMoverPawn ? OwningMoverPawn->GetActorLocation() : FVector::ZeroVector;
    }

    TArray<FVector> Ranked = UGASP_TacticalLibrary::RankNavigationCandidates(OwningMoverPawn, OwningMoverPawn->GetActorLocation(), Candidates, CandidateSearchDistance);
    if (Ranked.Num() == 0)
    {
        return OwningMoverPawn->GetActorLocation();
    }

    if (CurrentIntentTags.HasTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Defensive"), false)))
    {
        return Ranked.Last();
    }

    if (CurrentIntentTags.HasTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Idle"), false)))
    {
        return OwningMoverPawn->GetActorLocation();
    }

    return Ranked[0];
}

