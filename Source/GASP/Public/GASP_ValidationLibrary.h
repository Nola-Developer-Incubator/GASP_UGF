#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "GASP_ValidationLibrary.generated.h"

class UGASP_CombatComponent;
class AGASP_MoverCharacter;

UCLASS()
class GASP_API UGASP_ValidationLibrary : public UBlueprintFunctionLibrary
{
    GENERATED_BODY()

public:
    UFUNCTION(BlueprintCallable, Category="GASP|Validation")
    static bool ValidateCombatComponent(UGASP_CombatComponent* CombatComponent, bool bLog=true);

    UFUNCTION(BlueprintCallable, Category="GASP|Validation")
    static bool ValidateMoverPawn(AGASP_MoverCharacter* Pawn, bool bLog=true);
};

