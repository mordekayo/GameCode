// Fill out your copyright notice in the Description page of Project Settings.


#include "LedgeDetectorComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "../GameCodeTypes.h"
#include "DrawDebugHelpers.h"
#include "../Utils/GCTraceUtils.h"
#include "../Characters/GCBaseCharacter.h"
#include "../GCGameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "../Subsystems/DebugSubsystem.h"
#include "../Actors/Interactive/Environment/Zipline.h"

// Called when the game starts
void ULedgeDetectorComponent::BeginPlay()
{
	Super::BeginPlay();
	checkf(GetOwner()->IsA<ACharacter>(), TEXT("ULedgeDetectorComponent::BeginPlay() only a character can use ULedgeDetectorComponent"));
	CachedCharacterOwner = StaticCast<ACharacter*>(GetOwner());
}

bool ULedgeDetectorComponent::DetectLedge(OUT FLedgeDescription& LedgeDescription)
{
	ACharacter* DefaultCharacter = CachedCharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	FCollisionQueryParams QueryParams;
	QueryParams.bTraceComplex = true;
	QueryParams.AddIgnoredActor(GetOwner());

	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategoryLedgeDetection);
	float DrawTime = 10.0f;

	float BottomZOffset = 2.0f;
	FVector CharacterBottom = CachedCharacterOwner->GetActorLocation() -
		(DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() - BottomZOffset) * FVector::UpVector;

	//1.Forward check

	float ForwardTraceCapsuleRadius = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float ForwardTraceCapsuleHalfHeight = (MaximumLedgeHeight - MinimumLedgeHeight) * 0.5f;

	FHitResult ForwardTraceHitResult;
	FVector ForwardTraceStartLocation = CharacterBottom + (MinimumLedgeHeight + ForwardTraceCapsuleHalfHeight) * FVector::UpVector;
	FVector ForwardTraceEndLocation = ForwardTraceStartLocation + CachedCharacterOwner->GetActorForwardVector() * ForwardCheckDistance;

	if (!GCTraceUtils::SweepCapsuleSingleByChannel(GetWorld(), ForwardTraceHitResult, ForwardTraceStartLocation,
		ForwardTraceEndLocation, ECC_Climbing, ForwardTraceCapsuleRadius, ForwardTraceCapsuleHalfHeight, FQuat::Identity,
		QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	//2.Downward check

	FHitResult DownwardTraceHitResult;
	float DownwardTraceSphereRadius = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float DownwardTraceDepthOffset = 10.0f;

	FVector DownwardTraceStartLocation = ForwardTraceHitResult.ImpactPoint - ForwardTraceHitResult.ImpactNormal * DownwardTraceDepthOffset;
	DownwardTraceStartLocation.Z = CharacterBottom.Z + MaximumLedgeHeight + DownwardTraceSphereRadius;
	FVector DownwardTraceEndLocation(DownwardTraceStartLocation.X, DownwardTraceStartLocation.Y, CharacterBottom.Z + MinimumLedgeHeight);

	if (!GCTraceUtils::SweepSphereSingleByChannel(GetWorld(), DownwardTraceHitResult, DownwardTraceStartLocation,
		DownwardTraceEndLocation, ECC_Climbing, DownwardTraceSphereRadius,
		QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	//3.Overlap check

	float OverlapCapsuleRadius = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleRadius();
	float OverlapCapsuleHalfHeight = DefaultCharacter->GetCapsuleComponent()->GetScaledCapsuleHalfHeight();
	float OverlapCapsuleFloorOffset = 1.0f;
	FVector OverlapLocation = DownwardTraceHitResult.ImpactPoint + OverlapCapsuleHalfHeight * FVector::UpVector + OverlapCapsuleFloorOffset;

	if (GCTraceUtils::OverlapCapsuleBlockingByProfile(GetWorld(), OverlapLocation, OverlapCapsuleRadius, OverlapCapsuleHalfHeight,
		FQuat::Identity, CollisionProfilePawn, QueryParams, bIsDebugEnabled, DrawTime))
	{
		return false;
	}

	LedgeDescription.Location = DownwardTraceHitResult.ImpactPoint;
	LedgeDescription.MantledCharacterLocation = OverlapLocation;
	LedgeDescription.LedgeActor = DownwardTraceHitResult.GetActor();
	if (LedgeDescription.LedgeActor.IsValid())
	{
		if (LedgeDescription.LedgeActor->IsA<AZipline>())
		{
			return false;
		}
		UActorComponent* MeshComponent = LedgeDescription.LedgeActor->GetComponentByClass(UStaticMeshComponent::StaticClass());

		UPrimitiveComponent* PrimitiveComponent = StaticCast<UPrimitiveComponent*>(MeshComponent);

   		LedgeDescription.LedgeActorInitialLocation = PrimitiveComponent->GetComponentLocation();

	}
	LedgeDescription.Rotation = (ForwardTraceHitResult.ImpactNormal * FVector(-1.0f, -1.0f, 0.0f)).ToOrientationRotator();
	LedgeDescription.LedgeNormal = ForwardTraceHitResult.ImpactNormal;
	LedgeDescription.CurrentCharacterBottom = CharacterBottom;

	return true;
}
