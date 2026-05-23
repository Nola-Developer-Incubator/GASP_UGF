#include "GASP_CombatComponent.h"
#include "GameplayTagsManager.h"
#include "GameplayTagContainer.h"

UGASP_CombatComponent::UGASP_CombatComponent()
{
    PrimaryComponentTick.bCanEverTick = false;

    Health = Attributes.MaxHealth;
    Stamina = Attributes.MaxStamina;
    Momentum = 0.f;
    Poise = Attributes.MaxPoise;

    SkinCosmeticMutableUpdateTag = RequestGameplayTagSafe(TEXT("Skin.Cosmetic.MutableUpdate"));
    SkinSkeletalMetaHumanRigTag = RequestGameplayTagSafe(TEXT("Skin.Skeletal.MetaHumanRig"));
}

void UGASP_CombatComponent::BeginPlay()
{
    Super::BeginPlay();

    ClampAttributes();
    RefreshCombatTags();
}

void UGASP_CombatComponent::ApplyDamage(float DamageAmount)
{
    if (DamageAmount <= 0.f || Health <= 0.f)
    {
        return;
    }

    Health = FMath::Clamp(Health - DamageAmount, 0.f, Attributes.MaxHealth);
    OnDamageTaken.Broadcast(DamageAmount, Health);
    RefreshCombatTags();
}

void UGASP_CombatComponent::RestoreStamina(float StaminaDelta)
{
    if (FMath::IsNearlyZero(StaminaDelta))
    {
        return;
    }

    Stamina = FMath::Clamp(Stamina + StaminaDelta, 0.f, Attributes.MaxStamina);
    OnStaminaChanged.Broadcast(StaminaDelta, Stamina);
    RefreshCombatTags();
}

void UGASP_CombatComponent::AddMomentum(float MomentumDelta)
{
    if (MomentumDelta <= 0.f)
    {
        return;
    }

    Momentum = FMath::Clamp(Momentum + MomentumDelta, 0.f, Attributes.MaxMomentum);
    RefreshCombatTags();
}

void UGASP_CombatComponent::ApplyPoiseDamage(float PoiseDamage)
{
    if (PoiseDamage <= 0.f)
    {
        return;
    }

    Poise = FMath::Clamp(Poise - PoiseDamage, 0.f, Attributes.MaxPoise);
    RefreshCombatTags();
}

bool UGASP_CombatComponent::IsAlive() const
{
    return Health > 0.f;
}

void UGASP_CombatComponent::ClampAttributes()
{
    Health = FMath::Clamp(Health, 0.f, Attributes.MaxHealth);
    Stamina = FMath::Clamp(Stamina, 0.f, Attributes.MaxStamina);
    Momentum = FMath::Clamp(Momentum, 0.f, Attributes.MaxMomentum);
    Poise = FMath::Clamp(Poise, 0.f, Attributes.MaxPoise);
}

void UGASP_CombatComponent::RefreshCombatTags()
{
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.State.Downed"), false));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.Poise.Broken"), false));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.Health.Low"), false));
    CombatTags.RemoveTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.Stamina.Full"), false));

    if (Health <= 0.f)
    {
        CombatTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.State.Downed"), false));
    }

    if (Health > 0.f && Health <= Attributes.MaxHealth * 0.25f)
    {
        CombatTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.Health.Low"), false));
    }

    if (Stamina >= Attributes.MaxStamina * 0.9f)
    {
        CombatTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.Stamina.Full"), false));
    }

    if (Poise <= 0.f)
    {
        CombatTags.AddTag(FGameplayTag::RequestGameplayTag(TEXT("Combat.Poise.Broken"), false));
    }

    if (SkinCosmeticMutableUpdateTag.IsValid())
    {
        CombatTags.AddTag(SkinCosmeticMutableUpdateTag);
    }

    if (SkinSkeletalMetaHumanRigTag.IsValid())
    {
        CombatTags.AddTag(SkinSkeletalMetaHumanRigTag);
    }
}

void UGASP_CombatComponent::RequestSkinCosmeticMutableUpdate(FName ParameterName, float ParameterValue)
{
    if (ParameterName.IsNone())
    {
        return;
    }

    const FGameplayTag SkinTag = RequestGameplayTagSafe(TEXT("Skin.Cosmetic.MutableUpdate"));
    ApplySkinTag(SkinTag);
    RefreshCombatTags();
}

void UGASP_CombatComponent::RequestSkinSkeletalMetaHumanRigUpdate(FName RigParameterName, float Value)
{
    if (RigParameterName.IsNone())
    {
        return;
    }

    const FGameplayTag SkinTag = RequestGameplayTagSafe(TEXT("Skin.Skeletal.MetaHumanRig"));
    ApplySkinTag(SkinTag);
    RefreshCombatTags();
}

void UGASP_CombatComponent::ApplySkinTag(const FGameplayTag& SkinTag)
{
    if (SkinTag.IsValid())
    {
        CombatTags.AddTag(SkinTag);
    }
}

FGameplayTag UGASP_CombatComponent::RequestGameplayTagSafe(const FName& TagName) const
{
    return UGameplayTagsManager::Get().RequestGameplayTag(TagName, false);
}

