#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "Sound/SoundBase.h"
#include "GASP_SurfaceAudioSubsystem.generated.h"

UENUM(BlueprintType)
enum class EAudioSurfaceType : uint8
{
    Default UMETA(DisplayName = "Default"),
    Concrete UMETA(DisplayName = "Concrete"),
    Wood UMETA(DisplayName = "Wood"),
    Metal UMETA(DisplayName = "Metal"),
    Grass UMETA(DisplayName = "Grass"),
    Water UMETA(DisplayName = "Water"),
    Mud UMETA(DisplayName = "Mud")
};

UCLASS(ClassGroup = (GASP), meta = (BlueprintSpawnableComponent))
class GASP_API UGASP_SurfaceAudioSubsystem : public UActorComponent
{
    GENERATED_BODY()

public:
    UGASP_SurfaceAudioSubsystem();

    UFUNCTION(BlueprintCallable, Category = "GASP|Audio")
    void PlayImpactSound(EAudioSurfaceType Surface, FVector Location);

protected:
    UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "GASP|Audio")
    TMap<EAudioSurfaceType, USoundBase*> SurfaceImpactSounds;
};
