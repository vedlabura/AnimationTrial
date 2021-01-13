// Fill out your copyright notice in the Description page of Project Settings.


#include "SPawn.h"

// Sets default values
ASPawn::ASPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

}

// Called when the game starts or when spawned
void ASPawn::BeginPlay()
{
	Super::BeginPlay();
	
}

// Called every frame
void ASPawn::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

}

