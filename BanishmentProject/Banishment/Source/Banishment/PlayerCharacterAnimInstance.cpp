// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerCharacterAnimInstance.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "PlayerCharacter.h"

void UPlayerCharacterAnimInstance::NativeInitializeAnimation()
{
	// Check if pawn is empty
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner(); // Get pawn from owner
		if (Pawn)
		{
			PlayerCharacter = Cast<APlayerCharacter>(Pawn);
		}
	}
}

void UPlayerCharacterAnimInstance::UpdateAnimationProperties()
{
	// Check if pawn is empty
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner(); // Get pawn from owner
	}

	// If pawn is not empty
	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity(); // Return pawn velocity
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();

		bIsInAir = Pawn->GetMovementComponent()->IsFalling();

		if (PlayerCharacter == nullptr)
		{
			PlayerCharacter = Cast<APlayerCharacter>(Pawn);
		}
	}
}
