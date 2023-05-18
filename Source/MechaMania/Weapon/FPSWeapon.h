// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "MechaMania/Character/FPSCharacter.h"
#include "Animation/AnimationAsset.h"
#include "FPSWeapon.generated.h"

UENUM(BlueprintType)
enum class EWeaponState : uint8
{
	EWS_Initial UMETA(DisplayName = "Initial State"),
	EWS_Equipped UMETA(DisplayName = "Equipped"),
	EWS_Dropped UMETA(DisplayName = "Dropped"),
	
	EWS_MAX UMETA(DisplayName = "DefaultMAX")
};

USTRUCT(BlueprintType)
struct FIKProperties
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	class UAnimSequence* AnimPose;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float AimOffset = 15.f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FTransform CustomOffsetTransform;
};

UCLASS(Abstract)
class MECHAMANIA_API AFPSWeapon : public AActor
{
	GENERATED_BODY()

public:
	AFPSWeapon();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	void ShowPickupWidget(bool bShowWidget);
	virtual void Fire(const FVector& HitTarget);

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnSphereOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	UFUNCTION()
	virtual void OnSphereEndOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex);

public:
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MechaMania|Weapon Properties")
	class USkeletalMeshComponent* WeaponMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite, Category = "MechaMania|Weapon Properties")
	class USphereComponent* AreaSphere;
	
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "MechaMania|Weapon State")
	class AFPSCharacter* CurrentOwner;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MechaMania|Configurations")
	FIKProperties IKProperties;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "MechaMania|Configurations")
	FTransform PlacementTransform;
private:
	UPROPERTY(ReplicatedUsing = OnRep_WeaponState, VisibleAnywhere, Category = "MechaMania|Weapon State")
	EWeaponState WeaponState;

	UFUNCTION()
	void OnRep_WeaponState();

	UPROPERTY(VisibleAnywhere, Category = "MechaMania|Weapon Properties")
	class UWidgetComponent* PickupWidget;

	UPROPERTY(EditAnywhere, Category = "MechaMania|Weapon Animations")
	class UAnimationAsset* FireAnimation;

	UPROPERTY(EditAnywhere, Category = "MechaMania|Casing Properties")
	TSubclassOf<class AFPSCasing> CasingClass;

public:
	/**
	 * Textures for the weapon crosshairs
	 */
	UPROPERTY(EditAnywhere, Category = "MechaMania|Crosshair Properties")
	class UTexture2D* CrosshairsCenter;

	UPROPERTY(EditAnywhere, Category = "MechaMania|Crosshair Properties")
	UTexture2D* CrosshairsLeft;
	UPROPERTY(EditAnywhere, Category = "MechaMania|Crosshair Properties")
	UTexture2D* CrosshairsRight;
	UPROPERTY(EditAnywhere, Category = "MechaMania|Crosshair Properties")
	UTexture2D* CrosshairsTop;
	UPROPERTY(EditAnywhere, Category = "MechaMania|Crosshair Properties")
	UTexture2D* CrosshairsBottom;
public:
	void SetWeaponState(EWeaponState State);
	FORCEINLINE USphereComponent* GetAreaSphere() const { return AreaSphere; }
	FORCEINLINE USkeletalMeshComponent* GetWeaponMesh() const { return WeaponMesh; }
};
