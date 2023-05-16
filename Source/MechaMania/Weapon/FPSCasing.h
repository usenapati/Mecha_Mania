// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "FPSCasing.generated.h"

UCLASS()
class MECHAMANIA_API AFPSCasing : public AActor
{
	GENERATED_BODY()
	
public:	
	AFPSCasing();

protected:
	virtual void BeginPlay() override;

	UFUNCTION()
	virtual void OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, FVector NormalImpulse, const FHitResult& Hit);
	
private:
	UPROPERTY(VisibleAnywhere, Category = "MechaMania|Casing Properties")
	UStaticMeshComponent* CasingMesh;

	UPROPERTY(EditAnywhere, Category = "MechaMania|Casing Properties")
	float ShellEjectionImpulse;

	UPROPERTY(EditAnywhere, Category = "MechaMania|Casing Properties")
	class USoundCue* ShellSound;
};
