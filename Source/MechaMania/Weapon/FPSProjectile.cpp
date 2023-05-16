// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSProjectile.h"

#include "Components/BoxComponent.h"
#include "GameFramework/ProjectileMovementComponent.h"
#include "Kismet/GameplayStatics.h"
#include "Particles/ParticleSystemComponent.h"
#include "Particles/ParticleSystem.h"
#include "Sound/SoundCue.h"

AFPSProjectile::AFPSProjectile()
{
	PrimaryActorTick.bCanEverTick = true;
	bReplicates = true;

	CollisionBox = CreateDefaultSubobject<UBoxComponent>(TEXT("CollisionBox"));
	SetRootComponent(CollisionBox);
	CollisionBox->SetCollisionObjectType(ECC_WorldDynamic);
	CollisionBox->SetCollisionEnabled(ECollisionEnabled::QueryAndPhysics);
	CollisionBox->SetCollisionResponseToAllChannels(ECR_Ignore);
	CollisionBox->SetCollisionResponseToChannel(ECC_Visibility, ECR_Block);
	CollisionBox->SetCollisionResponseToChannel(ECC_WorldStatic, ECR_Block);

	ProjectileMovementComponent = CreateDefaultSubobject<UProjectileMovementComponent>(TEXT("ProjectileMovementComponent"));
	ProjectileMovementComponent->bRotationFollowsVelocity = true;

	
}

void AFPSProjectile::BeginPlay()
{
	Super::BeginPlay();

	if (Tracer)
	{
		TracerComponent = UGameplayStatics::SpawnEmitterAttached(
			Tracer,
			CollisionBox,
			FName(),
			GetActorLocation(),
			GetActorRotation(),
			EAttachLocation::KeepWorldPosition);
	}

	if (HasAuthority())
	{
		CollisionBox->OnComponentHit.AddDynamic(this, &AFPSProjectile::OnHit);
	}
	
}

void AFPSProjectile::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

}

void AFPSProjectile::Destroyed()
{
	Super::Destroyed();

	if (ImpactParticles)
	{
		UGameplayStatics::SpawnEmitterAtLocation(GetWorld(), ImpactParticles, GetActorTransform());
	}
	if (ImpactSound)
	{
		UGameplayStatics::PlaySoundAtLocation(this, ImpactSound, GetActorLocation());
	}
}

void AFPSProjectile::OnHit(UPrimitiveComponent* HitComp, AActor* OtherActor, UPrimitiveComponent* OtherComp,
	FVector NormalImpulse, const FHitResult& Hit)
{
	// Damage
	Destroy();
}



