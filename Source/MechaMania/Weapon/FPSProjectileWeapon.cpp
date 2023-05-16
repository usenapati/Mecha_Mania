// Fill out your copyright notice in the Description page of Project Settings.


#include "FPSProjectileWeapon.h"

#include "Engine/SkeletalMeshSocket.h"

void AFPSProjectileWeapon::Fire(const FVector& HitTarget)
{
	Super::Fire(HitTarget);
	if (!HasAuthority()) return;
	
	APawn* InstigatorPawn = Cast<APawn>(GetOwner());
	const USkeletalMeshSocket* MuzzleFlashSocket = GetWeaponMesh()->GetSocketByName(MuzzleSocket);
	if (MuzzleFlashSocket)
	{
		FTransform SocketTransform = MuzzleFlashSocket->GetSocketTransform(GetWeaponMesh());
		FVector ToTarget = HitTarget - SocketTransform.GetLocation();
		FRotator TargetRotation = ToTarget.Rotation();
		if (ProjectileClass && InstigatorPawn)
		{
			FActorSpawnParameters SpawnParams;
			SpawnParams.Owner = GetOwner();
			SpawnParams.Instigator = InstigatorPawn;
			UWorld* World = GetWorld();
			if (World)
			{
				World->SpawnActor<AFPSProjectile>(
					ProjectileClass,
					SocketTransform.GetLocation(),
					TargetRotation,
					SpawnParams
					);
			}
		}
	}
}
