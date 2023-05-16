// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSWeapon.h"
#include "FPSProjectile.h"
#include "FPSProjectileWeapon.generated.h"

/**
 * 
 */
UCLASS()
class MECHAMANIA_API AFPSProjectileWeapon : public AFPSWeapon
{
	GENERATED_BODY()
public:
	virtual void Fire(const FVector& HitTarget) override;

	UPROPERTY(EditAnywhere, Category = "MechaMania|Projectile Properties")
	FName MuzzleSocket;
private:
	UPROPERTY(EditAnywhere, Category = "MechaMania|Projectile Properties")
	TSubclassOf<class AFPSProjectile> ProjectileClass;
};
