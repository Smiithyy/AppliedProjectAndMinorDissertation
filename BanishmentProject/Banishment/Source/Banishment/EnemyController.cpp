// Fill out your copyright notice in the Description page of Project Settings.


#include "EnemyController.h"

#include "AIController.h"
#include "Components/SphereComponent.h"
#include "PlayerCharacter.h"
#include "PlayerCharacterController.h"
#include "Components/BoxComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Sound/SoundCue.h"
#include "Animation/AnimInstance.h"
#include "Components/CapsuleComponent.h"

// Sets default values
AEnemyController::AEnemyController()
{
 	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	AgroSphere = CreateDefaultSubobject<USphereComponent>(TEXT("AgroSphere"));
	AgroSphere->SetupAttachment(GetRootComponent());
	AgroSphere->SetCollisionResponseToChannel(ECollisionChannel::ECC_WorldDynamic, ECollisionResponse::ECR_Ignore);
	AgroSphere->InitSphereRadius(500.f);

	CombatSphere = CreateDefaultSubobject<USphereComponent>(TEXT("CombatSphere"));
	CombatSphere->SetupAttachment(GetRootComponent());
	CombatSphere->InitSphereRadius(125.f);

	CombatCollision = CreateDefaultSubobject<UBoxComponent>(TEXT("CombatCollision"));
	CombatCollision->SetupAttachment(GetMesh(), FName("EnemySocket"));
	
	bOverlappingCombatSphere = false;
	bAttacking = false;

	Health = 75.f;
	MaxHealth = 100.f;
	Damage = 10.f;

	AttackMinTime = 0.5f;
	AttackMaxTime = 2.0f;

	EnemyMovementStatus = EEnemyMovementStatus::EMS_Idle;

	DeathDelay = 3.f;

	bHasValidTarget = false;
}

// Called when the game starts or when spawned
void AEnemyController::BeginPlay()
{
	Super::BeginPlay();

	AIController = Cast<AAIController>(GetController());

	AgroSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyController::AgroSphereOnOverLapBegin);
	AgroSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemyController::AgroSphereOnOverLapEnd);
	
	CombatSphere->OnComponentBeginOverlap.AddDynamic(this, &AEnemyController::CombatSphereOnOverLapBegin);
	CombatSphere->OnComponentEndOverlap.AddDynamic(this, &AEnemyController::CombatSphereOnOverLapEnd);

	CombatCollision->OnComponentBeginOverlap.AddDynamic(this, &AEnemyController::CombatOnOverLapBegin);
	CombatCollision->OnComponentEndOverlap.AddDynamic(this, &AEnemyController::CombatOnOverLapEnd);

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatCollision->SetCollisionObjectType(ECollisionChannel::ECC_WorldDynamic);
	CombatCollision->SetCollisionResponseToAllChannels(ECollisionResponse::ECR_Ignore);
	CombatCollision->SetCollisionResponseToChannel(ECollisionChannel::ECC_Pawn, ECollisionResponse::ECR_Overlap);

	GetMesh()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
	GetCapsuleComponent()->SetCollisionResponseToChannel(ECollisionChannel::ECC_Camera, ECollisionResponse::ECR_Ignore);
}

// Called every frame
void AEnemyController::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void AEnemyController::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

void AEnemyController::AgroSphereOnOverLapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResults)
{
	if (OtherActor && Alive())
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			MoveToTarget(PlayerCharacter);
		}
	}
}

void AEnemyController::AgroSphereOnOverLapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor)
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			bHasValidTarget = false;
			if (PlayerCharacter->CombatTarget == this)
			{
				PlayerCharacter->SetCombatTarget(nullptr);
			}
			PlayerCharacter->SetHasCombatTarget(false);
			PlayerCharacter->UpdateCombatTarget();
			
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Idle);
			if (AIController)
			{
				AIController->StopMovement();
			}
		}
	}
}

void AEnemyController::CombatSphereOnOverLapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResults)
{
	if (OtherActor && Alive())
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			bHasValidTarget = true;
			
			PlayerCharacter->SetCombatTarget(this);
			PlayerCharacter->SetHasCombatTarget(true);
			PlayerCharacter->UpdateCombatTarget();
			
			CombatTarget = PlayerCharacter;
			bOverlappingCombatSphere = true;
			Attack();
		}
	}
}

