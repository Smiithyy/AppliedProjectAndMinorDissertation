// Fill out your copyright notice in the Description page of Project Settings.


#include "Explosive.h"

#include "EnemyController.h"
#include "PlayerCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/World.h"
#include "Sound/SoundCue.h"

AExplosive::AExplosive()
{
	Damage = 15.f;
}

void AExplosive::OnOverLapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResults)
{
	Super::OnOverLapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResults);

	if (OtherActor)
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
		AEnemyController* EnemyController = Cast<AEnemyController>(OtherActor);
		
		if (PlayerCharacter || EnemyController)
		{
			if (OverLapParticles)
			{
				UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), OverLapParticles, GetActorLocation(), FRotator(0.f), true);
			}
			if (OverLapSound)
			{
				UGameplayStatics::PlaySound2D(this, OverLapSound);
			}
			UGameplayStatics::ApplyDamage(OtherActor, Damage, nullptr, this, DamageTypeClass);
			
			Destroy();
		}
		
	}
}

void AExplosive::OnOverLapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnOverLapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);
	UE_LOG(LogTemp, Warning, TEXT("Explosive::OnOverLapEnd()"));
}



