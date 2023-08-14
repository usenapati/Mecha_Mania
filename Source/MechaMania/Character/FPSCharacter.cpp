// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "MechaMania/Input/InputConfigData.h"
#include "MechaMania/Weapon/FPSWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "MechaMania/CharacterComponents/CombatComponent.h"
#include "MechaMania/Anim/FPSAnimInstance.h"
#include "Components/WidgetComponent.h"
#include "Interfaces/OnlineSessionInterface.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetMathLibrary.h"

AFPSCharacter::AFPSCharacter()
{
	/**
	// Set size for collision capsule
	GetCapsuleComponent()->InitCapsuleSize(42.f, 96.0f);

	// set our turn rate for input
	TurnRateGamepad = 50.f;

	// Don't rotate when the controller rotates. Let that just affect the camera.
	bUseControllerRotationPitch = false;
	bUseControllerRotationYaw = false;
	bUseControllerRotationRoll = false;

	// Configure character movement
	GetCharacterMovement()->bOrientRotationToMovement = true; // Character moves in the direction of input...
	GetCharacterMovement()->RotationRate = FRotator(0.0f, 500.0f, 0.0f); // ...at this rotation rate

	// Note: For faster iteration times these variables, and many more, can be tweaked in the Character Blueprint
	// instead of recompiling to adjust them
	GetCharacterMovement()->JumpZVelocity = 700.f;
	GetCharacterMovement()->AirControl = 0.35f;
	GetCharacterMovement()->MaxWalkSpeed = 500.f;
	GetCharacterMovement()->MinAnalogWalkSpeed = 20.f;
	GetCharacterMovement()->BrakingDecelerationWalking = 2000.f;
	**/
	PrimaryActorTick.bCanEverTick = true;
	bStartInFirstPersonPerspective = true;
	bIsFirstPersonPerspective = false;
	Default1PFOV = 90.0f;
	Default3PFOV = 90.0f;
	BaseTurnRate = 1.f;

	CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
	CameraBoom->SetupAttachment(GetMesh());
	//Setting default properties of the SpringArmComp
	CameraBoom->bUsePawnControlRotation = true;
	CameraBoom->bEnableCameraLag = true;
	CameraBoom->TargetArmLength = 300.0f;

	ThirdPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("ThirdPersonCamera"));
	ThirdPersonCamera->AttachToComponent(CameraBoom, FAttachmentTransformRules::KeepRelativeTransform,
	                                     USpringArmComponent::SocketName);
	ThirdPersonCamera->bUsePawnControlRotation = false;
	ThirdPersonCamera->FieldOfView = Default3PFOV;

	FirstPersonCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FirstPersonCamera"));
	FirstPersonCamera->SetupAttachment(GetMesh(), FName("head"));
	FirstPersonCamera->bUsePawnControlRotation = true;
	FirstPersonCamera->FieldOfView = Default1PFOV;

	GetCharacterMovement()->bOrientRotationToMovement = true;

	OverheadWidget = CreateDefaultSubobject<UWidgetComponent>(TEXT("OverheadWidget"));
	OverheadWidget->SetupAttachment(RootComponent);

	CombatComponent = CreateDefaultSubobject<UCombatComponent>(TEXT("CombatComponent"));
	CombatComponent->SetIsReplicated(true);

	GetCharacterMovement()->NavAgentProps.bCanCrouch = true;
	GetCharacterMovement()->NavAgentProps.bCanJump = true;

	GetCapsuleComponent()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	GetMesh()->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);

	TurningInPlace = ETurningInPlace::ETIP_NotTurning;

	NetUpdateFrequency = 66.f;
	MinNetUpdateFrequency = 33.f;
}

bool AFPSCharacter::IsInFirstPersonPerspective() const
{
	return bIsFirstPersonPerspective;
}

