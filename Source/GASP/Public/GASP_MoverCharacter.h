#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "I_GASPBodyInterface.h"
#include "AbilitySystemInterface.h"
#include "MoverComponent.h"
#include "GASP_CombatComponent.h"
#include "GASP_CosmeticsComponent.h"
#include "GASP_StateTreeComponent.h"
#include "GASP_AbilitySystemComponent.h"
#include "GASP_SurfaceAudioSubsystem.h"
#include "GASP_MoverCharacter.generated.h"

class UGASP_HUDWidget;
class UAbilitySystemComponent;
class UGameplayAbility;
class UGASP_SurfaceAudioSubsystem;

/**
 * AGASP_MoverCharacter is the core pawn for mover-driven movement and gameplay.
 * It owns the custom mover component, GAS ability system, and the high-level
 * GASP_* support components for combat, state, ability, and cosmetics.
 *
 * Keep movement responsibilities on CharacterMoverComponent/UMoverComponent and
 * use GAS for ability/effect state, tags, and gameplay-driven responses.
 */
UCLASS(Blueprintable)
class GASP_API AGASP_MoverCharacter : public APawn, public IGASPBodyInterface, public IAbilitySystemInterface
{
    GENERATED_BODY()

public:
    AGASP_MoverCharacter();

    virtual void BeginPlay() override;
    virtual void PostInitializeComponents() override;
    virtual void Tick(float DeltaSeconds) override;
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

    virtual void SubmitCombatMoverInput(const FCombatMoverInputCmd& InputCmd) override;
    virtual void TickBodySimulation(float DeltaSeconds) override;
    virtual void OnReceivedReplicatedBodyState() override;
    virtual FVector GetPredictedBodyLocation() const override;
    UFUNCTION(Server, Reliable)
    void ServerSubmitCombatMoverInput(const FCombatMoverInputCmd& InputCmd);

    void MoveForward(float Value);
    void MoveRight(float Value);
    void StartSprint();
    void StopSprint();
    void RequestJump();
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Body")
    USceneComponent* RootSceneComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Body")
    UMoverComponent* MoverComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Combat")
    UGASP_CombatComponent* CombatComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Cosmetics")
    UGASP_CosmeticsComponent* SkinComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Audio")
    UGASP_SurfaceAudioSubsystem* SurfaceAudioComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="State")
    UGASP_StateTreeComponent* MindComponent;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Ability")
    UGASP_AbilitySystemComponent* BrainComponent;

    /**
     * Gameplay Ability System Component that owns abilities, effects and tags.
     * This combines with the mover system to keep movement separate from gameplay behavior.
     */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Abilities")
    UAbilitySystemComponent* AbilitySystemComponent;

    /**
     * Default gameplay abilities to grant to this pawn when play begins.
     * Add your gameplay ability Blueprint classes here in the Pawn Blueprint.
     */
    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Abilities")
    TArray<TSubclassOf<UGameplayAbility>> DefaultAbilities;

    UFUNCTION(BlueprintCallable, Category="Abilities")
    UAbilitySystemComponent* GetAbilitySystemComponent() const override;

    UFUNCTION(BlueprintCallable, Category="Audio")
    void PlayFootstepImpactAtLocation(const FVector& Location, EAudioSurfaceType SurfaceType);

    virtual void PossessedBy(AController* NewController) override;
    virtual void OnRep_PlayerState() override;

    void InitializeGameplayAbilities();

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="UI")
    TSubclassOf<UGASP_HUDWidget> HUDWidgetClass;

    UPROPERTY(Transient, BlueprintReadOnly, Category="UI")
    UGASP_HUDWidget* HUDWidget;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, ReplicatedUsing=OnRep_LatestMoverInputCmd, Category="Body")
    FCombatMoverInputCmd LatestMoverInputCmd;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body|Physics")
    float MaxStepHeight = 50.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body|Physics")
    float MaxAcceleration = 12000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body|Physics")
    float MaxDeceleration = 9000.f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Body|Replication")
    float ReplicationBoundaryRadius = 5000.f;

private:
    UFUNCTION()
    void OnRep_LatestMoverInputCmd();

    UFUNCTION()
    void HandleSkinMutableUpdate(FName ParameterName, float ParameterValue);

    UFUNCTION()
    void HandleMetaHumanRigUpdate(FName RigParameterName, float Value);

    UFUNCTION()
    void HandleCombatDamage(float DamageAmount, float NewHealth);

    UFUNCTION()
    void HandleStaminaChanged(float StaminaDelta, float NewStamina);

    void UpdateHUDAttributes();

    void EnsureCharacterMoverComponent();

    FCombatMoverInputCmd PendingMoverInputCmd;
};

