#include "GASP_MoverCharacter.h"
#include "AbilitySystemComponent.h"
#include "Abilities/GameplayAbility.h"
#include "GameplayAbilitySpec.h"
#include "MoverSimulationTypes.h"
#include "Net/UnrealNetwork.h"
#include "Blueprint/UserWidget.h"
#include "GASP_CosmeticsComponent.h"
#include "GASP_AbilitySystemComponent.h"
#include "GASP_HUDWidget.h"
#include "GASP_SurfaceAudioSubsystem.h"
#include "DefaultMovementSet/CharacterMoverComponent.h"

AGASP_MoverCharacter::AGASP_MoverCharacter()
{
    PrimaryActorTick.bCanEverTick = true;
    bReplicates = true;
    SetReplicateMovement(false);

    RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
    RootComponent = RootSceneComponent;

    MoverComponent = CreateDefaultSubobject<UMoverComponent>(TEXT("MoverComponent"));
    CombatComponent = CreateDefaultSubobject<UGASP_CombatComponent>(TEXT("GASPCombatComponent"));
    SkinComponent = CreateDefaultSubobject<UGASP_CosmeticsComponent>(TEXT("GASPSkinComponent"));
    SurfaceAudioComponent = CreateDefaultSubobject<UGASP_SurfaceAudioSubsystem>(TEXT("GASPSurfaceAudioSubsystem"));
    MindComponent = CreateDefaultSubobject<UGASP_StateTreeComponent>(TEXT("GASPMindComponent"));
    BrainComponent = CreateDefaultSubobject<UGASP_AbilitySystemComponent>(TEXT("GASPBrainComponent"));
    AbilitySystemComponent = CreateDefaultSubobject<UAbilitySystemComponent>(TEXT("AbilitySystemComponent"));
    if (AbilitySystemComponent)
    {
        AbilitySystemComponent->SetIsReplicated(true);
        AbilitySystemComponent->SetReplicationMode(EGameplayEffectReplicationMode::Minimal);
    }

    if (MoverComponent && GetClass()->ImplementsInterface(UMoverInputProducerInterface::StaticClass()))
    {
        MoverComponent->InputProducer = this;
    }
}

void AGASP_MoverCharacter::PostInitializeComponents()
{
    Super::PostInitializeComponents();
    EnsureCharacterMoverComponent();
}

void AGASP_MoverCharacter::BeginPlay()
{
    Super::BeginPlay();
    EnsureCharacterMoverComponent();

    if (SkinComponent && CombatComponent)
    {
        SkinComponent->OnMutableUpdateRequested.AddDynamic(this, &AGASP_MoverCharacter::HandleSkinMutableUpdate);
        SkinComponent->OnMetaHumanRigUpdateRequested.AddDynamic(this, &AGASP_MoverCharacter::HandleMetaHumanRigUpdate);
    }

    if (CombatComponent)
    {
        CombatComponent->OnDamageTaken.AddDynamic(this, &AGASP_MoverCharacter::HandleCombatDamage);
        CombatComponent->OnStaminaChanged.AddDynamic(this, &AGASP_MoverCharacter::HandleStaminaChanged);
    }

    if (IsLocallyControlled() && HUDWidgetClass)
    {
        if (UWorld* World = GetWorld())
        {
            HUDWidget = CreateWidget<UGASP_HUDWidget>(World, HUDWidgetClass);
            if (HUDWidget)
            {
                HUDWidget->AddToViewport();
            }
        }
    }

    UpdateHUDAttributes();
}

void AGASP_MoverCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (!MoverComponent || !Cast<UCharacterMoverComponent>(MoverComponent))
    {
        TickBodySimulation(DeltaSeconds);
    }
}

void AGASP_MoverCharacter::EnsureCharacterMoverComponent()
{
    if (!GetClass()->ImplementsInterface(UMoverInputProducerInterface::StaticClass()))
    {
        return;
    }

    if (UCharacterMoverComponent* CharacterMoverComp = FindComponentByClass<UCharacterMoverComponent>())
    {
        if (CharacterMoverComp != MoverComponent)
        {
            if (MoverComponent)
            {
                MoverComponent->Deactivate();
                MoverComponent->SetComponentTickEnabled(false);
                MoverComponent->InputProducer = nullptr;
            }
            MoverComponent = CharacterMoverComp;
        }

        if (!MoverComponent->InputProducer)
        {
            MoverComponent->InputProducer = this;
        }

        if (!CharacterMoverComp->GetUpdatedComponent())
        {
            if (USceneComponent* UpdatedComponent = GetRootComponent())
            {
                CharacterMoverComp->SetUpdatedComponent(UpdatedComponent);
            }
        }
    }
}

void AGASP_MoverCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    if (!PlayerInputComponent)
    {
        return;
    }

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AGASP_MoverCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AGASP_MoverCharacter::MoveRight);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Pressed, this, &AGASP_MoverCharacter::StartSprint);
    PlayerInputComponent->BindAction(TEXT("Sprint"), IE_Released, this, &AGASP_MoverCharacter::StopSprint);
    PlayerInputComponent->BindAction(TEXT("Jump"), IE_Pressed, this, &AGASP_MoverCharacter::RequestJump);
}

void AGASP_MoverCharacter::SubmitCombatMoverInput(const FCombatMoverInputCmd& InputCmd)
{
    LatestMoverInputCmd = InputCmd;
    if (!HasAuthority())
    {
        ServerSubmitCombatMoverInput(InputCmd);
        return;
    }

    OnReceivedReplicatedBodyState();
}