void AFPSCharacter::BeginPlay()
{
	Super::BeginPlay();

	if (HasAuthority())
	{
		for (const TSubclassOf<AFPSWeapon>& WeaponClass : DefaultWeapons)
		{
			if (!WeaponClass) continue;
			FActorSpawnParameters Params;
			Params.Owner = this;
			AFPSWeapon* SpawnedWeapon = GetWorld()->SpawnActor<AFPSWeapon>(WeaponClass, Params);
			const int32 Index = Weapons.Add(SpawnedWeapon);
			if (Index == CurrentIndex)
			{
				CurrentWeapon = SpawnedWeapon;
				OnRep_CurrentWeapon(nullptr);
			}
		}
	}
	APlayerController* PC = Cast<APlayerController>(GetController());
	// Set Initial Perspective
	if (PC && PC->IsLocalController())
	{
		if (bStartInFirstPersonPerspective)
		{
			ThirdPersonCamera->Deactivate();
			FirstPersonCamera->Activate();
			PC->SetViewTarget(this);
		}
		else
		{
			FirstPersonCamera->Deactivate();
			ThirdPersonCamera->Activate();
			PC->SetViewTarget(this);
		}
	}
}

void AFPSCharacter::PostInitializeComponents()
{
	Super::PostInitializeComponents();

	if (CombatComponent)
	{
		CombatComponent->Character = this;
	}
	StartingThirdPersonCameraBoomArmLength = CameraBoom->TargetArmLength;
	StartingThirdPersonCameraBoomArmLocation = CameraBoom->GetRelativeLocation();
}

void AFPSCharacter::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	AimOffset(DeltaTime);
}

UCameraComponent* AFPSCharacter::Get1PCamera_Implementation()
{
	return FirstPersonCamera;
}

UCameraComponent* AFPSCharacter::Get3PCamera_Implementation()
{
	return ThirdPersonCamera;
}

void AFPSCharacter::TogglePerspective()
{
	bIsFirstPersonPerspective = !bIsFirstPersonPerspective;
	SetPerspective(bIsFirstPersonPerspective);
}

void AFPSCharacter::SetPerspective(bool Is1PPerspective)
{
	if (!IsValid(FirstPersonCamera))
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("FP Cam is not valid"))
		);
		return;
	}
	if (!IsValid(ThirdPersonCamera))
	{
		GEngine->AddOnScreenDebugMessage(
			-1,
			15.f,
			FColor::Red,
			FString::Printf(TEXT("TP Cam is not valid"))
		);
		return;
	}

	APlayerController* PC = Cast<APlayerController>(GetController());
	if (PC && PC->IsLocalController())
	{
		if (Is1PPerspective)
		{
			//ThirdPersonCamera->SetActive(false);
			//FirstPersonCamera->SetActive(true);
			ThirdPersonCamera->Deactivate();
			FirstPersonCamera->Activate();
			PC->SetViewTarget(this);
		}
		else
		{
			//FirstPersonCamera->SetActive(false);
			//ThirdPersonCamera->SetActive(true);
			FirstPersonCamera->Deactivate();
			ThirdPersonCamera->Activate();
			PC->SetViewTarget(this);
		}
	}
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Get the player controller
	if (APlayerController* PC = Cast<APlayerController>(GetController()))
	{
		UE_LOG(LogTemp, Warning, TEXT("Successfully cast APlayerController"));
		// Get the local player subsystem
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(
			PC->GetLocalPlayer());
		// Clear out existing mapping, and add our mapping
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(InputMapping, 0);
	}

	// Get the EnhancedInputComponent
	if (UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(PlayerInputComponent))
	{
		UE_LOG(LogTemp, Warning, TEXT("Successfully cast UEnhancedInputComponent"));
		// Bind the actions
		PEI->BindAction(InputActions->InputMove, ETriggerEvent::Triggered, this, &AFPSCharacter::Move);
		PEI->BindAction(InputActions->InputLook, ETriggerEvent::Triggered, this, &AFPSCharacter::Look);
		PEI->BindAction(InputActions->InputNextWeapon, ETriggerEvent::Triggered, this, &AFPSCharacter::NextWeapon);
		PEI->BindAction(InputActions->InputLastWeapon, ETriggerEvent::Triggered, this, &AFPSCharacter::LastWeapon);
		PEI->BindAction(InputActions->InputShoot, ETriggerEvent::Triggered, this, &AFPSCharacter::Shoot);
		PEI->BindAction(InputActions->InputADS, ETriggerEvent::Triggered, this, &AFPSCharacter::ADS);
		PEI->BindAction(InputActions->InputChangeCamera, ETriggerEvent::Triggered, this, &AFPSCharacter::ChangeCamera);
		PEI->BindAction(InputActions->InputJump, ETriggerEvent::Triggered, this, &AFPSCharacter::JumpInput);
		PEI->BindAction(InputActions->InputCrouch, ETriggerEvent::Triggered, this, &AFPSCharacter::CrouchInput);
		PEI->BindAction(InputActions->InputInteract, ETriggerEvent::Triggered, this, &AFPSCharacter::Interact);
	}
}

