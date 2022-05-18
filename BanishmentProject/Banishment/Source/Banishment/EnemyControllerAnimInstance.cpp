// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyControllerAnimInstance.h"
#include "EnemyController.h"

void UEnemyControllerAnimInstance::NativeInitializeAnimation()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{
			EnemyController = Cast<AEnemyController>(Pawn);
		}
	}
}

void UEnemyControllerAnimInstance::UpdateAnimationProperties()
{
	if (Pawn == nullptr)
	{
		Pawn = TryGetPawnOwner();
		if (Pawn)
		{			
			EnemyController = Cast<AEnemyController>(Pawn);
		}
	}

	if (Pawn)
	{
		FVector Speed = Pawn->GetVelocity(); // Return pawn velocity
		FVector LateralSpeed = FVector(Speed.X, Speed.Y, 0.f);
		MovementSpeed = LateralSpeed.Size();
	}
}
