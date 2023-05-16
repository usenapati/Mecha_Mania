// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSCasing.h"
#include "Kismet/GameplayStatics.h"
#include "Sound/SoundCue.h"

// Sets default values
AFPSCasing::AFPSCasing()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = false;

	CasingMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CasingMesh"));
	SetRootComponent(CasingMesh);

	CasingMesh->SetCollisionResponseToChannel(ECC_Camera, ECR_Ignore);
	CasingMesh->SetSimulatePhysics(true);
	CasingMesh->SetEnableGravity(true);
	CasingMesh->SetNotifyRigidBodyCollision(true);
	ShellEjectionImpulse = 10.f;
}

// Called when the game starts or when spawned
void AFPSCasing::BeginPlay()
{
	Super::BeginPlay();

	CasingMesh->OnComponentHit.AddDynamic(this, &AFPSCasing::AFPSCasing::OnHit);
	CasingMesh->AddImpulse(GetActorForwardVector() * ShellEjectionImpulse);
	
}

void AFPSCasing::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	if (ShellSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ShellSound, GetActorLocation());
	}
	Destroy();
}


