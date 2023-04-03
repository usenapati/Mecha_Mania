// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "FPSCharacter.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FCurrentWeaponChangedDelegate, class AFPSWeapon*, CurrentWeapon, const class AFPSWeapon*, OldWeapon);

UCLASS()
class MECHAMANIA_API AFPSCharacter : public ACharacter
{
	GENERATED_BODY()

     /** Follow camera */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    class UCameraComponent* Camera;

    /** Camera boom positioning the camera behind the character */
    //UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Camera, meta = (AllowPrivateAccess = "true"))
    //class USpringArmComponent* CameraBoom;

public:
	AFPSCharacter();

    /** Base turn rate, in deg/sec. Other scaling may affect final turn rate. */
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category=Input)
    float TurnRateGamepad;
 
protected:
    virtual void BeginPlay() override;
    virtual void GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const override;
    

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
    
 
public:
    virtual void SetupPlayerInputComponent(class UInputComponent* PlayerInputComponent) override;


    /** Returns CameraBoom subobject **/
    //FORCEINLINE class USpringArmComponent* GetCameraBoom() const { return CameraBoom; }
    /** Returns FollowCamera subobject **/
    FORCEINLINE class UCameraComponent* GetCamera() const { return Camera; }

protected:
    // Weapon classes spawned by default
    UPROPERTY(EditDefaultsOnly, BlueprintReadWrite, Category = "Configurations")
	TArray<TSubclassOf<class AFPSWeapon>> DefaultWeapons;

public:
    UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Replicated, Category = "State")
	TArray<class AFPSWeapon*> Weapons;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, ReplicatedUsing = OnRep_CurrentWeapon, Category = "State")
	class AFPSWeapon* CurrentWeapon;

    // Called whenever Current Weapon is changed
    UPROPERTY(BlueprintAssignable, Category = "Delegates")
    FCurrentWeaponChangedDelegate CurrentWeaponChangedDelegate;

    UPROPERTY(VisibleInstanceOnly, BlueprintReadWrite, Category = "State")
	int32 CurrentIndex = 0;

    UFUNCTION(BlueprintCallable, Category = "Character")
    virtual void EquipWeapon(const int32 Index);

protected:
    UFUNCTION()
	virtual void OnRep_CurrentWeapon(const class AFPSWeapon* OldWeapon);

    UFUNCTION(Server, Reliable)
	void Server_SetCurrentWeapon(class AFPSWeapon* Weapon);
    virtual void Server_SetCurrentWeapon_Implementation(class AFPSWeapon* NewWeapon);

#pragma region /** Input */
protected:
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
    class UInputMappingContext* InputMapping;
 
    UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Enhanced Input")
    class UInputConfigData* InputActions;

    // Handle move input
    void Move(const FInputActionValue& Value);
    // Handle look input
    void Look(const FInputActionValue& Value);

    // Handle Weapon Swapping
    void NextWeapon(const FInputActionValue& Value);
    void LastWeapon(const FInputActionValue& Value);

#pragma endregion
};
