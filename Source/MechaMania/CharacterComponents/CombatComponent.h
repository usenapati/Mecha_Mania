// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MechaMania/Character/FPSCharacter.h"
#include "MechaMania/HUD/FPSHUD.h"
#include "MechaMania/Weapon/FPSWeapon.h"
#include "CombatComponent.generated.h"

#define TRACE_LENGTH 80000.f

class AFPSWeapon;

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class MECHAMANIA_API UCombatComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCombatComponent();
	friend class AFPSCharacter;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	
	void EquipWeapon(AFPSWeapon* WeaponToEquip);

protected:
	virtual void BeginPlay() override;
	void SetAiming(bool bIsAiming);

	UFUNCTION(Server, Reliable)
	void Server_SetAiming(bool bIsAiming);

	UFUNCTION()
	void OnRep_EquippedWeapon();

	void SetShooting(bool bIsShooting);

	UFUNCTION(Server, Reliable)
	void Server_Shooting(const FVector_NetQuantize& TraceHitTarget);

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Shooting(const FVector_NetQuantize& TraceHitTarget);

	void TraceUnderCrossHairs(FHitResult& TraceHitResult);

	void SetHUDCrosshairs(float DeltaTime);

private:
	class AFPSCharacter* Character;
	class AFPSPlayerController* PlayerController;
	class AFPSHUD* HUD;
	FHUDPackage HUDPackage;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AFPSWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(EditAnywhere)
	float BaseWalkSpeed;

	UPROPERTY(EditAnywhere)
	float AimWalkSpeed;

	UPROPERTY(Replicated)
	bool bShooting;

	/**
	 * HUD and Crosshairs
	 */
	float CrosshariVelocityFactor;
	float CrosshariInAirFactor;

public:	
	

		
};
