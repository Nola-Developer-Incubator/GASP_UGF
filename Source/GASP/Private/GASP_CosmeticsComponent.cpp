#include "GASP_CosmeticsComponent.h"
#include "GameplayTagsManager.h"

#ifndef UE_ENABLE_FEATURE_POSESEARCH_V1
#define UE_ENABLE_FEATURE_POSESEARCH_V1 0
#endif

UGASP_CosmeticsComponent::UGASP_CosmeticsComponent()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGASP_CosmeticsComponent::RequestMutableUpdate(FName ParameterName, float ParameterValue)
{
    if (!ParameterName.IsNone())
    {
        SkinTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Skin.Cosmetic.MutableUpdate"), false));
        BroadcastMutableUpdate(ParameterName, ParameterValue);
    }
}

void UGASP_CosmeticsComponent::RequestMetaHumanRigUpdate(FName RigParameterName, float Value)
{
    if (!RigParameterName.IsNone())
    {
        SkinTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Skin.Skeletal.MetaHumanRig"), false));
        BroadcastMetaHumanRigUpdate(RigParameterName, Value);
    }
}

bool UGASP_CosmeticsComponent::IsPoseSearchEnabled() const
{
#if UE_ENABLE_FEATURE_POSESEARCH_V1
    return true;
#else
    return false;
#endif
}

void UGASP_CosmeticsComponent::BroadcastMutableUpdate(FName ParameterName, float ParameterValue)
{
    OnMutableUpdateRequested.Broadcast(ParameterName, ParameterValue);
}

void UGASP_CosmeticsComponent::BroadcastMetaHumanRigUpdate(FName RigParameterName, float Value)
{
    UE_LOG(LogTemp, Verbose, TEXT("MetaHuman rig update requested: %s = %f"), *RigParameterName.ToString(), Value);
    OnMetaHumanRigUpdateRequested.Broadcast(RigParameterName, Value);
}

void UGASP_CosmeticsComponent::RequestPoseSearchMatch(const FVector& DesiredDirection)
{
    if (IsPoseSearchEnabled())
    {
        OnPoseSearchRequested.Broadcast(DesiredDirection);
    }
}

void UGASP_CosmeticsComponent::NotifyDamageConfirmed()
{
    OnDamageConfirmed.Broadcast();
}

void UGASP_CosmeticsComponent::NotifyDamageWindowBegin()
{
    SkinTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Effect.Status.Stunned"), false));
    OnDamageWindowBegin.Broadcast();
}

void UGASP_CosmeticsComponent::NotifyDamageWindowEnd()
{
    SkinTags.RemoveTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Effect.Status.Stunned"), false));
    OnDamageWindowEnd.Broadcast();
}

