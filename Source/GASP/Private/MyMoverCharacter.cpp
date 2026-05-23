#include "MyMoverCharacter.h"

AMyMoverCharacter::AMyMoverCharacter()
{
	RootSceneComponent = CreateDefaultSubobject<USceneComponent>(TEXT("RootComponent"));
	RootComponent = RootSceneComponent;

    // Initialize the MoverComponent. UMoverComponent derives from UActorComponent
	// (not a USceneComponent) so it should not call SetupAttachment.
	MoverComponent = CreateDefaultSubobject<UMoverComponent>(TEXT("MoverComponent"));
}

