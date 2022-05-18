// Copyright Epic Games, Inc. All Rights Reserved.

#include "BanishmentGameMode.h"
#include "BanishmentCharacter.h"
#include "UObject/ConstructorHelpers.h"

ABanishmentGameMode::ABanishmentGameMode()
{
	// set default pawn class to our Blueprinted character
	static ConstructorHelpers::FClassFinder<APawn> PlayerPawnBPClass(
		TEXT("/Game/ThirdPersonCPP/Blueprints/ThirdPersonCharacter"));
	if (PlayerPawnBPClass.Class != NULL)
	{
		DefaultPawnClass = PlayerPawnBPClass.Class;
	}
}
