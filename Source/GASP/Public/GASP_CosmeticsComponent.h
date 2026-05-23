#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GASP_CosmeticsComponent.generated.h"

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMutableUpdateRequested, FName, ParameterName, float, ParameterValue);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FMetaHumanRigUpdateRequested, FName, RigParameterName, float, Value);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FPoseSearchRequested, FVector, DesiredDirection);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDamageConfirmed);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDamageWindowBegin);

UDELEGATE(BlueprintAuthorityOnly)
DECLARE_DYNAMIC_MULTICAST_DELEGATE(FDamageWindowEnd);

UCLASS(ClassGroup=(GASP), meta=(BlueprintSpawnableComponent))
class GASP_API UGASP_CosmeticsComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGASP_CosmeticsComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Skin|Tags")
    FGameplayTagContainer SkinTags;

    UPROPERTY(BlueprintAssignable, Category="Skin|Events")
    FMutableUpdateRequested OnMutableUpdateRequested;

    UPROPERTY(BlueprintAssignable, Category="Skin|Events")
    FMetaHumanRigUpdateRequested OnMetaHumanRigUpdateRequested;

    UPROPERTY(BlueprintAssignable, Category="Skin|Events")
    FPoseSearchRequested OnPoseSearchRequested;

    UPROPERTY(BlueprintAssignable, Category="Skin|Events")
    FDamageConfirmed OnDamageConfirmed;

    UPROPERTY(BlueprintAssignable, Category="Skin|Events")
    FDamageWindowBegin OnDamageWindowBegin;

    UPROPERTY(BlueprintAssignable, Category="Skin|Events")
    FDamageWindowEnd OnDamageWindowEnd;

    UFUNCTION(BlueprintCallable, Category="Cosmetics")
    void RequestMutableUpdate(FName ParameterName, float ParameterValue);

    UFUNCTION(BlueprintCallable, Category="Cosmetics")
    void RequestMetaHumanRigUpdate(FName RigParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category="Cosmetics")
    void RequestPoseSearchMatch(const FVector& DesiredDirection);

    UFUNCTION(BlueprintCallable, Category="Cosmetics")
    void NotifyDamageConfirmed();

    UFUNCTION(BlueprintCallable, Category="Cosmetics")
    void NotifyDamageWindowBegin();

    UFUNCTION(BlueprintCallable, Category="Cosmetics")
    void NotifyDamageWindowEnd();

    UFUNCTION(BlueprintPure, Category="Cosmetics")
    bool IsPoseSearchEnabled() const;

private:
    void BroadcastMutableUpdate(FName ParameterName, float ParameterValue);
    void BroadcastMetaHumanRigUpdate(FName RigParameterName, float Value);
};

