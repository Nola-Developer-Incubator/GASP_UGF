#include "AnimNotifyState_DamageWindow.h"
#include "Components/SkeletalMeshComponent.h"
#include "GASP_CosmeticsComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotifyState_DamageWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration);

    if (!MeshComp)
    {
        return;
    }

    AActor* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    UGASP_CosmeticsComponent* SkinComponent = OwnerActor->FindComponentByClass<UGASP_CosmeticsComponent>();
    if (SkinComponent)
    {
        SkinComponent->NotifyDamageWindowBegin();
    }
}

void UAnimNotifyState_DamageWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation)
{
    Super::NotifyEnd(MeshComp, Animation);

    if (!MeshComp)
    {
        return;
    }

    AActor* OwnerActor = MeshComp->GetOwner();
    if (!OwnerActor)
    {
        return;
    }

    UGASP_CosmeticsComponent* SkinComponent = OwnerActor->FindComponentByClass<UGASP_CosmeticsComponent>();
    if (SkinComponent)
    {
        SkinComponent->NotifyDamageWindowEnd();
    }
}

