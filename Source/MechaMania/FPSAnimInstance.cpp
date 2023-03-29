// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSAnimInstance.h"

#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"

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

void UFPSAnimInstance::CurrentWeaponChanged(AWeapon* NewWeapon, const AWeapon* OldWeapon)
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
	CameraTransform = FTransform(Character->GetBaseAimRotation(), Character->GetCamera()->GetComponentLocation());

	const FTransform& RootOffset = Mesh->GetSocketTransform(FName("root"), RTS_Component).Inverse() * Mesh->GetSocketTransform(FName("ik_hand_root"));
	RelativeCameraTransform = CameraTransform.GetRelativeTransform(RootOffset);

}

void UFPSAnimInstance::CalculateWeaponSway(const float DeltaTime)
{
	
}

