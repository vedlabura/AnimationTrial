// Fill out your copyright notice in the Description page of Project Settings.


#include "SCharacter.h"
#include "SWeapon.h"
#include "Net/UnrealNetwork.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "Camera/CameraComponent.h"
#include "SHealthComponent.h"

// Sets default values
ASCharacter::ASCharacter()
{
	// Set this character to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true;

	Crouching = false;
	Proning = false;
	IsDead = false;
	SetReplicates(true);
	SetReplicateMovement(true);
	GetCharacterMovement()->MaxWalkSpeed = 600.0f;

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));
	MeshComp->SetupAttachment(RootComponent);
	WeaponComp = CreateDefaultSubobject<ASWeapon>(TEXT("Components"));

	

}
// Called when the game starts or when spawned
void ASCharacter::BeginPlay()
{
	Super::BeginPlay();

	
}

void ASCharacter::AddtoMesh()
{
	if (WeaponComp) {
		WeaponComp->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, "WeaponSocket");
		WeaponComp->SetOwner(this);
	}
}

void ASCharacter::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is forward
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get forward vector
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::X);
		AddMovementInput(Direction, Value);
	}
}

void ASCharacter::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		// find out which way is right
		const FRotator Rotation = Controller->GetControlRotation();
		const FRotator YawRotation(0, Rotation.Yaw, 0);

		// get right vector 
		const FVector Direction = FRotationMatrix(YawRotation).GetUnitAxis(EAxis::Y);
		// add movement in that direction
		AddMovementInput(Direction, Value);
	}
}


void ASCharacter::DoCrouch()
{
	if (this->GetLocalRole() == ROLE_Authority) {
	Crouching = !Crouching;
	Proning = false;
	
		if (Crouching) {
			GetCharacterMovement()->MaxWalkSpeed = 300.0f;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		}
	}
	else
	{
		ServerDoCrouch();
	}
}

void ASCharacter::ServerDoCrouch_Implementation()
{
	DoCrouch();
}

bool ASCharacter::ServerDoCrouch_Validate()
{
	return true;
}

void ASCharacter::Prone()
{
	if (this->GetLocalRole() == ROLE_Authority) {
		Proning = !Proning;
		Crouching = false;

		if (Proning) {
			GetCharacterMovement()->MaxWalkSpeed = 100.0f;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		}

	}
	else
	{
		ServerProne();
		if (Proning) {
			GetCharacterMovement()->MaxWalkSpeed = 100.0f;
		}
		else
		{
			GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		}
	}
}

void ASCharacter::DoJump()
{
	if (this->GetLocalRole() == ROLE_Authority) {
		if (!Proning && !Crouching) {
			GetCharacterMovement()->AddImpulse(FVector(0.0f, 0.0f, 100.0f), true);
		}

		else {
			Proning = false;
			Crouching = false;
			GetCharacterMovement()->MaxWalkSpeed = 600.0f;
		}
	}
	else
	{
		ServerDoJump();
		Proning = false;
		Crouching = false;
		GetCharacterMovement()->MaxWalkSpeed = 600.0f;
	}
}

void ASCharacter::ServerDoJump_Implementation()
{
	DoJump();
}

bool ASCharacter::ServerDoJump_Validate()
{
	return true;
}

void ASCharacter::ServerProne_Implementation()
{
	Prone();
}

bool ASCharacter::ServerProne_Validate()
{
	return true;
}



void ASCharacter::PickUp(ASWeapon* wep)
{
	if (this->GetLocalRole() == ROLE_Authority) {
		
		WeaponComp = wep;
		AddtoMesh();
	}
	else
	{
		ServerPickUp(wep);
	}
}

void ASCharacter::ServerPickUp_Implementation(ASWeapon* wep)
{
	PickUp(wep);
}

bool ASCharacter::ServerPickUp_Validate(ASWeapon* wep)
{
	return true;
}

void ASCharacter::Drop(ASWeapon* wep)
{

	if (this->GetLocalRole() == ROLE_Authority) {
		MulticastDropEffect();
		WeaponComp = NULL;
	}
	else
	{
		ServerDrop(wep);
	}
}

void ASCharacter::ServerDrop_Implementation(ASWeapon* wep)
{
	Drop(wep);
}

bool ASCharacter::ServerDrop_Validate(ASWeapon* wep)
{
	return true;
}


void ASCharacter::MulticastDropEffect_Implementation()
{
	WeaponComp->DetachRootComponentFromParent(true);
}

// Called every frame
void ASCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

// Called to bind functionality to input
void ASCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);
	PlayerInputComponent->BindAxis("MoveForward", this, &ASCharacter::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &ASCharacter::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &ASCharacter::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &ASCharacter::AddControllerYawInput);


	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &ASCharacter::DoCrouch);
	PlayerInputComponent->BindAction("Prone", IE_Pressed, this, &ASCharacter::Prone);

}

void ASCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(ASCharacter, IsDead);
	DOREPLIFETIME(ASCharacter, WeaponComp);
	
}