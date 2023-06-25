// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "Camera/CameraComponent.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "FPSCharacter.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCurrentWeaponChangedDelegate, class AFPSWeapon*, CurrentWeapon,
                                             const class AFPSWeapon*, OldWeapon);
class UCameraComponent;
class USpringArmComponent;

UCLASS()
class MECHAMANIA_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()
public:
	AFPSCharacter();
	virtual void Tick(float DeltaTime) override;
	virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;

	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "MechaMania|FPSCharacter")
	bool bStartInFirstPersonPerspective;

	UFUNCTION(BlueprintCallable, Category = "MechaMania|FPSCharacter")
	virtual bool IsInFirstPersonPerspective() const;

protected:
	virtual void BeginPlay() override;
	virtual void PostInitializeComponents() override;
	
	/** Camera Properties */
	UPROPERTY(BlueprintReadOnly, Category = "MechaMania|Camera")
	float StartingThirdPersonCameraBoomArmLength;

	UPROPERTY(BlueprintReadOnly, Category = "MechaMania|Camera")
	FVector StartingThirdPersonCameraBoomArmLocation;

	UPROPERTY(BlueprintReadOnly, Category = "MechaMania|Camera")
	bool bIsFirstPersonPerspective;

	UPROPERTY(BlueprintReadOnly, Category = "MechaMania|Camera")
	float Default1PFOV;

	UPROPERTY(BlueprintReadOnly, Category = "MechaMania|Camera")
	float Default3PFOV;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MechaMania|Camera")
	UCameraComponent* FirstPersonCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MechaMania|Camera")
	UCameraComponent* ThirdPersonCamera;
	
	/** Camera boom positioning the camera behind the character */
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MechaMania|Camera")
	USpringArmComponent* CameraBoom;

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MechaMania|Camera")
	UCameraComponent* Get1PCamera();

	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "MechaMania|Camera")
	UCameraComponent* Get3PCamera();
	
	void TogglePerspective();

	void SetPerspective(bool Is1PPerspective);

	// Widgets
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, meta = (AllowPrivateAccess = "true"))
	class UWidgetComponent* OverheadWidget;

public:
	/** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="MechaMania|Input")
	float BaseTurnRate;

#pragma region /** Locomotion */
	/** Called for forwards/backward input */
	void MoveForward(float Value);

	/** Called for side to side input */
	void MoveRight(float Value);

	/** 
	 * Called via input to turn at a given rate. 
	 * @param Rate  This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void TurnAtRate(float Rate);

	/**
	 * Called via input to turn look up/down at a given rate. 
	 * @param Rate  This is a normalized rate, i.e. 1.0 means 100% of desired turn rate
	 */
	void LookUpAtRate(float Rate);
#pragma endregion

#pragma region /** Input */

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MechaMania|Animation")
	bool IsFirstPerson;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MechaMania|Animation")
	bool IsShooting;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MechaMania|Animation")
	bool IsADS;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MechaMania|Animation")
	bool IsJumping;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MechaMania|Animation")
	bool IsCrouching;
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MechaMania|Animation")
	bool IsInteracting;
	

protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MechaMania|Enhanced Input")
	class UInputMappingContext* InputMapping;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "MechaMania|Enhanced Input")
	class UInputConfigData* InputActions;
	// Movement and Action Inputs
	void Move(const FInputActionValue& Value); // Handle move input
	void Look(const FInputActionValue& Value); // Handle look input
	void JumpInput(const FInputActionValue& Value);
	void CrouchInput(const FInputActionValue& Value);
	void ChangeCamera(const FInputActionValue& Value); // Switch between third person and first person camera
	void Interact(const FInputActionValue& Value);
	
	// Weapon Inputs
	void ADS(const FInputActionValue& Value);
	void Shoot(const FInputActionValue& Value);
	// Handle Weapon Swapping
	void NextWeapon(const FInputActionValue& Value);
	void LastWeapon(const FInputActionValue& Value);

#pragma endregion

#pragma region /** Animation */
protected:
	float AO_Yaw;
	float InterpAO_Yaw;
	float AO_Pitch;
	FRotator StartingAimRotation;

	// Turning in place
	void AimOffset(float DeltaTime);

public:
	FORCEINLINE float GetAO_Yaw() const { return AO_Yaw; }
	FORCEINLINE float GetAO_Pitch() const { return AO_Pitch; }
	AFPSWeapon* GetEquippedWeapon();	
#pragma endregion

#pragma region /** Networked Weapons System */

protected:
	// Weapon classes spawned by default
	UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "MechaMania|Configurations")
	TArray<TSubclassOf<class AFPSWeapon>> DefaultWeapons;

	UPROPERTY(VisibleAnywhere)
	class UCombatComponent* CombatComponent;
public:
	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "MechaMania|Inventory")
	TArray<class AFPSWeapon*> Weapons;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentWeapon, Category = "MechaMania|Inventory")
	class AFPSWeapon* CurrentWeapon;

	// Called whenever Current Weapon is changed
	UPROPERTY(BlueprintAssignable, Category = "MechaMania|Delegates")
	FCurrentWeaponChangedDelegate CurrentWeaponChangedDelegate;

	UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "MechaMania|Inventory")
	int32 CurrentIndex = 0;

	UFUNCTION(BlueprintCallable, Category = "MechaMania|FPSCharacter")
	virtual void EquipWeapon(const int32 Index);

	void PlayFireWeaponMontage();

protected:
	UFUNCTION()
	virtual void OnRep_CurrentWeapon(const class AFPSWeapon* OldWeapon);

	UFUNCTION(Server, Reliable)
	void Server_SetCurrentWeapon(class AFPSWeapon* Weapon);
	virtual void Server_SetCurrentWeapon_Implementation(class AFPSWeapon* NewWeapon);

	UFUNCTION(Server, Reliable)
	void Server_EquipWeapon();
	virtual void Server_EquipWeapon_Implementation();
	
	UPROPERTY(EditAnywhere, Category = "MechaMania|Anim")
	class UAnimMontage* FireRifleWeaponMontage;

public:
	void SetOverlappingWeapon(AFPSWeapon* Weapon);
	bool IsWeaponEquipped();
	bool IsAiming();

private:
	UPROPERTY(ReplicatedUsing = OnRep_OverlappingWeapon)
	class AFPSWeapon* OverlappingWeapon;

	UFUNCTION()
	void OnRep_OverlappingWeapon(AFPSWeapon* LastWeapon);
#pragma endregion

#pragma region /** Online Subsystem */
	/*
public:
	// Pointer to Online Session Interface
	IOnlineSessionPtr OnlineSessionInterface;

protected:
	UFUNCTION(BlueprintCallable)
	void CreateGameSession();

	UFUNCTION(BlueprintCallable)
	void JoinGameSession();

	void OnCreateSessionComplete(FName SessionName, bool bWasSuccessful);
	void OnFindSessionsComplete(bool bWasSuccessful);
	void OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result);

private:
	FOnCreateSessionCompleteDelegate CreateSessionCompleteDelegate;
	FOnFindSessionsCompleteDelegate FindSessionsCompleteDelegate;
	TSharedPtr<FOnlineSessionSearch> SessionSearch;
	FOnJoinSessionCompleteDelegate JoinSessionCompleteDelegate;
	*/
#pragma endregion

};
