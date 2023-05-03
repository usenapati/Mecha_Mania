// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAnimInstance.h"

#include "MechaMania/Character/FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "Kismet/KismetMathLibrary.h"

UFPSAnimInstance::UFPSAnimInstance()
{
}

void UFPSAnimInstance::NativeBeginPlay()
{
	Super::NativeBeginPlay();

	Character = Cast<AFPSCharacter>(TryGetPawnOwner());
	if (Character)
	{
		Mesh = Character->GetMesh();
		Character->CurrentWeaponChangedDelegate.AddDynamic(this, &UFPSAnimInstance::CurrentWeaponChanged);
		CurrentWeaponChanged(Character->CurrentWeapon, nullptr);
	}
}

void UFPSAnimInstance::NativeUpdateAnimation(float DeltaTime)
{
	Super::NativeUpdateAnimation(DeltaTime);

	if (!Character) return;

	SetVars(DeltaTime);
	CalculateWeaponSway(DeltaTime);
}

void UFPSAnimInstance::CurrentWeaponChanged(AFPSWeapon* NewWeapon, const AFPSWeapon* OldWeapon)
{
	CurrentWeapon = NewWeapon;
	if (CurrentWeapon)
	{
		IKProperties = CurrentWeapon->IKProperties;
	}
	else
	{
	}
}

void UFPSAnimInstance::SetVars(const float DeltaTime)
{
	if (!Character) return;
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	Speed = Velocity.Size();

	bIsInAir = Character->GetCharacterMovement()->IsFalling();
	bIsAccelerating = Character->GetCharacterMovement()->GetCurrentAcceleration().Size() > 0 ? true : false;
	bWeaponEquipped = Character->IsWeaponEquipped();
	bIsCrouched = Character->bIsCrouched;
	bIsAiming = Character->IsAiming();

	// Offset Yaw for Strafing
	FRotator AimRotation = Character->GetBaseAimRotation();
	FRotator MovementRotation = UKismetMathLibrary::MakeRotFromX(Character->GetVelocity());
	FRotator DeltaRot = UKismetMathLibrary::NormalizedDeltaRotator(MovementRotation, AimRotation);
	DeltaRotation = FMath::RInterpTo(DeltaRotation, DeltaRot, DeltaTime, 12.f);
	YawOffset = DeltaRotation.Yaw;

	// Lean
	CharacterRotationLastFrame = CharacterRotation;
	CharacterRotation = Character->GetActorRotation();
	const FRotator Delta = UKismetMathLibrary::NormalizedDeltaRotator(CharacterRotation, CharacterRotationLastFrame);
	const float Target = Delta.Yaw / DeltaTime;
	const float Interp = FMath::FInterpTo(Lean, Target, DeltaTime, 6.f);
	Lean = FMath::Clamp(Interp, -90.f, 90.f);
	
	// Camera Vars
	FVector CameraLocation;
	if (Character->IsInFirstPersonPerspective())
	{
		CameraLocation = Character->Get1PCamera_Implementation()->GetComponentLocation();
	}
	else
	{
		CameraLocation = Character->Get3PCamera_Implementation()->GetComponentLocation();
	}
	CameraTransform = FTransform(Character->GetBaseAimRotation(), CameraLocation); //

	const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->
		GetSocketTransform(FName("ik_hand_root"));
	RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);
	
}

void UFPSAnimInstance::CalculateWeaponSway(const float DeltaTime)
{
}
