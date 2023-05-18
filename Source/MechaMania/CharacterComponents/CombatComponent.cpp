// Fill out your copyright notice in the Description page of Project Settings.


#include "CombatComponent.h"

#include "Components/SphereComponent.h"
#include "Engine/SkeletalMeshSocket.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "MechaMania/Character/FPSCharacter.h"
#include "MechaMania/Weapon/FPSWeapon.h"
#include "MechaMania//PlayerController/FPSPlayerController.h"
#include "MechaMania/HUD/FPSHUD.h"
#include "Net/UnrealNetwork.h"
#include "Kismet/GameplayStatics.h"
#include "DrawDebugHelpers.h"

UCombatComponent::UCombatComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	BaseWalkSpeed = 600.f;
	AimWalkSpeed = 450.f;

}

void UCombatComponent::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME(UCombatComponent, EquippedWeapon);
	DOREPLIFETIME(UCombatComponent, bAiming);
	DOREPLIFETIME(UCombatComponent, bShooting);
}

void UCombatComponent::BeginPlay()
{
	Super::BeginPlay();

	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = BaseWalkSpeed;
	}
		
}

void UCombatComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	SetHUDCrosshairs(DeltaTime);

}

void UCombatComponent::SetAiming(bool bIsAiming)
{
	bAiming = bIsAiming;
	Server_SetAiming(bIsAiming);
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::Server_SetAiming_Implementation(bool bIsAiming)
{
	bAiming = bIsAiming;
	if (Character)
	{
		Character->GetCharacterMovement()->MaxWalkSpeed = bIsAiming ? AimWalkSpeed : BaseWalkSpeed;
	}
}

void UCombatComponent::EquipWeapon(AFPSWeapon* WeaponToEquip)
{
	if (Character == nullptr || WeaponToEquip == nullptr) return;
	EquippedWeapon = WeaponToEquip;
	EquippedWeapon->SetWeaponState(EWeaponState::EWS_Equipped);

	const FTransform PlacementTransform = EquippedWeapon->PlacementTransform * Character->GetMesh()->GetSocketTransform(
				FName("hand_r"));
	EquippedWeapon->SetActorTransform(PlacementTransform, false, nullptr, ETeleportType::TeleportPhysics);
	EquippedWeapon->AttachToComponent(Character->GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("hand_r"));

	EquippedWeapon->WeaponMesh->SetVisibility(true);
	EquippedWeapon->SetOwner(Character);
	
}

void UCombatComponent::OnRep_EquippedWeapon()
{
	if (EquippedWeapon && Character)
	{
		Character->GetCharacterMovement()->bOrientRotationToMovement = false;
		Character->bUseControllerRotationYaw = true;
	}
}

void UCombatComponent::SetShooting(bool bIsShooting)
{
	bShooting = bIsShooting;
	if (bShooting)
	{
		FHitResult HitResult;
		TraceUnderCrossHairs(HitResult);
		Server_Shooting(HitResult.ImpactPoint);
	}
}

void UCombatComponent::Multicast_Shooting_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	if (EquippedWeapon == nullptr) return;
	if (Character)
	{
		Character->PlayFireWeaponMontage();
		EquippedWeapon->Fire(TraceHitTarget);
	}
}

void UCombatComponent::Server_Shooting_Implementation(const FVector_NetQuantize& TraceHitTarget)
{
	Multicast_Shooting(TraceHitTarget);
}


void UCombatComponent::TraceUnderCrossHairs(FHitResult& TraceHitResult)
{
	FVector2d ViewportSize;
	if (GEngine && GEngine->GameViewport)
	{
		GEngine->GameViewport->GetViewportSize(ViewportSize);
	}
	FVector2d CrosshairLocation(ViewportSize.X / 2.f, ViewportSize.Y / 2.f);
	FVector CrosshairWorldPosition;
	FVector CrosshairWorldDirection;
	bool bScreenToWorld = UGameplayStatics::DeprojectScreenToWorld(
		UGameplayStatics::GetPlayerController(this, 0),
		CrosshairLocation,
		CrosshairWorldPosition,
		CrosshairWorldDirection
	);
	if (bScreenToWorld)
	{
		FVector Start = CrosshairWorldPosition;
		FVector End = Start + CrosshairWorldDirection * TRACE_LENGTH;

		GetWorld()->LineTraceSingleByChannel(
			TraceHitResult,
			Start,
			End,
			ECC_Visibility
		);
		/*
		if (!TraceHitResult.bBlockingHit)
		{
			TraceHitResult.ImpactPoint = End;
		}
		else
		{
			
			DrawDebugSphere(
				GetWorld(),
				TraceHitResult.ImpactPoint,
				12.f,
				12.f,
				FColor::Red
			);
			
		}
		*/
	}
}

void UCombatComponent::SetHUDCrosshairs(float DeltaTime)
{
	if (Character == nullptr || Character->Controller == nullptr) return;
	PlayerController = PlayerController == nullptr ? Cast<AFPSPlayerController>(Character->Controller) : PlayerController;

	if (!PlayerController) return;
	HUD = HUD == nullptr ? Cast<AFPSHUD>(PlayerController->GetHUD()) : HUD;

	if (!HUD) return;
	if (EquippedWeapon)
	{
		HUDPackage.CrosshairCenter = EquippedWeapon->CrosshairsCenter;
		HUDPackage.CrosshairLeft = EquippedWeapon->CrosshairsLeft;
		HUDPackage.CrosshairRight = EquippedWeapon->CrosshairsRight;
		HUDPackage.CrosshairTop = EquippedWeapon->CrosshairsTop;
		HUDPackage.CrosshairBottom = EquippedWeapon->CrosshairsBottom;
	}
	else
	{
		HUDPackage.CrosshairCenter = nullptr;
		HUDPackage.CrosshairLeft = nullptr;
		HUDPackage.CrosshairRight = nullptr;
		HUDPackage.CrosshairTop = nullptr;
		HUDPackage.CrosshairBottom = nullptr;
	}

	// Calculate Crosshair Spread based on Player's Velocity
	// [0, 600]  -> [0, 1]
	FVector2D WalkSpeedRange(0.f, Character->GetCharacterMovement()->MaxWalkSpeed);
	FVector2D VelocityMultiplierRange(0.f, 1.f);
	FVector Velocity = Character->GetVelocity();
	Velocity.Z = 0.f;
	CrosshariVelocityFactor = FMath::GetMappedRangeValueClamped(WalkSpeedRange, VelocityMultiplierRange, Velocity.Size());

	if (Character->GetCharacterMovement()->IsFalling())
	{
		CrosshariInAirFactor = FMath::FInterpTo(CrosshariInAirFactor, 2.25f, DeltaTime, 2.25f);
	}
	else
	{
		CrosshariInAirFactor = FMath::FInterpTo(CrosshariInAirFactor, 0.f, DeltaTime, 30.f);
	}
	
	HUDPackage.CrosshairSpread = CrosshariVelocityFactor + CrosshariInAirFactor;
	
	HUD->SetHUDPackage(HUDPackage);
}


