// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/DamageType.h"
#include "GameFramework/Controller.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/ArrowComponent.h"
#include "Engine/CollisionProfile.h"
#include "SWeapon.h"
#include "Components/CapsuleComponent.h"
#include "GameFramework/FloatingPawnMovement.h"
#include "MyPawn.generated.h"


class UCapsuleComponent;
class USHealthComponent;

UCLASS()





class ANIMATIONTRIAL_API AMyPawn : public APawn
{
	GENERATED_BODY()

public:
	// Sets default values for this pawn's properties
	AMyPawn();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
		bool bOnFloor;


	FVector NewLoc;
	FVector OldLoc;

	bool bWantsToJump;

	FVector velInAir;

	FVector_NetQuantizeNormal FloorNormal;

	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly);
	bool bCrouching;

	UPROPERTY( VisibleAnywhere, BlueprintReadOnly);
	bool bProning;

	UPROPERTY( VisibleAnywhere, BlueprintReadWrite, Category = "Components");
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* CapsuleComponent;

	UPROPERTY(Category = Character, VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
		UCapsuleComponent* ProneCapsuleComponent;

	UFUNCTION(Server, Unreliable)
		void ServerMove(FVector Vel, FVector oldLoc, FVector newLoc, FRotator Rot, float timeStamp);

	UPROPERTY()
		UFloatingPawnMovement* Movem;



public:


	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly);
	bool IsDead;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite);
	bool Firing;

	UPROPERTY(ReplicatedUsing = AddtoMesh, VisibleAnywhere, BlueprintReadWrite, Category = "Components");
	ASWeapon* WeaponComp;

	UPROPERTY(Replicated,VisibleAnywhere, BlueprintReadWrite, Category = "Components");
	USHealthComponent* HealthComp;



	// Called every frame
	virtual void Tick(float DeltaTime) override;



	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


	void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const;

	void MoveForward(float Value);

	void MoveRight(float Value);


	void CheckFloor();



	UFUNCTION(BlueprintCallable, Category = "ReplicatedFunction")
		void AddtoMesh();
	UFUNCTION(BlueprintCallable, Category = "Movement")
		void PickUp(ASWeapon* wep);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerPickUp(ASWeapon* wep);

	UFUNCTION(BlueprintCallable, Category = "Movement")
		void Drop(ASWeapon* wep);

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDrop(ASWeapon* wep);

	UFUNCTION(NetMulticast, Reliable)
		void MulticastDropEffect();




	UFUNCTION(BlueprintCallable, Category = "Movement")
		void DoCrouch();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDoCrouch();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastDoCrouch();

	UFUNCTION(BlueprintCallable, Category = "Movement")
		void DoProne();

	UFUNCTION(Server, Reliable, WithValidation)
		void ServerDoProne();

	UFUNCTION(NetMulticast, Reliable)
		void MulticastDoProne();

	UFUNCTION(BlueprintCallable, Category = "Movement")
		void DoJump();
// 
// 	UFUNCTION(Server, Reliable, WithValidation)
// 		void ServerDoJump();
// 
// 	UFUNCTION(NetMulticast, Reliable)
// 		void MulticastDoJump();
//

	
};