void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFPSCharacter, Weapons, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacter, CurrentWeapon, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacter, OverlappingWeapon, COND_OwnerOnly);
}

void AFPSCharacter::OnRep_CurrentWeapon(const AFPSWeapon* OldWeapon)
{
	if (CurrentWeapon)
	{
		if (!CurrentWeapon->CurrentOwner)
		{
			const FTransform PlacementTransform = CurrentWeapon->PlacementTransform * GetMesh()->GetSocketTransform(
				FName("hand_r"));
			CurrentWeapon->SetActorTransform(PlacementTransform, false, nullptr, ETeleportType::TeleportPhysics);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("hand_r"));

			CurrentWeapon->WeaponMesh->SetVisibility(true);
			CurrentWeapon->CurrentOwner = this;
		}

		CurrentWeapon->WeaponMesh->SetVisibility(true);
	}

	if (OldWeapon)
	{
		OldWeapon->WeaponMesh->SetVisibility(false);
	}

	CurrentWeaponChangedDelegate.Broadcast(CurrentWeapon, OldWeapon);
}

void AFPSCharacter::Move(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D MoveValue = Value.Get<FVector2D>();
		const FRotator MovementRotation(0, Controller->GetControlRotation().Yaw, 0);

		// Forward/Backward direction
		if (MoveValue.Y != 0.f)
		{
			// Get forward vector
			const FVector Direction = MovementRotation.RotateVector(FVector::ForwardVector);
			//UE_LOG(LogTemp, Warning, TEXT("Input Action Value %s (magnitude %f)"),
			//	*Value.ToString(), Value.GetMagnitude());
			AddMovementInput(Direction, MoveValue.Y);
		}

		// Right/Left direction
		if (MoveValue.X != 0.f)
		{
			// Get right vector
			const FVector Direction = MovementRotation.RotateVector(FVector::RightVector);
			//UE_LOG(LogTemp, Warning, TEXT("Input Action Value %s (magnitude %f)"),
			//	*Value.ToString(), Value.GetMagnitude());
			AddMovementInput(Direction, MoveValue.X);
		}
	}
}

void AFPSCharacter::Look(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const FVector2D LookValue = Value.Get<FVector2D>();
		if (IsFirstPerson)
		{
			if (LookValue.X != 0.f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Input Action Value %s (magnitude %f)"),
				//	*Value.ToString(), Value.GetMagnitude());
				AddControllerYawInput(LookValue.X);
			}

			if (LookValue.Y != 0.f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Input Action Value %s (magnitude %f)"),
				//	*Value.ToString(), Value.GetMagnitude());
				AddControllerPitchInput(LookValue.Y);
			}
		}
		else
		{
			if (LookValue.X != 0.f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Input Action Value %s (magnitude %f)"),
				//	*Value.ToString(), Value.GetMagnitude());
				AddControllerYawInput(LookValue.X * BaseTurnRate);
			}

			if (LookValue.Y != 0.f)
			{
				//UE_LOG(LogTemp, Warning, TEXT("Input Action Value %s (magnitude %f)"),
				//	*Value.ToString(), Value.GetMagnitude());
				AddControllerPitchInput(LookValue.Y * BaseTurnRate);
			}
		}
	}
}

