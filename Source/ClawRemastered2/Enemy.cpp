// Fill out your copyright notice in the Description page of Project Settings.


#include "Enemy.h"
#include "PaperFlipbookComponent.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "Components/BoxComponent.h"
#include "ClawRemastered2Character.h"
#include "BlueOfficerBullet.h"
#include "Kismet/GameplayStatics.h"
#include "Engine/Engine.h"

AEnemy::AEnemy()
{
	// Use only Yaw from the controller and ignore the rest of the rotation.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = true;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->GravityScale = 2.0f;
	GetCharacterMovement()->AirControl = 0.80f;
	GetCharacterMovement()->JumpZVelocity = 1000.f;
	GetCharacterMovement()->GroundFriction = 3.0f;
	GetCharacterMovement()->MaxWalkSpeed = 100.0f;
	GetCharacterMovement()->MaxFlySpeed = 100.0f;

	GetCharacterMovement()->bOrientRotationToMovement = false;

	// Lock character motion onto the XZ plane, so the character can't move in or out of the screen
	GetCharacterMovement()->bConstrainToPlane = true;
	GetCharacterMovement()->SetPlaneConstraintNormal(FVector(0.0f, -1.0f, 0.0f));

	// Behave like a traditional 2D platformer character, with a flat bottom instead of a curved capsule bottom
	// Note: This can cause a little floating when going up inclines; you can choose the tradeoff between better
	// behavior on the edge of a ledge versus inclines by setting this to true or false
	GetCharacterMovement()->bUseFlatBaseForFloorChecks = true;

	// Enable replication on the Sprite component so animations show up when networked
	GetSprite()->SetIsReplicated(true);
	bReplicates = true;

	//UE_LOG(LogTemp, Warning, TEXT("swording"));
	OfficerHealth = CreateDefaultSubobject<UHealthComponent>(TEXT("HealthComponent"));

	OfficerIdleSightCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Officer Idle Sight"));
	OfficerIdleSightCollisionBox->SetBoxExtent(FVector(300.0f, 20.0f, 60.0f));
	OfficerIdleSightCollisionBox->SetCollisionProfileName("Tr1igger");
	OfficerIdleSightCollisionBox->SetupAttachment(RootComponent);

	OfficerWalkSightCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Officer Walk Sight"));
	OfficerWalkSightCollisionBox->SetBoxExtent(FVector(32.0f, 32.0f, 60.0f));
	OfficerWalkSightCollisionBox->SetCollisionProfileName("Trigger");
	OfficerWalkSightCollisionBox->SetupAttachment(RootComponent);
}

void AEnemy::BeginPlay()
{
	// Call the base class  
	Super::BeginPlay();

	OfficerIdleSightCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlapBeginIdleSightCollisionBox);
	OfficerIdleSightCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnOverlapEndIdleSightCollisionBox);

	OfficerWalkSightCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AEnemy::OnOverlapBeginWalkSightCollisionBox);
	OfficerWalkSightCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AEnemy::OnOverlapEndWalkSightCollisionBox);

	//UE_LOG(LogTemp, Warning, TEXT("beginplay"));

	currentState = walking;
	walkDirection = 1.0f;

	GetWorldTimerManager().SetTimer(EndWalkTimer, this, &AEnemy::TurnLeft, walkDuration, false);
}

void AEnemy::UpdateCharacter()
{
	const FVector PlayerVelocity = GetVelocity();
	const float PlayerSpeedSqr = PlayerVelocity.SizeSquared();

	UPaperFlipbook* CurrentAnimation = GetSprite()->GetFlipbook();

	if (currentState == walking) {
		AddMovementInput(FVector(walkDirection, 0.0f, 0.0f), 1);
		if (CurrentAnimation != WalkingAnimation)
		{
			GetSprite()->SetFlipbook(WalkingAnimation);

			//UE_LOG(LogTemp, Warning, TEXT("right"));
		}
	}
	else if (currentState == idling) {
		if (CurrentAnimation != IdleAnimation)
		{
			GetSprite()->SetFlipbook(IdleAnimation);
		}
	}
	else if (currentState == aggroed) {
		if (CurrentAnimation != AggroedAnimation)
		{
			GetSprite()->SetFlipbook(AggroedAnimation);
		}
		ActAggroed();
	}
	else if (currentState == dead) {
		if (CurrentAnimation != DeadAnimation)
		{
			GetSprite()->SetFlipbook(DeadAnimation);
		}
		AddMovementInput(FVector(deathJumpDirection, 0.0f, 0.0f), 1);
	}
	//else if (currentState == hurt) {
	//	DesiredAnimation = HurtAnimation;
	//}
}

void AEnemy::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	UpdateCharacter();

	UpdateRotation();

	//UE_LOG(LogTemp, Error, TEXT("Value = %f"), walkDirection);
	//UE_LOG(LogTemp, Error, TEXT("Value = %f"), GetWorldTimerManager().GetTimerRemaining(EndWalkTimer));
}

void AEnemy::OnOverlapBeginIdleSightCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (currentState == idling && OtherActor && OtherActor->IsA(AClawRemastered2Character::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		//UE_LOG(LogTemp, Error, TEXT("begin overlap idle sight"));
		UpdateToClawCharacterDirection(OtherComp);

		currentState = aggroed;
		GetWorldTimerManager().PauseTimer(EndWalkTimer);
	}
}

