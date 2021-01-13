// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "SWeapon.h"
#include "SCharacter.generated.h"


class UCameraComponent;
class USpringArmComponent;
class USHealthComponent;

UCLASS()
class ANIMATIONTRIAL_API ASCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this character's properties
	ASCharacter();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(Replicated,VisibleAnywhere, BlueprintReadOnly);
	bool Crouching;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly);
	bool Proning;

	

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite);
	bool Firing;

	UPROPERTY(ReplicatedUsing = AddtoMesh, VisibleAnywhere, BlueprintReadWrite, Category = "Components");
	ASWeapon* WeaponComp;

	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadWrite, Category = "Components");
	USkeletalMeshComponent* MeshComp;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "Components");
	USHealthComponent* HealthComp;

	UFUNCTION(BlueprintCallable,Category="ReplicatedFunction")
	void AddtoMesh();







	



	void MoveForward(float Value);
	
	void MoveRight(float Value);
	
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoCrouch();
	
	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDoCrouch();

	UFUNCTION(BlueprintCallable,Category="Movement")
	void Prone();
	UFUNCTION(BlueprintCallable, Category = "Movement")
	void DoJump();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerDoJump();

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerProne();

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void PickUp(ASWeapon* wep);

	UFUNCTION(Server, Reliable, WithValidation)
	void ServerPickUp(ASWeapon* wep);

	UFUNCTION(BlueprintCallable, Category = "Movement")
		void Drop(ASWeapon* wep);

	UFUNCTION(Server,Reliable, WithValidation)
		void ServerDrop(ASWeapon* wep);
	
	UFUNCTION(NetMulticast, Reliable)
	void MulticastDropEffect();

public:	


	UPROPERTY(Replicated, VisibleAnywhere, BlueprintReadOnly);
	bool IsDead;
	// Called every frame
	virtual void Tick(float DeltaTime) override;

	// Called to bind functionality to input
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


};
