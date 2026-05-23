#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "GASP_HUDWidget.generated.h"

UCLASS()
class GASP_API UGASP_HUDWidget : public UUserWidget
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="HUD")
    void SetAttributeValues(float Health, float MaxHealth, float Stamina, float MaxStamina, float Momentum, float MaxMomentum, float Poise, float MaxPoise);

    UFUNCTION(BlueprintImplementableEvent, Category="HUD")
    void OnAttributeValuesUpdated(float Health, float MaxHealth, float Stamina, float MaxStamina, float Momentum, float MaxMomentum, float Poise, float MaxPoise);
};

