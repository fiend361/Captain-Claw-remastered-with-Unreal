// Fill out your copyright notice in the Description page of Project Settings.


#include "BlueOfficerBullet.h"
#include "PaperFlipbookComponent.h" 
#include "Components/BoxComponent.h"
#include "ClawRemastered2Character.h"
#include "EnemyCharacter.h"
#include "Kismet/GameplayStatics.h"	
#include "Components/CapsuleComponent.h" 
#include "GameFramework/ProjectileMovementComponent.h"
#include "ClawGameMode.h"
#include "BlueOfficer.h"
#include "Engine/Engine.h"


ABlueOfficerBullet::ABlueOfficerBullet()
{
	PrimaryActorTick.bCanEverTick = false;

	ProjectileMovement = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("Projectile Movement"));
	ProjectileMovement->InitialSpeed = MovementSpeed;
	ProjectileMovement->MaxSpeed = MovementSpeed;

	InitialLifeSpan = 2.0f;

	// initializes the Bullet's box collision 
	HitCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bullet Collision"));
	HitCollisionBox->SetBoxExtent(FVector(5.0f, 5.0f, 5.0f));
	HitCollisionBox->SetCollisionProfileName("Trigger");
	HitCollisionBox->SetupAttachment(RootComponent);

	HitCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &ABlueOfficerBullet::OnOverlapBegin); 
}

void ABlueOfficerBullet::BeginPlay()
{
	Super::BeginPlay();
	//UE_LOG(LogTemp, Warning, TEXT("pistoling"));

	GameModeRef = Cast<AClawGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}

void ABlueOfficerBullet::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	UE_LOG(LogTemp, Warning, TEXT("overlapping"));
	// check it it's claw who's overlapping with the score object.
	if (OtherActor && OtherActor->IsA(AClawRemastered2Character::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		// decrease the enemy health.
		UGameplayStatics::ApplyDamage(OtherActor, Damage, GetInstigatorController(), this, DamageType);

		// destroy the bullet.
		this->Destroy(); 
	}
}

