#include "GASP_TacticalLibrary.h"
#include "NavigationSystem.h"
#include "GameplayTagsManager.h"
#include "Engine/Engine.h"

float UGASP_TacticalLibrary::EvaluateTacticalDistance(const FVector& From, const FVector& To)
{
    return FVector::Dist(From, To);
}

float UGASP_TacticalLibrary::EvaluateCombatIntentScore(const FGameplayTagContainer& IntentTags, const FVector& From, const FVector& To)
{
    const float DistanceScore = FMath::Clamp(10000.f / (EvaluateTacticalDistance(From, To) + 1.f), 0.f, 100.f);
    const bool bHasAggressiveTag = IntentTags.HasTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Aggressive"), false));
    const bool bHasDefensiveTag = IntentTags.HasTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Defensive"), false));
    float IntentScore = 50.f;
    if (bHasAggressiveTag)
    {
        IntentScore += 20.f;
    }
    if (bHasDefensiveTag)
    {
        IntentScore -= 20.f;
    }
    return FMath::Clamp(DistanceScore + IntentScore, 0.f, 100.f);
}

TArray<FVector> UGASP_TacticalLibrary::RankNavigationCandidates(const UObject* WorldContextObject, const FVector& Origin, const TArray<FVector>& Candidates, float MaxDistance)
{
    TArray<FVector> SortedCandidates = Candidates;
    if (!WorldContextObject)
    {
        return SortedCandidates;
    }

    const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
    if (!World)
    {
        return SortedCandidates;
    }

    SortedCandidates.Sort([Origin, MaxDistance](const FVector& A, const FVector& B)
    {
        const float ScoreA = A.Equals(Origin) ? 0.f : FVector::DistSquared(Origin, A);
        const float ScoreB = B.Equals(Origin) ? 0.f : FVector::DistSquared(Origin, B);
        return ScoreA < ScoreB;
    });

    SortedCandidates.RemoveAll([Origin, MaxDistance](const FVector& Candidate)
    {
        return FVector::Dist(Origin, Candidate) > MaxDistance;
    });

    return SortedCandidates;
}

