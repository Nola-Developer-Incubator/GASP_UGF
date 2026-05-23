#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "GameplayTagContainer.h"
#include "GASP_CombatComponent.generated.h"

USTRUCT(BlueprintType)
struct FGASPCombatAttributes
{
    GENERATED_BODY()

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attributes", meta=(ClampMin=0.0f))
    float MaxHealth = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attributes", meta=(ClampMin=0.0f))
    float MaxStamina = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attributes", meta=(ClampMin=0.0f))
    float MaxMomentum = 100.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attributes", meta=(ClampMin=0.0f))
    float MaxPoise = 100.f;
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatDamageEvent, float, DamageAmount, float, NewHealth);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCombatStaminaEvent, float, StaminaDelta, float, NewStamina);

UCLASS(ClassGroup=(GASP), meta=(BlueprintSpawnableComponent))
class GASP_API UGASP_CombatComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UGASP_CombatComponent();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Attributes")
    FGASPCombatAttributes Attributes;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|State")
    float Health;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|State")
    float Stamina;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|State")
    float Momentum;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat|State")
    float Poise;

    UPROPERTY(BlueprintAssignable, Category="Combat|Events")
    FCombatDamageEvent OnDamageTaken;

    UPROPERTY(BlueprintAssignable, Category="Combat|Events")
    FCombatStaminaEvent OnStaminaChanged;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Tags")
    FGameplayTagContainer CombatTags;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Tags")
    FGameplayTag SkinCosmeticMutableUpdateTag;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Combat|Tags")
    FGameplayTag SkinSkeletalMetaHumanRigTag;

    virtual void BeginPlay() override;

    UFUNCTION(BlueprintCallable, Category="Combat")
    void ApplyDamage(float DamageAmount);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void RestoreStamina(float StaminaDelta);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void AddMomentum(float MomentumDelta);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void ApplyPoiseDamage(float PoiseDamage);

    UFUNCTION(BlueprintCallable, Category="Combat")
    bool IsAlive() const;

    UFUNCTION(BlueprintCallable, Category="Combat")
    void RequestSkinCosmeticMutableUpdate(FName ParameterName, float ParameterValue);

    UFUNCTION(BlueprintCallable, Category="Combat")
    void RequestSkinSkeletalMetaHumanRigUpdate(FName RigParameterName, float Value);

    UFUNCTION(BlueprintCallable, Category="Combat")
    FGameplayTagContainer GetCombatTags() const { return CombatTags; }

private:
    void ClampAttributes();
    void RefreshCombatTags();
    void ApplySkinTag(const FGameplayTag& SkinTag);
    FGameplayTag RequestGameplayTagSafe(const FName& TagName) const;
};

