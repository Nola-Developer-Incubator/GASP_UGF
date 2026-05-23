#include "GASP_AbilitySystemComponent.h"
#include "GASP_MoverCharacter.h"
#include "GASP_CombatComponent.h"
#include "GASP_StateTreeComponent.h"
#include "GASP_CosmeticsComponent.h"
#include "GameplayTagsManager.h"

UGASP_AbilitySystemComponent::UGASP_AbilitySystemComponent()
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UGASP_AbilitySystemComponent::BeginPlay()
{
    Super::BeginPlay();

    OwningPawn = Cast<AGASP_MoverCharacter>(GetOwner());
    if (!OwningPawn)
    {
        return;
    }

    CombatComponent = OwningPawn->CombatComponent;
    MindComponent = OwningPawn->MindComponent;
    SkinComponent = OwningPawn->SkinComponent;

    if (CombatComponent)
    {
        CombatComponent->OnDamageTaken.AddDynamic(this, &UGASP_AbilitySystemComponent::OnDamageTaken);
    }

    ForceEvaluateBrainState();
}

void UGASP_AbilitySystemComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    if (!OwningPawn || !CombatComponent)
    {
        return;
    }

    TimeSinceLastBrainDecision += DeltaTime;
    if (TimeSinceLastBrainDecision >= BrainDecisionInterval)
    {
        TimeSinceLastBrainDecision = 0.f;
        UpdateEffects(DeltaTime);
        EvaluateBrainState();
        PushIntentToMind();
    }
}

void UGASP_AbilitySystemComponent::ActivateAbility(const FGameplayTag& AbilityTag)
{
    if (!AbilityTag.IsValid())
    {
        return;
    }

    ActiveAbilityTags.AddTag(AbilityTag);
    ApplyAbilityCosts(AbilityTag);
}

void UGASP_AbilitySystemComponent::ApplyEffect(const FGameplayTag& EffectTag, float Duration)
{
    if (!EffectTag.IsValid() || Duration <= 0.f)
    {
        return;
    }

    ActiveEffectTags.AddTag(EffectTag);
    EffectDurations.FindOrAdd(EffectTag) = Duration;
}

void UGASP_AbilitySystemComponent::ResetIntentOverride()
{
    CurrentIntentTags.Reset();
    if (MindComponent)
    {
        MindComponent->ClearBrainDrivenIntent();
    }
}

void UGASP_AbilitySystemComponent::RequestDamageResponse(float DamageAmount)
{
    if (DamageAmount <= 0.f)
    {
        return;
    }

    if (SkinComponent)
    {
        SkinComponent->NotifyDamageConfirmed();
    }

    ActivateAbility(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Ability.Block"), false));
}

void UGASP_AbilitySystemComponent::ForceEvaluateBrainState()
{
    EvaluateBrainState();
    PushIntentToMind();
}

void UGASP_AbilitySystemComponent::EvaluateBrainState()
{
    CurrentIntentTags.Reset();

    if (!CombatComponent)
    {
        return;
    }

    const float HealthPercent = CombatComponent->Attributes.MaxHealth > 0.f
        ? CombatComponent->Health / CombatComponent->Attributes.MaxHealth
        : 1.f;
    const float StaminaPercent = CombatComponent->Attributes.MaxStamina > 0.f
        ? CombatComponent->Stamina / CombatComponent->Attributes.MaxStamina
        : 1.f;
    const bool bIsDowned = CombatComponent->CombatTags.HasTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Combat.State.Downed"), false));
    const bool bHasFullStamina = CombatComponent->CombatTags.HasTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Combat.Stamina.Full"), false));

    if (bIsDowned)
    {
        CurrentIntentTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Idle"), false));
        return;
    }

    if (HealthPercent < 0.25f)
    {
        CurrentIntentTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Retreat"), false));
        return;
    }

    if (StaminaPercent > 0.7f && bHasFullStamina)
    {
        CurrentIntentTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Attack"), false));
        ActivateAbility(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Ability.Rush"), false));
        return;
    }

    if (CombatComponent->Poise <= 0.f)
    {
        CurrentIntentTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Defensive"), false));
        return;
    }

    CurrentIntentTags.AddTag(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Intent.Search"), false));
}

void UGASP_AbilitySystemComponent::UpdateEffects(float DeltaTime)
{
    TArray<FGameplayTag> ExpiredEffects;
    for (auto& Pair : EffectDurations)
    {
        Pair.Value -= DeltaTime;
        if (Pair.Value <= 0.f)
        {
            ExpiredEffects.Add(Pair.Key);
        }
    }

    for (const FGameplayTag& ExpiredEffect : ExpiredEffects)
    {
        ActiveEffectTags.RemoveTag(ExpiredEffect);
        EffectDurations.Remove(ExpiredEffect);
    }
}

void UGASP_AbilitySystemComponent::PushIntentToMind()
{
    if (MindComponent)
    {
        MindComponent->SetIntentTags(CurrentIntentTags);
    }
}

void UGASP_AbilitySystemComponent::ApplyAbilityCosts(const FGameplayTag& AbilityTag)
{
    if (!CombatComponent)
    {
        return;
    }

    if (AbilityTag.MatchesTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Ability.Rush"), false)))
    {
        CombatComponent->RestoreStamina(-15.f);
    }
    else if (AbilityTag.MatchesTagExact(UGameplayTagsManager::Get().RequestGameplayTag(TEXT("Ability.Block"), false)))
    {
        CombatComponent->RestoreStamina(-5.f);
    }
}

void UGASP_AbilitySystemComponent::OnDamageTaken(float DamageAmount, float NewHealth)
{
    if (DamageAmount > 0.f)
    {
        RequestDamageResponse(DamageAmount);
    }
}

