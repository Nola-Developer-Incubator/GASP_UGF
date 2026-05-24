#include "GASP_SurfaceAudioSubsystem.h"
#include "GASP_SurfaceAudioSettings.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundBase.h"
#include "GameFramework/Actor.h"

UGASP_SurfaceAudioSubsystem::UGASP_SurfaceAudioSubsystem()
{
    PrimaryComponentTick.bCanEverTick = false;
}

void UGASP_SurfaceAudioSubsystem::PlayImpactSound(EAudioSurfaceType Surface, FVector Location)
{
    USoundBase** SoundPtr = SurfaceImpactSounds.Find(Surface);
    if (!SoundPtr || !*SoundPtr)
    {
        SoundPtr = SurfaceImpactSounds.Find(EAudioSurfaceType::Default);
    }

    USoundBase* Sound = SoundPtr ? *SoundPtr : nullptr;
    if (!Sound || !GetWorld())
    {
        return;
    }

    const UGASP_SurfaceAudioSettings* Settings = GetDefault<UGASP_SurfaceAudioSettings>();
    const float Velocity = GetOwner() ? GetOwner()->GetVelocity().Size() : 0.f;
    const float BaseVolume = FMath::Clamp(Velocity / 600.0f, Settings->MinVolumeMultiplier, Settings->MaxVolumeMultiplier);
    const float Volume = FMath::Clamp(BaseVolume * Settings->BaseVolumeMultiplier, Settings->MinVolumeMultiplier, Settings->MaxVolumeMultiplier);
    const float Pitch = FMath::RandRange(Settings->MinPitch, Settings->MaxPitch);

    UGameplayStatics::PlaySoundAtLocation(this, Sound, Location, Volume, Pitch);
}
