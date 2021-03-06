// Fill out your copyright notice in the Description page of Project Settings.


#include "ClawPotion.h"
#include "PaperFlipbookComponent.h" 
#include "Components/BoxComponent.h"
#include "ClawRemastered2Character.h"
#include "Kismet/GameplayStatics.h"
#include "Components/CapsuleComponent.h" 
#include "ClawGameMode.h"
#include "Engine/Engine.h"

AClawPotion::AClawPotion()
{
    PrimaryActorTick.bCanEverTick = false;

    // initializes the Bullet's box collision 
    HitCollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("Bullet Collision"));
    HitCollisionBox->SetBoxExtent(FVector(17.0f, 17.0f, 17.0f));
    HitCollisionBox->SetCollisionProfileName("Trigger");
    HitCollisionBox->SetupAttachment(RootComponent);

    HitCollisionBox->OnComponentBeginOverlap.AddDynamic(this, &AClawPotion::OnOverlapBegin);
    HitCollisionBox->OnComponentEndOverlap.AddDynamic(this, &AClawPotion::OnOverlapEnd);
}


void AClawPotion::BeginPlay()
{
    Super::BeginPlay();
    // the potion has touched claw
    UE_LOG(LogTemp, Warning, TEXT("potion"));

    GameModeRef = Cast<AClawGameMode>(UGameplayStatics::GetGameMode(GetWorld()));
}


void AClawPotion::OnOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
    // check it it's claw who's overlapping with the score object.
    if (OtherActor && OtherActor->IsA(AClawRemastered2Character::StaticClass()) && OtherComp->IsA(UCapsuleComponent::StaticClass()))
    {
        UE_LOG(LogTemp, Warning, TEXT("potion touched"));

        AClawRemastered2Character* clawCharacter;
        clawCharacter = Cast<AClawRemastered2Character>(OtherActor);

        if (clawCharacter->ClawHealth->GetHealth() != 100)
        {
            clawCharacter->ClawHealth->SetHealth(-20);

            // destroy the potion
            this->Destroy();
        } 
    }
}

void AClawPotion::OnOverlapEnd(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
}