void AFPSCharacter::JumpInput(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const bool JumpValue = Value.Get<bool>();
		IsJumping = JumpValue;
		if (IsJumping && !GetCharacterMovement()->IsFalling())
		{
			Jump();
			//UE_LOG(LogTemp, Warning, TEXT("Jump Button Pressed"));
		}
	}
}

void AFPSCharacter::CrouchInput(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const bool CrouchValue = Value.Get<bool>();
		IsCrouching = CrouchValue;
		if (!IsCrouching)
		{
			UnCrouch();
		}
		else
		{
			Crouch();
		}
	}
}

void AFPSCharacter::ChangeCamera(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const bool CameraValue = Value.Get<bool>();
		if (CameraValue)
		{
			TogglePerspective();
		}
	}
}

void AFPSCharacter::Interact(const FInputActionValue& Value)
{
	if (Controller == nullptr) return;

	const bool InteractValue = Value.Get<bool>();
	IsInteracting = InteractValue;

	if (!CombatComponent) return;
	if (IsInteracting)
	{
		if (HasAuthority())
		{
			CombatComponent->EquipWeapon(OverlappingWeapon);
		}
		else
		{
			Server_EquipWeapon();
		}
	}
}

void AFPSCharacter::Jump()
{
	if (bIsCrouched)
	{
		UnCrouch(); // Could do a crouch jump
	}
	else
	{
		Super::Jump();
	}
}

void AFPSCharacter::ADS(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const bool ADSValue = Value.Get<bool>();
		IsADS = ADSValue;
		if (CombatComponent)
		{
			CombatComponent->SetAiming(IsADS);
		}
	}
}

void AFPSCharacter::Shoot(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const bool ShootingValue = Value.Get<bool>();
		IsShooting = ShootingValue;
		if (CombatComponent)
		{
			CombatComponent->SetShooting(IsShooting);
		}
	}
}

void AFPSCharacter::EquipWeapon(const int32 Index)
{
	if (!Weapons.IsValidIndex(Index) || CurrentWeapon == Weapons[Index]) return;
	if (IsLocallyControlled())
	{
		CurrentIndex = Index;

		const AFPSWeapon* OldWeapon = CurrentWeapon;
		CurrentWeapon = Weapons[Index];
		OnRep_CurrentWeapon(OldWeapon);
	}
	else if (!HasAuthority())
	{
		Server_SetCurrentWeapon(Weapons[Index]);
	}
}

void AFPSCharacter::PlayFireWeaponMontage()
{
	if (CombatComponent == nullptr || CombatComponent->EquippedWeapon == nullptr) return;
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (AnimInstance && FireRifleWeaponMontage)
	{
		AnimInstance->Montage_Play(FireRifleWeaponMontage);
		//UE_LOG(LogTemp, Warning, TEXT("Fire Montage"));
		// No Section Name
	}
}

void AFPSCharacter::SetOverlappingWeapon(AFPSWeapon* Weapon)
{
	if (OverlappingWeapon)
	{
		OverlappingWeapon->ShowPickupWidget(false);
	}
	OverlappingWeapon = Weapon;
	if (!IsLocallyControlled()) return;
	if (!OverlappingWeapon) return;
	OverlappingWeapon->ShowPickupWidget(true);
}

bool AFPSCharacter::IsWeaponEquipped()
{
	return (CombatComponent && CombatComponent->EquippedWeapon);
}

bool AFPSCharacter::IsAiming()
{
	return (CombatComponent && CombatComponent->bAiming);
}

void AFPSCharacter::Server_SetCurrentWeapon_Implementation(AFPSWeapon* NewWeapon)
{
	const AFPSWeapon* OldWeapon = CurrentWeapon;
	CurrentWeapon = NewWeapon;
	OnRep_CurrentWeapon(OldWeapon);
}

