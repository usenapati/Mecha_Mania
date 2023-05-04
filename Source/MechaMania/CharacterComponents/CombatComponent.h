// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "MechaMania/Character/FPSCharacter.h"
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
	void Server_Shooting();

	UFUNCTION(NetMulticast, Reliable)
	void Multicast_Shooting();

	void TraceUnderCrossHairs(FHitResult& TraceHitResult);

private:
	class AFPSCharacter* Character;

	UPROPERTY(ReplicatedUsing = OnRep_EquippedWeapon)
	AFPSWeapon* EquippedWeapon;

	UPROPERTY(Replicated)
	bool bAiming;

	UPROPERTY(Replicated)
	bool bShooting;
public:	
	

		
};
