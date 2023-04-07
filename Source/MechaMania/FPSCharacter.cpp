// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCharacter.h"
#include "Camera/CameraComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/InputComponent.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Controller.h"
//#include "GameFramework/SpringArmComponent.h"
#include "EnhancedInput/Public/EnhancedInputSubsystems.h"
#include "EnhancedInput/Public/InputMappingContext.h"
#include "EnhancedInput/Public/EnhancedInputComponent.h"
#include "EnhancedInput/Public/InputActionValue.h"
#include "InputConfigData.h"
#include "FPSWeapon.h"
#include "Components/SkeletalMeshComponent.h"
#include "Net/UnrealNetwork.h"
#include "OnlineSubsystem.h"
#include "OnlineSessionSettings.h"
#include "Interfaces/OnlineSessionInterface.h"

AFPSCharacter::AFPSCharacter() :
	CreateSessionCompleteDelegate(FOnCreateSessionCompleteDelegate::CreateUObject(this, &ThisClass::OnCreateSessionComplete)),
	FindSessionsCompleteDelegate(FOnFindSessionsCompleteDelegate::CreateUObject(this, &ThisClass::OnFindSessionsComplete)),
	JoinSessionCompleteDelegate(FOnJoinSessionCompleteDelegate::CreateUObject(this, &AFPSCharacter::OnJoinSessionComplete))
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
	PrimaryActorTick.bCanEverTick = false;

	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->bUsePawnControlRotation = true;
	Camera->SetupAttachment(GetMesh(), FName("head"));

	// Online Subsystem
	IOnlineSubsystem* OnlineSubsystem = IOnlineSubsystem::Get();
	if (OnlineSubsystem)
	{
		OnlineSessionInterface = OnlineSubsystem->GetSessionInterface();

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(
				-1,
				15.f,
				FColor::Blue,
				FString::Printf(TEXT("Found subsystem %s"), *OnlineSubsystem->GetSubsystemName().ToString())
			);
		}
	}


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
}

void AFPSCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
	// Get the player controller
	if (APlayerController* PC = Cast<APlayerController>(GetController())) {
		UE_LOG(LogTemp, Warning, TEXT("Successfully cast APlayerController"));
		// Get the local player subsystem
		UEnhancedInputLocalPlayerSubsystem* Subsystem = ULocalPlayer::GetSubsystem<UEnhancedInputLocalPlayerSubsystem>(PC->GetLocalPlayer());
		// Clear out existing mapping, and add our mapping
		Subsystem->ClearAllMappings();
		Subsystem->AddMappingContext(InputMapping, 0);
	}

	// Get the EnhancedInputComponent
	if (UEnhancedInputComponent* PEI = Cast<UEnhancedInputComponent>(PlayerInputComponent)) {
		UE_LOG(LogTemp, Warning, TEXT("Successfully cast UEnhancedInputComponent"));
		// Bind the actions
		PEI->BindAction(InputActions->InputMove, ETriggerEvent::Triggered, this, &AFPSCharacter::Move);
		PEI->BindAction(InputActions->InputLook, ETriggerEvent::Triggered, this, &AFPSCharacter::Look);
		PEI->BindAction(InputActions->InputNextWeapon, ETriggerEvent::Triggered, this, &AFPSCharacter::NextWeapon); 
		PEI->BindAction(InputActions->InputLastWeapon, ETriggerEvent::Triggered, this, &AFPSCharacter::LastWeapon); 
	}

}

void AFPSCharacter::GetLifetimeReplicatedProps(TArray<FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);

	DOREPLIFETIME_CONDITION(AFPSCharacter, Weapons, COND_None);
	DOREPLIFETIME_CONDITION(AFPSCharacter, CurrentWeapon, COND_None);
}

void AFPSCharacter::OnRep_CurrentWeapon(const AFPSWeapon* OldWeapon)
{
	if (CurrentWeapon)
	{
		if (!CurrentWeapon->CurrentOwner)
		{
			const FTransform PlacementTransform = CurrentWeapon->PlacementTransform * GetMesh()->GetSocketTransform(FName("hand_r"));
			CurrentWeapon->SetActorTransform(PlacementTransform, false, nullptr, ETeleportType::TeleportPhysics);
			CurrentWeapon->AttachToComponent(GetMesh(), FAttachmentTransformRules::KeepWorldTransform, FName("hand_r"));

			CurrentWeapon->Mesh->SetVisibility(true);
			CurrentWeapon->CurrentOwner = this;
		}

		CurrentWeapon->Mesh->SetVisibility(true);
	}

	if (OldWeapon)
	{
		OldWeapon->Mesh->SetVisibility(false);
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
}

void AFPSCharacter::EquipWeapon(const int32 Index)
{
	if(!Weapons.IsValidIndex(Index) || CurrentWeapon == Weapons[Index]) return;
	if (IsLocallyControlled())
	{
		CurrentIndex = Index;

		const AFPSWeapon* OldWeapon = CurrentWeapon;
		CurrentWeapon = Weapons[Index];
		OnRep_CurrentWeapon(OldWeapon);
		
	}
	else if(!HasAuthority())
	{
		Server_SetCurrentWeapon(Weapons[Index]);
	}
}

void AFPSCharacter::Server_SetCurrentWeapon_Implementation(AFPSWeapon* NewWeapon)
{
	const AFPSWeapon* OldWeapon = CurrentWeapon;
	CurrentWeapon = NewWeapon;
	OnRep_CurrentWeapon(OldWeapon);
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
				World->ServerTravel(FString("Game/MechaMania/Maps/Lobby?listen"));
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