void AFPSCharacter::Server_EquipWeapon_Implementation()
{
	if (!CombatComponent) return;
	CombatComponent->EquipWeapon(OverlappingWeapon);
}

void AFPSCharacter::OnRep_OverlappingWeapon(AFPSWeapon* LastWeapon)
{
	if (!OverlappingWeapon) return;
	OverlappingWeapon->ShowPickupWidget(true);
	if (!LastWeapon) return;
	LastWeapon->ShowPickupWidget(false);
}

void AFPSCharacter::NextWeapon(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const int32 Index = Weapons.IsValidIndex(CurrentIndex + 1) ? CurrentIndex + 1 : 0;
		EquipWeapon(Index);
	}
}

void AFPSCharacter::LastWeapon(const FInputActionValue& Value)
{
	if (Controller != nullptr)
	{
		const int32 Index = Weapons.IsValidIndex(CurrentIndex - 1) ? CurrentIndex - 1 : Weapons.Num() - 1;
		EquipWeapon(Index);
	}
}

void AFPSCharacter::AimOffset(float DeltaTime)
{
	if (CombatComponent && CombatComponent->EquippedWeapon == nullptr) return; // May want to account for Unarmed State
	FVector Velocity = GetVelocity();
	Velocity.Z = 0.f;
	float Speed = Velocity.Size();
	bool bIsInAir = GetCharacterMovement()->IsFalling();

	if (IsInFirstPersonPerspective())
	{
	}
	else
	{
		if (Speed == 0.f && !bIsInAir) // standing still, not jumping
		{
			FRotator CurrentAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			FRotator DeltaAimRotation = UKismetMathLibrary::NormalizedDeltaRotator(
				CurrentAimRotation, StartingAimRotation);

			AO_Yaw = DeltaAimRotation.Yaw;
			if (TurningInPlace == ETurningInPlace::ETIP_NotTurning)
			{
				InterpAO_Yaw = AO_Yaw;
			}
			//bUseControllerRotationYaw = true; // Depends on 3PP or 1PP
			TurnInPlace(DeltaTime);
		}
		if (Speed > 0.f || bIsInAir) // running or jumping
		{
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
			AO_Yaw = 0.f;
			//bUseControllerRotationYaw = true; // Depends on 3PP or 1PP
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
		}
	}

	AO_Pitch = GetBaseAimRotation().Pitch;
	if (AO_Pitch > 90.f && !IsLocallyControlled())
	{
		// Map Pitch from [270, 360) to [-90, 0)
		FVector2D InRange(270.f, 360.f);
		FVector2D OutRange(-90.f, 0.f);
		AO_Pitch = FMath::GetMappedRangeValueClamped(InRange, OutRange, AO_Pitch);
	}
}

void AFPSCharacter::TurnInPlace(float DeltaTime)
{
	if (AO_Yaw > 90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Right;
	}
	else if (AO_Yaw < -90.f)
	{
		TurningInPlace = ETurningInPlace::ETIP_Left;
	}
	if (TurningInPlace != ETurningInPlace::ETIP_NotTurning)
	{
		InterpAO_Yaw = FMath::FInterpTo(InterpAO_Yaw, 0.f, DeltaTime, 6.f);
		AO_Yaw = InterpAO_Yaw;
		if (FMath::Abs(AO_Yaw) < 15.f)
		{
			TurningInPlace = ETurningInPlace::ETIP_NotTurning;
			StartingAimRotation = FRotator(0.f, GetBaseAimRotation().Yaw, 0.f);
		}
	}
}


AFPSWeapon* AFPSCharacter::GetEquippedWeapon()
{
	if (CombatComponent == nullptr) return nullptr;
	return CombatComponent->EquippedWeapon;
}