void AGASP_MoverCharacter::ServerSubmitCombatMoverInput_Implementation(const FCombatMoverInputCmd& InputCmd)
{
    LatestMoverInputCmd = InputCmd;
    OnReceivedReplicatedBodyState();
}

void AGASP_MoverCharacter::MoveForward(float Value)
{
    LatestMoverInputCmd.ForwardValue = Value;
    SubmitCombatMoverInput(LatestMoverInputCmd);
}

void AGASP_MoverCharacter::MoveRight(float Value)
{
    LatestMoverInputCmd.RightValue = Value;
    SubmitCombatMoverInput(LatestMoverInputCmd);
}

void AGASP_MoverCharacter::StartSprint()
{
    LatestMoverInputCmd.bSprintRequested = true;
    SubmitCombatMoverInput(LatestMoverInputCmd);
}

void AGASP_MoverCharacter::StopSprint()
{
    LatestMoverInputCmd.bSprintRequested = false;
    SubmitCombatMoverInput(LatestMoverInputCmd);
}

void AGASP_MoverCharacter::RequestJump()
{
    LatestMoverInputCmd.bJumpRequested = true;
    SubmitCombatMoverInput(LatestMoverInputCmd);
}

void AGASP_MoverCharacter::TickBodySimulation(float DeltaSeconds)
{
    if (!RootComponent)
    {
        return;
    }

    const FVector DesiredMovement = FVector(LatestMoverInputCmd.ForwardValue, LatestMoverInputCmd.RightValue, 0.f);
    const float Speed = LatestMoverInputCmd.bSprintRequested ? 1200.f : 600.f;
    const FVector DeltaMovement = DesiredMovement.GetClampedToMaxSize(1.0f) * Speed * DeltaSeconds;

    if (!DeltaMovement.IsNearlyZero())
    {
        RootComponent->AddWorldOffset(DeltaMovement, true);
    }
}

void AGASP_MoverCharacter::OnReceivedReplicatedBodyState()
{
    // This hook can be extended to reconcile authoritative simulation state when replication arrives.
}

void AGASP_MoverCharacter::HandleSkinMutableUpdate(FName ParameterName, float ParameterValue)
{
    if (CombatComponent)
    {
        CombatComponent->RequestSkinCosmeticMutableUpdate(ParameterName, ParameterValue);
    }
}

void AGASP_MoverCharacter::HandleMetaHumanRigUpdate(FName RigParameterName, float Value)
{
    if (CombatComponent)
    {
        CombatComponent->RequestSkinSkeletalMetaHumanRigUpdate(RigParameterName, Value);
    }
}

UAbilitySystemComponent* AGASP_MoverCharacter::GetAbilitySystemComponent() const
{
    return AbilitySystemComponent;
}

void AGASP_MoverCharacter::PlayFootstepImpactAtLocation(const FVector& Location, EAudioSurfaceType SurfaceType)
{
    if (SurfaceAudioComponent)
    {
        SurfaceAudioComponent->PlayImpactSound(SurfaceType, Location);
    }
}

void AGASP_MoverCharacter::PossessedBy(AController* NewController)
{
    Super::PossessedBy(NewController);
    InitializeGameplayAbilities();
}

void AGASP_MoverCharacter::OnRep_PlayerState()
{
    Super::OnRep_PlayerState();
    InitializeGameplayAbilities();
}

void AGASP_MoverCharacter::InitializeGameplayAbilities()
{
    if (!AbilitySystemComponent)
    {
        return;
    }

    AbilitySystemComponent->InitAbilityActorInfo(this, this);

    if (!HasAuthority())
    {
        return;
    }

    for (TSubclassOf<UGameplayAbility> AbilityClass : DefaultAbilities)
    {
        if (!AbilityClass)
        {
            continue;
        }

        if (!AbilitySystemComponent->FindAbilitySpecFromClass(AbilityClass))
        {
            AbilitySystemComponent->GiveAbility(FGameplayAbilitySpec(AbilityClass, 1, INDEX_NONE, this));
        }
    }
}

FVector AGASP_MoverCharacter::GetPredictedBodyLocation() const
{
    return RootComponent ? RootComponent->GetComponentLocation() : FVector::ZeroVector;
}

void AGASP_MoverCharacter::OnRep_LatestMoverInputCmd()
{
    OnReceivedReplicatedBodyState();
}

void AGASP_MoverCharacter::HandleCombatDamage(float DamageAmount, float NewHealth)
{
    UpdateHUDAttributes();
}

void AGASP_MoverCharacter::HandleStaminaChanged(float StaminaDelta, float NewStamina)
{
    UpdateHUDAttributes();
}

void AGASP_MoverCharacter::UpdateHUDAttributes()
{
    if (!HUDWidget || !CombatComponent)
    {
        return;
    }

    HUDWidget->SetAttributeValues(
        CombatComponent->Health,
        CombatComponent->Attributes.MaxHealth,
        CombatComponent->Stamina,
        CombatComponent->Attributes.MaxStamina,
        CombatComponent->Momentum,
        CombatComponent->Attributes.MaxMomentum,
        CombatComponent->Poise,
        CombatComponent->Attributes.MaxPoise);
}

void AGASP_MoverCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
    Super::GetLifetimeReplicatedProps(OutLifetimeProps);
    DOREPLIFETIME(AGASP_MoverCharacter, LatestMoverInputCmd);
}

