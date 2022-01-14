// Fill out your copyright notice in the Description page of Project Settings.


#include "WeaponBarellComponent.h"
#include "GameCodeTypes.h"
#include "DrawDebugHelpers.h"
#include "Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "NiagaraFunctionLibrary.h"
#include "NiagaraComponent.h"

void UWeaponBarellComponent::Shot(FVector ShotStart, FVector ShotDirection, AController* Controller)
{
	FVector MuzzleLocation = GetComponentLocation();
	FVector ShotEnd = ShotStart + FiringRange * ShotDirection;

	UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), MuzzleFlashFX, MuzzleLocation, GetComponentRotation());

	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryRangeWeapon);

	FHitResult ShotResult;
	if (GetWorld()->LineTraceSingleByChannel(ShotResult, ShotStart, ShotEnd, ECC_Bullet))
	{
		ShotEnd = ShotResult.ImpactPoint;
		
		if (bIsDebugEnabled)
		{
			DrawDebugSphere(GetWorld(), ShotEnd, 10.0f, 24, FColor::Red, false, 1.0f);
		}
		AActor* HitActor = ShotResult.GetActor();
		if (IsValid(HitActor))
		{
			float DistanceDamageCoef = 1.0f;
			if (IsValid(FalloffDiagram))
			{
				DistanceDamageCoef = FalloffDiagram->GetFloatValue((ShotEnd - MuzzleLocation).Size());
			}
			HitActor->TakeDamage(DamageAmount * DistanceDamageCoef, FDamageEvent{}, Controller, GetOwner());
		}
	}

	UNiagaraComponent* TraceFXComponent = UNiagaraFunctionLibrary::SpawnSystemAtLocation(GetWorld(), BulletTraceFX, MuzzleLocation, GetComponentRotation());

	TraceFXComponent->SetVectorParameter(FXParamTraceEnd, ShotEnd);

	if (bIsDebugEnabled)
	{
		DrawDebugLine(GetWorld(), ShotStart, ShotEnd, FColor::Red, false, 1.0f, 0, 3.0f);
	}
}
