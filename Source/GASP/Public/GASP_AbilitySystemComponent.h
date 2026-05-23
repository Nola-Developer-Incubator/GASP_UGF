#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GASP_AbilitySystemComponent.generated.h"

class AGASP_MoverCharacter;
class UGASP_CombatComponent;
class UGASP_StateTreeComponent;
class UGASP_CosmeticsComponent;

UCLASS(ClassGroup=(GASP), meta=(BlueprintSpawnableComponent))
class GASP_API UGASP_AbilitySystemComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGASP_AbilitySystemComponent();

    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UFUNCTION(BlueprintCallable, Category="Ability")
    void ActivateAbility(const FGameplayTag& AbilityTag);

    UFUNCTION(BlueprintCallable, Category="Ability")
    void ApplyEffect(const FGameplayTag& EffectTag, float Duration);

    UFUNCTION(BlueprintCallable, Category="Ability")
    void ResetIntentOverride();

    UFUNCTION(BlueprintCallable, Category="Ability")
    void RequestDamageResponse(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category="Ability")
    void ForceEvaluateBrainState();

private:
    void EvaluateBrainState();
    void UpdateEffects(float DeltaTime);
    void PushIntentToMind();
    void ApplyAbilityCosts(const FGameplayTag& AbilityTag);
    void OnDamageTaken(float DamageAmount, float NewHealth);

    AGASP_MoverCharacter* OwningPawn = nullptr;
    UGASP_CombatComponent* CombatComponent = nullptr;
    UGASP_StateTreeComponent* MindComponent = nullptr;
    UGASP_CosmeticsComponent* SkinComponent = nullptr;

    FGameplayTagContainer ActiveAbilityTags;
    FGameplayTagContainer ActiveEffectTags;
    TMap<FGameplayTag, float> EffectDurations;
    FGameplayTagContainer CurrentIntentTags;
    float BrainDecisionInterval = 0.2f;
    float TimeSinceLastBrainDecision = 0.f;
};

