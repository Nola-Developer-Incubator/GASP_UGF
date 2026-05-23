#pragma once

#include "CoreMinimal.h"
#include "GASP_MotionMatchingTypes.generated.h"

USTRUCT(BlueprintType)
struct FGASPBoneTrackingMetric
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP|MotionMatching")
    FName BoneName = NAME_None;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP|MotionMatching")
    float TrackingWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP|MotionMatching")
    bool bTrackVelocity = true;

    FGASPBoneTrackingMetric() = default;

    FGASPBoneTrackingMetric(FName InName, float InWeight, bool bInTrackVelocity = true)
        : BoneName(InName)
        , TrackingWeight(InWeight)
        , bTrackVelocity(bInTrackVelocity)
    {}
};

USTRUCT(BlueprintType)
struct FGASPLocomotionSchemaProfile
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP|MotionMatching")
    float PositionWeight = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP|MotionMatching")
    float VelocityWeight = 1.5f;

    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP|MotionMatching")
    TArray<FGASPBoneTrackingMetric> CoreBones;

    FGASPLocomotionSchemaProfile()
    {
        CoreBones.Add(FGASPBoneTrackingMetric(TEXT("pelvis"), 1.0f, false));
        CoreBones.Add(FGASPBoneTrackingMetric(TEXT("foot_l"), 1.2f, true));
        CoreBones.Add(FGASPBoneTrackingMetric(TEXT("foot_r"), 1.2f, true));
    }
};