/* Online Sessions
void AFPSCharacter::CreateGameSession()
{
	if (!OnlineSessionInterface.IsValid()) return;
	auto ExistingSession = OnlineSessionInterface->GetNamedSession(NAME_GameSession);
	if (ExistingSession != nullptr)
	{
		OnlineSessionInterface->DestroySession(NAME_GameSession);
	}

	OnlineSessionInterface->AddOnCreateSessionCompleteDelegate_Handle(CreateSessionCompleteDelegate);

	TSharedPtr<FOnlineSessionSettings> SessionSettings = MakeShareable(new FOnlineSessionSettings());
	SessionSettings->bIsLANMatch = false;
	SessionSettings->NumPublicConnections = 4;
	SessionSettings->bAllowJoinInProgress = true;
	SessionSettings->bAllowJoinViaPresence = true;
	SessionSettings->bShouldAdvertise = true;
	SessionSettings->bUsesPresence = true;
	SessionSettings->bUseLobbiesIfAvailable = true;
	SessionSettings->Set(FName("MatchType"), FString("FreeForAll"), EOnlineDataAdvertisementType::ViaOnlineServiceAndPing);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->CreateSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, *SessionSettings);
}

void AFPSCharacter::JoinGameSession()
{
	// Find game session
	if (!OnlineSessionInterface.IsValid()) return;

	OnlineSessionInterface->AddOnFindSessionsCompleteDelegate_Handle(FindSessionsCompleteDelegate);

	SessionSearch = MakeShareable(new FOnlineSessionSearch);
	SessionSearch->MaxSearchResults = 10000;
	SessionSearch->bIsLanQuery = false;
	SessionSearch->QuerySettings.Set(SEARCH_PRESENCE, true, EOnlineComparisonOp::Equals);

	const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
	OnlineSessionInterface->FindSessions(*LocalPlayer->GetPreferredUniqueNetId(), SessionSearch.ToSharedRef());
}


void AFPSCharacter::OnCreateSessionComplete(FName SessionName, bool bWasSuccessful)
{

	if (bWasSuccessful)
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Green,
				FString::Printf(TEXT("Created session: %s"), *SessionName.ToString())
			);
			UWorld* World = GetWorld();
			if (World)
			{
				World->ServerTravel(FString("/Game/MechaMania/Maps/Lobby?listen"));
			}
		}
	}
	else
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Red,
				FString::Printf(TEXT("Failed to create session"))
			);
		}
	}

}

void AFPSCharacter::OnFindSessionsComplete(bool bWasSuccessful)
{
	if (!OnlineSessionInterface.IsValid()) return;
	for (auto Result : SessionSearch->SearchResults)
	{
		FString Id = Result.GetSessionIdStr();
		FString User = Result.Session.OwningUserName;
		FString MatchType;
		Result.Session.SessionSettings.Get(FName("MatchType"), MatchType);
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Cyan,
				FString::Printf(TEXT("Id: %s, User: %s"), *Id, *User)
			);
		}
		if (MatchType == FString("FreeForAll"))
		{
			if (GEngine)
			{
				GEngine->AddOnScreenDebugMessage(
					-1,
					15.f,
					FColor::Cyan,
					FString::Printf(TEXT("Joining Match Type %s"), *MatchType)
				);
			}

			OnlineSessionInterface->AddOnJoinSessionCompleteDelegate_Handle(JoinSessionCompleteDelegate);
			const ULocalPlayer* LocalPlayer = GetWorld()->GetFirstLocalPlayerFromController();
			OnlineSessionInterface->JoinSession(*LocalPlayer->GetPreferredUniqueNetId(), NAME_GameSession, Result);
		}
	}
}

void AFPSCharacter::OnJoinSessionComplete(FName SessionName, EOnJoinSessionCompleteResult::Type Result)
{
	if (!OnlineSessionInterface.IsValid()) return;
	FString Address;
	if (OnlineSessionInterface->GetResolvedConnectString(NAME_GameSession, Address))
	{
		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Yellow,
				FString::Printf(TEXT("Connect string: %s"), *Address)
			);
		}

		APlayerController* PlayerController = GetGameInstance()->GetFirstLocalPlayerController();
		if (PlayerController)
		{
			PlayerController->ClientTravel(Address, ETravelType::TRAVEL_Absolute);
		}
	}
}
*/