void AEnemy::OnOverlapEndIdleSightCollisionBox(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (currentState == aggroed && OtherActor && OtherActor->IsA(AClawRemastered2Character::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		//UE_LOG(LogTemp, Error, TEXT("end overlap idle sight"));
		UpdateToClawCharacterDirection(OtherComp);

		currentState = walking;
		GetWorldTimerManager().UnPauseTimer(EndWalkTimer);
	}
}

void AEnemy::OnOverlapBeginWalkSightCollisionBox(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	if (currentState == walking && OtherActor && OtherActor->IsA(AClawRemastered2Character::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		//UE_LOG(LogTemp, Error, TEXT("begin overlap walk sight"));
		UpdateToClawCharacterDirection(OtherComp);

		currentState = aggroed;
		GetWorldTimerManager().PauseTimer(EndWalkTimer);
	}
}

void AEnemy::OnOverlapEndWalkSightCollisionBox(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	if (currentState == aggroed && OtherActor && OtherActor->IsA(AClawRemastered2Character::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
	{
		//UE_LOG(LogTemp, Error, TEXT("end overlap walk sight"));
		UpdateToClawCharacterDirection(OtherComp);

		currentState = walking;
		GetWorldTimerManager().UnPauseTimer(EndWalkTimer);
	}
}

void AEnemy::TurnRight()
{
	currentState = walking;

	GetWorldTimerManager().SetTimer(EndWalkTimer, this, &AEnemy::TurnLeft, walkDuration, false);
	walkDirection *= -1;

	//UE_LOG(LogTemp, Error, TEXT("turn right"));
}

void AEnemy::TurnLeft()
{
	patrols++;

	if (patrols == 2) 
	{
		patrols = 0;

		currentState = idling;
		
		UPrimitiveComponent* clawCapsuleComponent = CheckIfClawInSight();
		if (clawCapsuleComponent)
		{
			UpdateToClawCharacterDirection(clawCapsuleComponent);

			currentState = aggroed;
			GetWorldTimerManager().PauseTimer(EndWalkTimer);
		}
			
		GetWorldTimerManager().SetTimer(EndWalkTimer, this, &AEnemy::TurnRight, idlingDuration, false);

		//UE_LOG(LogTemp, Error, TEXT("start idle"));
	}
	else
	{
		currentState = walking;

		GetWorldTimerManager().SetTimer(EndWalkTimer, this, &AEnemy::TurnRight, walkDuration, false);
		walkDirection *= -1;

		//UE_LOG(LogTemp, Error, TEXT("turn left"));
	}
}

UPrimitiveComponent* AEnemy::CheckIfClawInSight()
{
	TSet<UPrimitiveComponent*> OverlappingComponents;

	OfficerIdleSightCollisionBox->GetOverlappingComponents(OverlappingComponents);

	for (auto& Component : OverlappingComponents)
	{
		if (Component->IsA(UCapsuleComponent::StaticClass()))
		{
			if (Component->GetOwner()->IsA(AClawRemastered2Character::StaticClass()))
			{
				return Component;
			}
		}
	}
	
	return nullptr;
}

void AEnemy::UpdateRotation()
{
	float direction = walkDirection;
	if (currentState == aggroed) 
	{
		direction = toClawCharacterDirection;
	}

	if (direction == 1) 
	{
		SetRotationToRight();
	}
	else 
	{
		SetRotationToLeft();
	}
}

void AEnemy::UpdateToClawCharacterDirection(UPrimitiveComponent* clawCapsule)
{
	//UE_LOG(LogTemp, Error, TEXT("Value = %f"), GetActorLocation().X - clawCapsule->GetComponentLocation().X);
	if (GetActorLocation().X - clawCapsule->GetComponentLocation().X < 0) {
		toClawCharacterDirection = 1.0f;
	}
	else {
		toClawCharacterDirection = -1.0f;
	}
}

void AEnemy::SetRotationToRight()
{
	SetActorRotation(FRotator(0.0f, 180.0f, 0.0f));
}

void AEnemy::SetRotationToLeft()
{
	SetActorRotation(FRotator(0.0f, 0.0f, 0.0f));
}

void AEnemy::OnDamageTaken(float DamageAmount, const UDamageType* damageType, AController* InstigatedBy, AActor* DamageCauser)
{
	if (GetActorLocation().X - DamageCauser->GetActorLocation().X < 0) {
		deathJumpDirection = -1.0f;
	}
	
	if (OfficerHealth->GetHealth() <= 0)
	{
		HandleDeath();
	}
}

void AEnemy::HandleDeath()
{
	UGameplayStatics::SpawnSound2D(this, DeathSound, 1.0f, 1.0f, 0.0f);

	FTimerHandle UnusedHandle;
	GetWorldTimerManager().SetTimer(UnusedHandle, this, &AEnemy::DestroySelf, 1.6f, false);
	GetWorldTimerManager().ClearTimer(EndWalkTimer);

	Jump();
	this->SetActorEnableCollision(false);

	currentState = dead;
}

void AEnemy::DestroySelf()
{
	UE_LOG(LogTemp, Error, TEXT("destroyed.."));
	this->Destroy();
}

void AEnemy::ActAggroed()
{
	//UE_LOG(LogTemp, Error, TEXT("claw Detected.."));

	AddMovementInput(FVector(toClawCharacterDirection, 0.0f, 0.0f), 1);
}

