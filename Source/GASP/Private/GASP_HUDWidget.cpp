#include "GASP_HUDWidget.h"

void UGASP_HUDWidget::SetAttributeValues(float Health, float MaxHealth, float Stamina, float MaxStamina, float Momentum, float MaxMomentum, float Poise, float MaxPoise)
{
    OnAttributeValuesUpdated(Health, MaxHealth, Stamina, MaxStamina, Momentum, MaxMomentum, Poise, MaxPoise);
}

