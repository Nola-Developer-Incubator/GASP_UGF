#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "GASP_SurfaceAudioSettings.generated.h"

UCLASS(Config = Engine, DefaultConfig, meta = (DisplayName = "GASP: Surface Audio"))
class GASP_API UGASP_SurfaceAudioSettings : public UDeveloperSettings
{
    GENERATED_BODY()

public:
    UGASP_SurfaceAudioSettings();

    UPROPERTY(Config, EditAnywhere, Category = "Tracing", meta = (ToolTip = "Collision channel used to determine floor material types."))
    TEnumAsByte<ECollisionChannel> SurfaceTraceChannel = ECC_Visibility;

    UPROPERTY(Config, EditAnywhere, Category = "Tracing", meta = (ToolTip = "Maximum downward raycast length for footstep checks."))
    float MaxTraceDistance = 150.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Modulation", meta = (ClampMin = "0.0", ClampMax = "2.0", ToolTip = "Base volume multiplier applied to footstep impact sounds."))
    float BaseVolumeMultiplier = 1.0f;

    UPROPERTY(Config, EditAnywhere, Category = "Modulation", meta = (ClampMin = "0.0", ClampMax = "4.0", ToolTip = "Minimum volume multiplier applied when movement is very slow."))
    float MinVolumeMultiplier = 0.2f;

    UPROPERTY(Config, EditAnywhere, Category = "Modulation", meta = (ClampMin = "0.0", ClampMax = "4.0", ToolTip = "Maximum volume multiplier applied at high movement speeds."))
    float MaxVolumeMultiplier = 1.5f;

    UPROPERTY(Config, EditAnywhere, Category = "Modulation", meta = (ClampMin = "0.5", ClampMax = "2.0", ToolTip = "Minimum randomized pitch for footstep sounds."))
    float MinPitch = 0.85f;

    UPROPERTY(Config, EditAnywhere, Category = "Modulation", meta = (ClampMin = "0.5", ClampMax = "2.0", ToolTip = "Maximum randomized pitch for footstep sounds."))
    float MaxPitch = 1.15f;
};
