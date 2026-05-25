#include "AnimNotifyState_DamageWindow.h"
#include "Components/SkeletalMeshComponent.h"
#include "GASP_CosmeticsComponent.h"
#include "GameFramework/Actor.h"

void UAnimNotifyState_DamageWindow::NotifyBegin(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, float TotalDuration, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyBegin(MeshComp, Animation, TotalDuration, EventReference);

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

void UAnimNotifyState_DamageWindow::NotifyEnd(USkeletalMeshComponent* MeshComp, UAnimSequenceBase* Animation, const FAnimNotifyEventReference& EventReference)
{
    Super::NotifyEnd(MeshComp, Animation, EventReference);

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

