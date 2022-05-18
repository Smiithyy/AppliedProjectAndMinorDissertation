// Fill out your copyright notice in the Description page of Project Settings.


#include "Pickup.h"
#include "PlayerCharacter.h"
#include "Sound/SoundCue.h"
#include "Kismet/GameplayStatics.h"

APickup::APickup()
{
	
}

void APickup::OnOverLapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResults)
{
	Super::OnOverLapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResults);


	if (OtherActor)
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
		
		if (PlayerCharacter)
		{
			OnPickUpBP(PlayerCharacter);
			PlayerCharacter->PickupLocations.Add(GetActorLocation());

			if (OverLapParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverLapParticles, GetActorLocation(), FRotator(0.f), true);
			}
			if (OverLapSound)
			{
				UGameplayStatics::PlaySound2D(this, OverLapSound);
			}

			Destroy();
		}
	}
}

void APickup::OnOverLapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverLapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
}
