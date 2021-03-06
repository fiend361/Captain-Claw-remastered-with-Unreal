// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "ClawGameMode.h"
#include "Components/ActorComponent.h"
#include "HealthComponent.generated.h"


class AClawGameMode;


UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class CLAWREMASTERED2_API UHealthComponent : public UActorComponent
{
	GENERATED_BODY()

private:

	UPROPERTY(EditAnywhere)
	float DefaultHealth = 100.0f;
	float Health = 0.0f;

	AClawGameMode* GameModeRef;

public:	
	// Sets default values for this component's properties
	UHealthComponent();

	float GetHealth();
	void SetHealth(float damage);

protected:
	// Called when the game starts
	virtual void BeginPlay() override;

	UFUNCTION()
	void TakeDamage(AActor* DamagedActor, float Damage, const UDamageType* DamageType, AController* InstigatedBy, AActor* DamageCauser);
		
};
