// Fill out your copyright notice in the Description page of Project Settings.


#include "WallRunComponent.h"
#include "../GameCodeTypes.h"
#include "../Utils/GCTraceUtils.h"
#include "Kismet/GameplayStatics.h"
#include "../Subsystems/DebugSubsystem.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "CollisionQueryParams.h"

void UWallRunComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(GetOwner()->IsA<ACharacter>(), TEXT("UWallRunComponent::BeginPlay() only a character can use UWallRunComponent"));
	CachedCharacterOwner = StaticCast<ACharacter*>(GetOwner());
}

bool UWallRunComponent::DetectWall(OUT FWallRunParameters& WallRunParameters)
{
	ACharacter* DefaultCharacter = CachedCharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	FHitResult TraceHitResult;

	FVector LeftTraceStartLocation = CachedCharacterOwner->GetActorLocation() - CachedCharacterOwner->GetActorRightVector() * WallSearchStartOffset;
	FVector RightTraceStartLocation = CachedCharacterOwner->GetActorLocation() + CachedCharacterOwner->GetActorRightVector() * WallSearchStartOffset;

	FVector LeftTraceEndLocation = LeftTraceStartLocation - CachedCharacterOwner->GetActorRightVector() * WallSearchTraceDistance;
	FVector RightTraceEndLocation = RightTraceStartLocation + CachedCharacterOwner->GetActorRightVector() * WallSearchTraceDistance;

	float TraceCapsuleRadius = (DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius())/5;
	float TraceCapsuleHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(GetOwner());

	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryWallRun);
	float DrawTime = 1.0f;

	if (GCTraceUtils::SweepCapsuleSingleByChannel(GetWorld(), TraceHitResult, LeftTraceStartLocation,
		LeftTraceEndLocation, ECC_WallRunnable, TraceCapsuleRadius, TraceCapsuleHalfHeight, FQuat::Identity,
		QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime, FColor::Magenta))
	{
		WallRunParameters.Side = EWallRunSide::Left;
		WallRunParameters.Direction = FVector::CrossProduct(TraceHitResult.ImpactNormal, FVector::UpVector).GetSafeNormal();
		WallRunParameters.WallNormal = TraceHitResult.ImpactNormal;
		TraceHitResult.ImpactPoint;
	}
	else if (GCTraceUtils::SweepCapsuleSingleByChannel(GetWorld(), TraceHitResult, RightTraceStartLocation,
			RightTraceEndLocation, ECC_WallRunnable, TraceCapsuleRadius, TraceCapsuleHalfHeight, FQuat::Identity,
			QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		WallRunParameters.Side = EWallRunSide::Right;
		WallRunParameters.Direction = FVector::CrossProduct(FVector::UpVector, TraceHitResult.ImpactNormal).GetSafeNormal();
		WallRunParameters.WallNormal = TraceHitResult.ImpactNormal;
	}
	else
	{
		return false;
	}

	FVector CharacterBottom = CachedCharacterOwner->GetActorLocation() -
		(DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * FVector::UpVector;

	FVector DownwardTraceStartLocation = CharacterBottom;
	FVector DownwardTraceEndLocation = CharacterBottom + FVector::DownVector * FloorSearchTraceDistance;

	if (GCTraceUtils::SweepBoxSingleByChannel(GetWorld(), TraceHitResult, DownwardTraceStartLocation,
		DownwardTraceEndLocation, ECC_Pawn, TraceCapsuleRadius , TraceCapsuleRadius , 10.0f, FQuat::Identity,
		QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, 5.0f, FColor::Cyan))
	{
		return false;
	}

	return true;
}


