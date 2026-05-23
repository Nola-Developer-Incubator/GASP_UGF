#include "GASP_ValidationLibrary.h"
#include "GASP_CombatComponent.h"
#include "GASP_MoverCharacter.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/Actor.h"
#include "Engine/Engine.h"

bool UGASP_ValidationLibrary::ValidateCombatComponent(UGASP_CombatComponent* CombatComponent, bool bLog)
{
    if (!CombatComponent)
    {
        if (bLog)
        {
            UE_LOG(LogTemp, Error, TEXT("GASP Validation failed: CombatComponent is null."));
        }
        return false;
    }

    const bool bOk = CombatComponent->Health >= 0.f && CombatComponent->Health <= CombatComponent->Attributes.MaxHealth &&
        CombatComponent->Stamina >= 0.f && CombatComponent->Stamina <= CombatComponent->Attributes.MaxStamina &&
        CombatComponent->Momentum >= 0.f && CombatComponent->Momentum <= CombatComponent->Attributes.MaxMomentum &&
        CombatComponent->Poise >= 0.f && CombatComponent->Poise <= CombatComponent->Attributes.MaxPoise;

    if (bLog)
    {
        UE_LOG(LogTemp, Log, TEXT("GASP CombatComponent validation: %s"), bOk ? TEXT("PASS") : TEXT("FAIL"));
    }

    return bOk;
}

bool UGASP_ValidationLibrary::ValidateMoverPawn(AGASP_MoverCharacter* Pawn, bool bLog)
{
    if (!Pawn)
    {
        if (bLog)
        {
            UE_LOG(LogTemp, Error, TEXT("GASP Validation failed: Pawn is null."));
        }
        return false;
    }

    const bool bOk = Pawn->MoverComponent != nullptr && Pawn->CombatComponent != nullptr && Pawn->GetRootComponent() != nullptr;
    if (bLog)
    {
        UE_LOG(LogTemp, Log, TEXT("GASP Pawn validation: %s"), bOk ? TEXT("PASS") : TEXT("FAIL"));
    }

    return bOk;
}

