// Fill out your copyright notice in the Description page of Project Settings.


#include "MyPawn.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/CollisionProfile.h"
#include "Engine/EngineTypes.h"
#include "SHealthComponent.h"
#include "SWeapon.h"
#include "Components/CapsuleComponent.h"
#include "Net/UnrealNetwork.h"



// Sets default values
AMyPawn::AMyPawn()
{
 	// Set this pawn to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	CapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("CapsuleComp");
	CapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	CapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	CapsuleComponent->CanCharacterStepUpOn = ECB_No;
	RootComponent = CapsuleComponent;

	ProneCapsuleComponent = CreateDefaultSubobject<UCapsuleComponent>("ProneCapsuleComp");
	ProneCapsuleComponent->InitCapsuleSize(34.0f, 88.0f);
	ProneCapsuleComponent->SetCollisionProfileName(UCollisionProfile::Pawn_ProfileName);

	ProneCapsuleComponent->SetupAttachment(CapsuleComponent);


	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>("MeshComp");
	MeshComp->SetupAttachment(CapsuleComponent);
	
	WeaponComp = CreateDefaultSubobject<ASWeapon>(TEXT("Components"));
	HealthComp = CreateDefaultSubobject<USHealthComponent>(TEXT("HealthComp"));

}

// Called when the game starts or when spawned
void AMyPawn::BeginPlay()
{
	Super::BeginPlay();
	bOnFloor = false;

	NewLoc = GetActorLocation();
	OldLoc = GetActorLocation();
	
}





void AMyPawn::ServerMove_Implementation(FVector Vel, FVector oldLoc, FVector newLoc, FRotator Rot, float timeStamp)
{
	if (GetLocalRole() == ROLE_Authority) {
		SetActorLocation(newLoc);
		GetMovementComponent()->Velocity = Vel;
		if (bProning)
		{
			RootComponent->SetWorldRotation(FRotator(-90, Rot.Yaw, 0));
		}
		else
		{
			RootComponent->SetWorldRotation(FRotator(0, Rot.Yaw, 0));
		}
		
	}
}


// Called every frame
void AMyPawn::Tick(float DeltaTime)
{

	Super::Tick(DeltaTime);
	CheckFloor();

	OldLoc = NewLoc;
	NewLoc = GetActorLocation();



	if (GetLocalRole() == ROLE_AutonomousProxy) {
		if (!bProning) {
			RootComponent->SetRelativeRotation(FRotator(0, GetControlRotation().Yaw, 0));
		}
		else
		{
			RootComponent->SetRelativeRotation(FRotator(-90, GetControlRotation().Yaw, 0));
		}

		if (!bOnFloor) {
			velInAir = velInAir- FVector(0,0,1000)*DeltaTime;
//			SetActorLocation(GetActorLocation() +velInAir*DeltaTime, true);
			

			FHitResult Hit(1.f);
			GetMovementComponent()->SafeMoveUpdatedComponent(velInAir*DeltaTime,FRotator(0,GetControlRotation().Yaw,0), true, Hit);

			if (Hit.IsValidBlockingHit())
			{
				GetMovementComponent()->HandleImpact(Hit, DeltaTime, velInAir * DeltaTime);
				// Try to slide the remaining distance along the surface.
				GetMovementComponent()->SlideAlongSurface(velInAir * DeltaTime, 1.f - Hit.Time, Hit.Normal, Hit, true);
			}


			CheckFloor();
			if (bOnFloor) {
				velInAir = GetMovementComponent()->Velocity;
			}
		}
		
		if (bWantsToJump) {
			GetMovementComponent()->Velocity = GetMovementComponent()->Velocity + FVector(0, 0, 450);
			velInAir = GetMovementComponent()->Velocity;


			FHitResult Hit(1.f);
			GetMovementComponent()->SafeMoveUpdatedComponent(velInAir * DeltaTime, FRotator(0, GetControlRotation().Yaw, 0), true, Hit);

			if (Hit.IsValidBlockingHit())
			{
				GetMovementComponent()->HandleImpact(Hit, DeltaTime, velInAir * DeltaTime);
				// Try to slide the remaining distance along the surface.
				GetMovementComponent()->SlideAlongSurface(velInAir * DeltaTime, 1.f - Hit.Time, Hit.Normal, Hit, true);
			}

//			SetActorLocation(GetActorLocation() + GetMovementComponent()->Velocity*DeltaTime, true);
			bOnFloor = false;
			bWantsToJump = false;

		}

		ServerMove(GetMovementComponent()->Velocity, OldLoc, NewLoc, GetControlRotation(), 1000.0f);
	}
}


void AMyPawn::MoveForward(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		FVector right = GetActorRightVector();

		FVector Direction = FVector::CrossProduct( right, FloorNormal);
		if (!bOnFloor) {
			Value = Value / 100;
		}

		Direction.Normalize();
		AddMovementInput(Direction, Value);
	}
}