void AEnemyController::CombatSphereOnOverLapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (OtherActor && OtherComp)
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			bOverlappingCombatSphere = false;
			if(EnemyMovementStatus != EEnemyMovementStatus::EMS_Attacking)
			{
				MoveToTarget(PlayerCharacter);
				CombatTarget = nullptr;
			}

			if (PlayerCharacter->CombatTarget == this)
			{
				PlayerCharacter->SetCombatTarget(nullptr);
				PlayerCharacter->bHasCombatTarget = false;
				PlayerCharacter->UpdateCombatTarget();
			}
			if (PlayerCharacter->PlayerCharacterController)
			{
				USkeletalMeshComponent* PlayerCharacterMesh = Cast<USkeletalMeshComponent>(OtherComp);
				if (PlayerCharacterMesh) PlayerCharacter->PlayerCharacterController->RemoveEnemyHealthBar();
			}
			
			GetWorldTimerManager().ClearTimer(AttackTimer);
		}
	}
}

void AEnemyController::MoveToTarget(APlayerCharacter* Target)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_MoveToTarget);

	if (AIController)
	{
		FAIMoveRequest MoveRequest;
		MoveRequest.SetGoalActor(Target);
		MoveRequest.SetAcceptanceRadius(40.0f);

		FNavPathSharedPtr NavPath;

		AIController->MoveTo(MoveRequest, &NavPath);

		/*auto PathPoints = NavPath->GetPathPoints();
		for (auto Point : PathPoints)
		{
			FVector Location = Point.Location;

			UKismetSystemLibrary::DrawDebugSphere(this, Location, 25.f, 8, FLinearColor::Red, 10.f, 1.5f);
			
		}*/
	}
}

void AEnemyController::CombatOnOverLapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResults)
{
	if (OtherActor)
	{
		APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(OtherActor);
		if (PlayerCharacter)
		{
			if (PlayerCharacter->HitParticles)
			{
				const USkeletalMeshSocket* TipSocket = GetMesh()->GetSocketByName("TipSocket");
				if (TipSocket)
				{
					FVector SocketLocation = TipSocket->GetSocketLocation(GetMesh());
					UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), PlayerCharacter->HitParticles, SocketLocation, FRotator(0.f), false);
				}
			}
			if (PlayerCharacter->HitSound)
			{
				UGameplayStatics::PlaySound2D(this, PlayerCharacter->HitSound);
			}
			if (DamageTypeClass)
			{
				UGameplayStatics::ApplyDamage(PlayerCharacter, Damage, AIController, this, DamageTypeClass);
			}
		}
	}
}

void AEnemyController::CombatOnOverLapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	
}

void AEnemyController::ActivateCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	if (SwingSound)
	{
		UGameplayStatics::PlaySound2D(this, SwingSound);
	}
}

void AEnemyController::EndCollision()
{
	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
}

void AEnemyController::Attack()
{
	if (Alive() && bHasValidTarget)
	{
		if (AIController)
		{
			AIController->StopMovement();
			SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Attacking);
		}
		if (!bAttacking)
		{
			bAttacking = true;
			UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
			if (AnimInstance)
			{
				AnimInstance->Montage_Play(CombatMontage, 1.35f);
				AnimInstance->Montage_JumpToSection(FName("Attack"), CombatMontage);
			}
		}
	}
}

void AEnemyController::AttackEnd()
{
	bAttacking = false;
	if (bOverlappingCombatSphere)
	{
		float AttackTime = FMath::FRandRange(AttackMinTime, AttackMaxTime);
		GetWorldTimerManager().SetTimer(AttackTimer, this, &AEnemyController::Attack, AttackTime);
	}
}

float AEnemyController::TakeDamage(float DamageAmount, FDamageEvent const& DamageEvent, AController* EventInstigator, AActor* DamageCauser)
{
	if (Health - DamageAmount <= 0.f)
	{
		Health -= DamageAmount;
		Die(DamageCauser);
	}
	else
	{
		Health -= DamageAmount;
	}
	return DamageAmount;
}

void AEnemyController::Die(AActor* Causer)
{
	SetEnemyMovementStatus(EEnemyMovementStatus::EMS_Dead);
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance)
	{
		AnimInstance->Montage_Play(CombatMontage, 1.35f);
		AnimInstance->Montage_JumpToSection(FName("Death"), CombatMontage);
	}

	CombatCollision->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	AgroSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CombatSphere->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	GetCapsuleComponent()->SetCollisionEnabled(ECollisionEnabled::NoCollision);

	bAttacking = false;

	APlayerCharacter* PlayerCharacter = Cast<APlayerCharacter>(Causer);
	if (PlayerCharacter)
	{
		PlayerCharacter->UpdateCombatTarget();
	}
}

void AEnemyController::DeathEnd()
{
	GetMesh()->bPauseAnims = true;
	GetMesh()->bNoSkeletonUpdate = true;

	GetWorldTimerManager().SetTimer(DeathTimer, this, &AEnemyController::Disappear, DeathDelay);
}

bool AEnemyController::Alive()
{
	return GetEnemyMovementStatus() != EEnemyMovementStatus::EMS_Dead;
}

void AEnemyController::Disappear()
{
	Destroy();
}