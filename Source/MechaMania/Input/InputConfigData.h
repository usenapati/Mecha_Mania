// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "EnhancedInput/Public/InputAction.h"
#include "InputConfigData.generated.h"

/**
 *
 */
UCLASS()
class MECHAMANIA_API UInputConfigData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputMove;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputLook;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputChangeCamera;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputInteract;
	
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputJump;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputCrouch;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputADS;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputShoot;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputNextWeapon;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	UInputAction* InputLastWeapon;
};