void AMyPawn::MoveRight(float Value)
{
	if ((Controller != NULL) && (Value != 0.0f))
	{
		FVector fwd = GetActorForwardVector();

		FVector Direction = FVector::CrossProduct(FloorNormal, fwd);

		if (!bOnFloor) {
			Value = Value / 20;
		}

		Direction.Normalize();
		AddMovementInput(Direction, Value);
	}
}



void AMyPawn::CheckFloor()
{
		FHitResult Hit;
		FCollisionQueryParams QueryParams;
		QueryParams.AddIgnoredActor(GetOwner());
		QueryParams.AddIgnoredActor(this);
		FVector trace = FVector(0,0,-105.0f);
		FVector loc = FVector(0, 0, 89.0f);
		float dist = 90;
		if (bProning) {
			trace.Z = -52.0f;
			dist = 36.0f;
		}
		if (bCrouching) {
			trace.Z = -80.0f;
			dist = 68.0f;
		}
		loc.Z = dist - 1.0f;
		if (GetWorld()->LineTraceSingleByChannel(Hit, RootComponent->GetComponentLocation(), RootComponent->GetComponentLocation() + trace, ECC_WorldStatic, QueryParams)) {
			bOnFloor = true;
			FloorNormal = Hit.ImpactNormal;
		}
		else
		{
			bOnFloor = false;
		}
}

void AMyPawn::DoCrouch()
{
	UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();
	if (!bCrouching) {
		moveComp->MaxSpeed = 200;
		bCrouching = !bCrouching;
		bProning = false;
		
		if (RootComponent != CapsuleComponent) {
			CapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			RootComponent = CapsuleComponent;
			
			ProneCapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			if (moveComp != NULL) {
				moveComp->SetUpdatedComponent(RootComponent);
				
			}

		}

		MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		CapsuleComponent->SetCapsuleHalfHeight(66.0f);
		CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() - FVector(0, 0, 22.0f));
		MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
		ProneCapsuleComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
	}
	else
	{
		moveComp->MaxSpeed = 400;
		bCrouching = !bCrouching;
		bProning = false;

		ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		CapsuleComponent->SetCapsuleHalfHeight(88.0f);
		CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() + FVector(0, 0, 22.0f));
		MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
		
		ProneCapsuleComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));


	}
	
	ServerDoCrouch();
}

void AMyPawn::ServerDoCrouch_Implementation()
{
	if (!bCrouching) {
		bCrouching = !bCrouching;
		bProning = false;

		if (RootComponent != CapsuleComponent) {
			CapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			RootComponent = CapsuleComponent;
			UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();
			ProneCapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			if (moveComp != NULL) {
				moveComp->SetUpdatedComponent(RootComponent);
			}

		}

		MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		CapsuleComponent->SetCapsuleHalfHeight(66.0f);
		CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() - FVector(0, 0, 22.0f));
		MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
		ProneCapsuleComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
	}
	else
	{
		bCrouching = !bCrouching;
		bProning = false;

		ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		CapsuleComponent->SetCapsuleHalfHeight(88.0f);
		CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() + FVector(0, 0, 22.0f));
		MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
		ProneCapsuleComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));


	}
	MulticastDoCrouch();
}

bool AMyPawn::ServerDoCrouch_Validate()
{
	return true;
}

void AMyPawn::MulticastDoCrouch_Implementation()
{
	if (GetLocalRole() == ROLE_SimulatedProxy) {

		if (!bCrouching) {
			bCrouching = !bCrouching;
			bProning = false;

			if (RootComponent != CapsuleComponent) {
				CapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
				RootComponent = CapsuleComponent;
				UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();
				ProneCapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
				if (moveComp != NULL) {
					moveComp->SetUpdatedComponent(RootComponent);
				}

			}

			MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			CapsuleComponent->SetCapsuleHalfHeight(66.0f);
			CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() - FVector(0, 0, 22.0f));
			MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			ProneCapsuleComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
		}
		else
		{
			bCrouching = !bCrouching;
			bProning = false;

			ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			CapsuleComponent->SetCapsuleHalfHeight(88.0f);
			CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() + FVector(0, 0, 22.0f));
			MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			ProneCapsuleComponent->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));


		}

	}

}

void AMyPawn::DoProne()
{

	UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();
	if (!bProning) {
		moveComp->MaxSpeed = 100;
		if (!bCrouching) {
			bProning = !bProning;
			bCrouching = false;
			ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			RootComponent = ProneCapsuleComponent;

			CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			if (moveComp != NULL) {
				moveComp->SetUpdatedComponent(RootComponent);
			}
		}

		else {
			bProning = !bProning;
			bCrouching = false;

			ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			RootComponent = ProneCapsuleComponent;

			MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			CapsuleComponent->SetCapsuleHalfHeight(88.0f);
			CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() + FVector(0, 0, 22.0f));
			MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));


			CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			if (moveComp != NULL) {
				moveComp->SetUpdatedComponent(RootComponent);
			}
		}
	}
	else
	{
		bProning = !bProning;
		bCrouching = false;
		moveComp->MaxSpeed = 400;
		CapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		RootComponent = CapsuleComponent;
		ProneCapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
		if (moveComp != NULL) {
			moveComp->SetUpdatedComponent(RootComponent);
		}

	}
	ServerDoProne();
}

