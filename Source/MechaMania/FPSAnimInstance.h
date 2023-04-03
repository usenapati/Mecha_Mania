// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "FPSWeapon.h"
#include "Animation/AnimInstance.h"
#include "FPSAnimInstance.generated.h"

/**
 * 
 */
UCLASS()
class MECHAMANIA_API UFPSAnimInstance : public UAnimInstance
{
	GENERATED_BODY()
public:
	UFPSAnimInstance();

protected:
	virtual void NativeBeginPlay() override;
	virtual void NativeUpdateAnimation(float DeltaTime) override;

	virtual void CurrentWeaponChanged(class AFPSWeapon* NewWeapon, const class AFPSWeapon* OldWeapon);
	virtual void SetVars(const float DeltaTime);
	virtual void CalculateWeaponSway(const float DeltaTime);

public:
	/*
	 * REFERENCES
	 */
	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class AFPSCharacter* Character;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class USkeletalMeshComponent* Mesh;

	UPROPERTY(BlueprintReadWrite, Category = "Anim")
	class AFPSWeapon* CurrentWeapon;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	FIKProperties IKProperties;

	/*
	 * IK VARS
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	FTransform CameraTransform;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Anim")
	FTransform RelativeCameraTransform;
};
