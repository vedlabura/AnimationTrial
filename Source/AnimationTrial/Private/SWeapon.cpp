// Fill out your copyright notice in the Description page of Project Settings.


#include "SWeapon.h"
#include "Components/SphereComponent.h"
#include <Components/SkeletalMeshComponent.h>
#include <DrawDebugHelpers.h>
#include <Kismet/GameplayStatics.h>



// Sets default values
ASWeapon::ASWeapon()
{

	MeshComp = CreateDefaultSubobject<USkeletalMeshComponent>(TEXT("MeshComp"));
	PickUpSphere = CreateDefaultSubobject<USphereComponent>(TEXT("PickUPSphere"));
	RootComponent = MeshComp;
	PickUpSphere->SetupAttachment(MeshComp);


	SetReplicates(true);
	SetReplicateMovement(true);

	MuzzleSocketName = "MuzzleSocket";

}

void ASWeapon::Fire()
{
	if (this->GetLocalRole() == ROLE_Authority)
	{
		AActor* MyOwner = GetOwner();
		if (MyOwner) {
			FVector EyeLocation;
			FRotator EyeRotation;
			MyOwner->GetInstigatorController()->GetPlayerViewPoint(EyeLocation, EyeRotation);
			FVector TraceEnd = EyeLocation + (EyeRotation.Vector() * 10000);

			FCollisionQueryParams QueryParams;
			QueryParams.AddIgnoredActor(MyOwner);
			QueryParams.AddIgnoredActor(this);
			QueryParams.bTraceComplex = true;

			FHitResult Hit;

			if (GetWorld()->LineTraceSingleByChannel(Hit, EyeLocation, TraceEnd, ECC_Visibility, QueryParams)) {

				AActor* HitActor = Hit.GetActor();



				FVector start = MeshComp->GetSocketLocation(MuzzleSocketName);


				FHitResult MuzzleHit;
				if (GetWorld()->LineTraceSingleByChannel(MuzzleHit, start, Hit.Location, ECC_Visibility, QueryParams)) {
					if (MuzzleHit.Actor == Hit.Actor) {
						if (ImpactEffect) {
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
						}

						DrawDebugLine(GetWorld(), start, Hit.Location, FColor::White, true, 1.0f, 0, 1.0f);
						UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, EyeRotation.Vector(), Hit, MyOwner->GetInstigatorController(), this, DamageType);

					}
					else {
						if (ImpactEffect) {
							UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, MuzzleHit.ImpactPoint, MuzzleHit.ImpactNormal.Rotation());
						}
						DrawDebugLine(GetWorld(), start, MuzzleHit.Location, FColor::White, true, 1.0f, 0, 1.0f);
						UGameplayStatics::ApplyPointDamage(MuzzleHit.GetActor(), 20.0f, EyeRotation.Vector(), MuzzleHit, MyOwner->GetInstigatorController(), this, DamageType);

					}
				}
				else
				{
					if (ImpactEffect) {
						UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactEffect, Hit.ImpactPoint, Hit.ImpactNormal.Rotation());
					}

					DrawDebugLine(GetWorld(), start, Hit.Location, FColor::White, true, 1.0f, 0, 1.0f);
					UGameplayStatics::ApplyPointDamage(HitActor, 20.0f, EyeRotation.Vector(), Hit, MyOwner->GetInstigatorController(), this, DamageType);

				}
			}

			if (MuzzleEffect) {
				UGameplayStatics::SpawnEmitterAttached(MuzzleEffect, MeshComp, MuzzleSocketName);
			}
		}
	}
	else
	{
		ServerFire();
	}
}

void ASWeapon::ServerFire_Implementation()
{
	Fire();
}

bool ASWeapon::ServerFire_Validate()
{
	return true;
}

// Called when the game starts or when spawned
void ASWeapon::BeginPlay()
{
	Super::BeginPlay();
	
}