void AMyPawn::ServerDoProne_Implementation()
{
	if (!bProning) {
		if (!bCrouching) {
			bProning = !bProning;
			bCrouching = false;

			ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			RootComponent = ProneCapsuleComponent;
			UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();
			CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			if (moveComp != NULL) {
				moveComp->SetUpdatedComponent(RootComponent);
			}
		}

		else {
			bProning = !bProning;
			bCrouching = false;

			ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			RootComponent = ProneCapsuleComponent;
			UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();

			MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			CapsuleComponent->SetCapsuleHalfHeight(88.0f);
			CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() + FVector(0, 0, 22.0f));
			MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));


			CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			if (moveComp != NULL) {
				moveComp->SetUpdatedComponent(RootComponent);
			}
		}
	}
	else
	{
		bProning = !bProning;
		bCrouching = false;

		CapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
		RootComponent = CapsuleComponent;
		UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();
		ProneCapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
		if (moveComp != NULL) {
			moveComp->SetUpdatedComponent(RootComponent);
		}

	}

	MulticastDoProne();
}

bool AMyPawn::ServerDoProne_Validate()
{
	return true;
}

void AMyPawn::MulticastDoProne_Implementation()
{
	if (GetLocalRole() == ROLE_SimulatedProxy) {
		if (!bProning) {
			if (!bCrouching) {
				bProning = !bProning;
				bCrouching = false;

				ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
				RootComponent = ProneCapsuleComponent;
				UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();
				CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
				if (moveComp != NULL) {
					moveComp->SetUpdatedComponent(RootComponent);
				}
			}

			else {
				bProning = !bProning;
				bCrouching = false;

				ProneCapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
				RootComponent = ProneCapsuleComponent;
				UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();

				MeshComp->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
				CapsuleComponent->SetCapsuleHalfHeight(88.0f);
				CapsuleComponent->SetWorldLocation(CapsuleComponent->GetComponentLocation() + FVector(0, 0, 22.0f));
				MeshComp->AttachToComponent(CapsuleComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));


				CapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
				if (moveComp != NULL) {
					moveComp->SetUpdatedComponent(RootComponent);
				}
			}
		}
		else
		{
			bProning = !bProning;
			bCrouching = false;

			CapsuleComponent->DetachFromComponent(FDetachmentTransformRules(EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, EDetachmentRule::KeepWorld, true));
			RootComponent = CapsuleComponent;
			UFloatingPawnMovement* moveComp = (UFloatingPawnMovement*)GetMovementComponent();
			ProneCapsuleComponent->AttachToComponent(RootComponent, FAttachmentTransformRules(EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, EAttachmentRule::KeepWorld, false));
			if (moveComp != NULL) {
				moveComp->SetUpdatedComponent(RootComponent);
			}

		}
	}
}

void AMyPawn::DoJump()
{
	if (bOnFloor) {
		bWantsToJump = true;
	}
}



void AMyPawn::PickUp(ASWeapon* wep)
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

void AMyPawn::ServerPickUp_Implementation(ASWeapon* wep)
{
	PickUp(wep);
}

bool AMyPawn::ServerPickUp_Validate(ASWeapon* wep)
{
	return true;
}

void AMyPawn::Drop(ASWeapon* wep)
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

void AMyPawn::ServerDrop_Implementation(ASWeapon* wep)
{
	Drop(wep);
}

bool AMyPawn::ServerDrop_Validate(ASWeapon* wep)
{
	return true;
}


void AMyPawn::MulticastDropEffect_Implementation()
{
	WeaponComp->DetachRootComponentFromParent(true);
}


void AMyPawn::AddtoMesh()
{
	if (WeaponComp) {
		WeaponComp->AttachToComponent(MeshComp, FAttachmentTransformRules::SnapToTargetIncludingScale, "WeaponSocket");
		WeaponComp->SetOwner(this);
	}
}

// Called to bind functionality to input
void AMyPawn::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	Super::SetupPlayerInputComponent(PlayerInputComponent);

	PlayerInputComponent->BindAxis("MoveForward", this, &AMyPawn::MoveForward);
	PlayerInputComponent->BindAxis("MoveRight", this, &AMyPawn::MoveRight);
	PlayerInputComponent->BindAxis("LookUp", this, &AMyPawn::AddControllerPitchInput);
	PlayerInputComponent->BindAxis("LookRight", this, &AMyPawn::AddControllerYawInput);
	PlayerInputComponent->BindAction("Prone", IE_Pressed, this, &AMyPawn::DoProne);
	PlayerInputComponent->BindAction("Crouch", IE_Pressed, this, &AMyPawn::DoCrouch);
	PlayerInputComponent->BindAction("Jump", IE_Pressed, this, &AMyPawn::DoJump);
	


}



void AMyPawn::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const {
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);


	DOREPLIFETIME(AMyPawn, IsDead);
	DOREPLIFETIME(AMyPawn, WeaponComp);
}
