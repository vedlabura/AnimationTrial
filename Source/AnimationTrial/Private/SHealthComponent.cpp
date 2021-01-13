// Fill out your copyright notice in the Description page of Project Settings.


#include "SHealthComponent.h"
#include "MyPawn.h"
#include "Net/UnrealNetwork.h"

// Sets default values for this component's properties
USHealthComponent::USHealthComponent()
{

	Health = 100.0f;
	DefaultHealth = 100.0f;
}


// Called when the game starts
void USHealthComponent::BeginPlay()
{
	Super::BeginPlay();

	AMyPawn* MyOwner = Cast<AMyPawn>(GetOwner());
	if (MyOwner)
	{
		MyOwner->OnTakeAnyDamage.AddDynamic(this, &USHealthComponent::HandleTakeAnyDamage);
	}

	DefaultHealth = Health;
	
	
}

void USHealthComponent::HandleTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (Damage < 0.0f) {
		return;
	}
	Health = FMath::Clamp(Health - Damage, 0.0f, DefaultHealth);

	AMyPawn* MyOwner = Cast<AMyPawn>(GetOwner());
	if (Health == 0.0f) {
		MyOwner->IsDead = true;
	}
	
}


void USHealthComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(USHealthComponent, Health);
}

