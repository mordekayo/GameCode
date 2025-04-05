// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacter.h"
#include "Actors/Interactive/Environment/Ladder.h"
#include "Actors/Interactive/Environment/Zipline.h"
#include "Components/SkeletalMeshComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "Components/LedgeDetectorComponent.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"
#include "Components/CharacterComponents/CharacterEquipmentComponent.h"
#include "Controllers/GCPlayerController.h"
#include "Curves/CurveVector.h"
#include "DrawDebugHelpers.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameCodeTypes.h"
#include "GameFramework/PhysicsVolume.h"
#include "Kismet/KismetSystemLibrary.h"
#include "Actors/Equipment/Weapons/RangeWeaponItem.h"
#include "Curves/CurveFloat.h"
#include "Engine/DamageEvents.h"
#include "Net/UnrealNetwork.h"

AGCBaseCharacter::AGCBaseCharacter(const FObjectInitializer& ObjectInitializer)
	:Super(ObjectInitializer.SetDefaultSubobjectClass<UGCBaseCharacterMovementComponent>(ACharacter::CharacterMovementComponentName))
{
	GCBaseCharacterMovementComponent = StaticCast<UGCBaseCharacterMovementComponent*>(GetCharacterMovement());

	GetMesh()->CastShadow = true;
	GetMesh()->bCastDynamicShadow = true;

	LedgeDetectorComponent = CreateDefaultSubobject<ULedgeDetectorComponent>(TEXT("LedgeDetector"));
	WallRunComponent = CreateDefaultSubobject<UWallRunComponent>(TEXT("WallRun"));
	CharacterAttributesComponent = CreateDefaultSubobject<UCharacterAttributesComponent>(TEXT("CharacterAttributes"));
	CharacterEquipmentComponent = CreateDefaultSubobject<UCharacterEquipmentComponent>(TEXT("CharacterEquipment"));
}

void AGCBaseCharacter::BeginPlay()
{
	Super::BeginPlay();
	AvaiableInteractiveActors.Reserve(10);
	GetBaseCharacterMovementComponent()->bNotifyApex = true;
	CharacterAttributesComponent->OnDeathEvent.AddUObject(this, &AGCBaseCharacter::OnDeath);
	CharacterAttributesComponent->OnOutOfStaminaEvent.AddUObject(this, &AGCBaseCharacter::OnStaminaOutOrMax);
}

void AGCBaseCharacter::Tick(float DeltaTime)
{
	if (bIsMantleRequested && !GetBaseCharacterMovementComponent()->IsMantling())
	Mantle();
	Super::Tick(DeltaTime);
	TryChangeSprintState(DeltaTime);
	
	if (bIKEnabled)
		UpdateIKSettings(DeltaTime);
}

void AGCBaseCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);
	GCPlayerController = Cast<AGCPlayerController>(NewController);
}

void AGCBaseCharacter::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	DOREPLIFETIME(AGCBaseCharacter, bIsMantling);
}

UWallRunComponent* AGCBaseCharacter::GetWallRunComponent()
{
	return WallRunComponent;
}

const UCharacterEquipmentComponent* AGCBaseCharacter::GetCharacterEquipmentComponent() const
{
	return CharacterEquipmentComponent;
}

const UCharacterAttributesComponent* AGCBaseCharacter::GetCharacterAttributesComponent() const
{
	return CharacterAttributesComponent;
}

void AGCBaseCharacter::ChangeCrouchState()
{
	if (GetCharacterMovement()->IsCrouching())
	{
		UnCrouch();
	}
	else if (CanCrouch())
	{
		Crouch();
	}
}

bool AGCBaseCharacter::CanCrouch()
{
	return GetCharacterMovement()->IsMovingOnGround();
}

bool AGCBaseCharacter::CanMantle() const
{
	return !GetBaseCharacterMovementComponent()->IsOnLadder() &&
		!GetBaseCharacterMovementComponent()->IsMantling() && !GetBaseCharacterMovementComponent()->IsOnZipline();
}

void AGCBaseCharacter::StartSprint()
{
	bIsSprintRequested = true;
	if (bIsCrouched)
	{
		UnCrouch();
	}
}

void AGCBaseCharacter::StopSprint()
{
	bIsSprintRequested = false;
}

void AGCBaseCharacter::StartSlide()
{
	if (GCBaseCharacterMovementComponent->IsSprinting())
	{
		GCBaseCharacterMovementComponent->StartSlide();
	}
}

void AGCBaseCharacter::StopSlide()
{
	if (GCBaseCharacterMovementComponent->IsSliding())
	{
		GCBaseCharacterMovementComponent->StopSlide();
	}
}

UGCBaseCharacterMovementComponent* AGCBaseCharacter::GetBaseCharacterMovementComponent() const
{
	return GCBaseCharacterMovementComponent;
}

void AGCBaseCharacter::OnSprintStart_Implementation()
{
	UE_LOG(LogTemp, Log, TEXT("OnSprintStart_Implementation()"))
}

void AGCBaseCharacter::OnSprintEnd_Implementation()
{
	UE_LOG(LogTemp, Warning, TEXT("OnSprintEnd_Implementation()"))
}

bool AGCBaseCharacter::CanSprint()
{
	return GetCharacterMovement()->IsMovingOnGround() && !GCBaseCharacterMovementComponent->IsOutOfStamina() &&
		!GCBaseCharacterMovementComponent->IsCrouching();
}

const FMantlingSettings& AGCBaseCharacter::GetMantlingSettings(float LedgeHeight) const
{
	return LedgeHeight > LowMantleMaxHeight ? HighMantleSettings : LowMantleSettings;
}

void AGCBaseCharacter::EnableRagdoll()
{
	GetMesh()->SetCollisionProfileName(CollisionProfileRagdoll);
	GetMesh()->SetSimulatePhysics(true);
}

void AGCBaseCharacter::TryChangeSprintState(float DeltaTime)
{
	if (bIsSprintRequested && !GCBaseCharacterMovementComponent->IsSprinting() && CanSprint() && !FMath::IsNearlyZero(GCBaseCharacterMovementComponent->GetCurrentMoveForwardInput()))
	{
		GCBaseCharacterMovementComponent->StartSprint();
		OnSprintStart();
	}

	if (GCBaseCharacterMovementComponent->IsSprinting() && (FMath::IsNearlyZero(GCBaseCharacterMovementComponent->GetCurrentMoveForwardInput()) || !bIsSprintRequested || !CanSprint()))
	{
		GCBaseCharacterMovementComponent->StopSprint();
		OnSprintEnd();
	}
}

float AGCBaseCharacter::CalculateIKOffsetForASocket(const FName& SocketName) const
{
	float ResultOffset = 0.f;

	const FVector SocketLocation = GetMesh()->GetSocketLocation(SocketName);
	float CapsuleHalfHeight = GetCapsuleComponent()->GetScaledCapsuleHalfHeight();

	FVector TraceStart(SocketLocation.X, SocketLocation.Y, GetActorLocation().Z);
	FVector TraceEnd = TraceStart - (CapsuleHalfHeight + IKTraceDistance) * FVector::UpVector;

	FHitResult HitResult;
	ETraceTypeQuery TraceType = UEngineTypes::ConvertToTraceType(ECC_Visibility);
	const FVector FootSizeBox = FVector(1.f, 15.f, 7.f);
	if (UKismetSystemLibrary::BoxTraceSingle(GetWorld(), TraceStart, TraceEnd, FootSizeBox, GetMesh()->GetSocketRotation(SocketName),
		TraceType, true, TArray<AActor*>(), EDrawDebugTrace::None, HitResult, true))
	{
		ResultOffset = TraceStart.Z - CapsuleHalfHeight - HitResult.Location.Z;
	}

	return ResultOffset;
}

void AGCBaseCharacter::UpdateIKSettings(float DeltaSeconds)
{
	IKLeftFootOffset = FMath::FInterpTo(IKLeftFootOffset, CalculateIKOffsetForASocket(LeftFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKRightFootOffset = FMath::FInterpTo(IKRightFootOffset, CalculateIKOffsetForASocket(RightFootSocketName), DeltaSeconds, IKInterpSpeed);
	IKPelvisOffset = FMath::FInterpTo(IKPelvisOffset, CalculateIKPelvisOffset(), DeltaSeconds, IKInterpSpeed);
}

float AGCBaseCharacter::CalculateIKPelvisOffset() const
{
	return  IKRightFootOffset > IKLeftFootOffset ? -IKRightFootOffset : -IKLeftFootOffset;
}

void AGCBaseCharacter::SwimForward(float Value)
{

}

void AGCBaseCharacter::SwimRight(float Value)
{

}

void AGCBaseCharacter::SwimUp(float Value)
{

}

void AGCBaseCharacter::Mantle(bool bForce /*= false*/)
{
	if (!CanMantle() && !bForce)
	{
		return;
	}

	FLedgeDescription LedgeDescription;
	if (LedgeDetectorComponent->DetectLedge(LedgeDescription))
	{
		bIsMantling = true;
		
		FMantlingMovementParameters MantlingParameters;

		MantlingParameters.InitialCharacterLocation = GetActorLocation();
		MantlingParameters.InitialCharacterRotation = GetActorRotation();
		MantlingParameters.TargetCharacterLocation = LedgeDescription.MantledCharacterLocation;
		MantlingParameters.TargetCharacterRotation = LedgeDescription.Rotation;
		MantlingParameters.LedgeActor = LedgeDescription.LedgeActor;
		MantlingParameters.LedgeActorInitialLocation = LedgeDescription.LedgeActorInitialLocation;

		float MantlingHeight = (LedgeDescription.Location - LedgeDescription.CurrentCharacterBottom).Z;
		const FMantlingSettings& MantlingSettings =	GCBaseCharacterMovementComponent->IsSwimming() ? HighMantleSettings : GetMantlingSettings(MantlingHeight);

		float MinRange;
		float MaxRange;
		MantlingSettings.MantlingCurve->GetTimeRange(MinRange, MaxRange);

		MantlingParameters.Duration = MaxRange - MinRange;

		MantlingParameters.MantlingCurve = MantlingSettings.MantlingCurve;
		FVector2D SourceRange(MantlingSettings.MinHeight, MantlingSettings.MaxHeight);
		FVector2D TargetRange(MantlingSettings.MinHeightStartTime, MantlingSettings.MaxHeightStartTime);

		MantlingParameters.StartTime = FMath::GetMappedRangeValueClamped(SourceRange, TargetRange, MantlingHeight);

		MantlingParameters.InitialAnimationCharacterLocation = MantlingParameters.TargetCharacterLocation - MantlingSettings.AnimationCorrectionZ * FVector::UpVector
			+ MantlingSettings.AnimationCorrectionXY * LedgeDescription.LedgeNormal;

		if (IsLocallyControlled() || GetLocalRole() == ROLE_Authority)
		{
			GCBaseCharacterMovementComponent->StartMantle(MantlingParameters);
		}

		UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
		AnimInstance->Montage_Play(MantlingSettings.MantlingMontage, 1.0f, EMontagePlayReturnType::Duration, MantlingParameters.StartTime);
		OnMantle(MantlingSettings, MantlingParameters.StartTime);
	}
}

void AGCBaseCharacter::StartFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StartFire();
	}
}

void AGCBaseCharacter::StopFire()
{
	ARangeWeaponItem* CurrentRangeWeapon = CharacterEquipmentComponent->GetCurrentRangeWeapon();
	if (IsValid(CurrentRangeWeapon))
	{
		CurrentRangeWeapon->StopFire();
	}
}

void AGCBaseCharacter::OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime)
{

}

void AGCBaseCharacter::StartMantleRequesting()
{
	bIsMantleRequested = true;
	Mantle();
}

void AGCBaseCharacter::StopMantleRequesting()
{
	bIsMantleRequested = false;
}

void AGCBaseCharacter::Falling()
{
	GetBaseCharacterMovementComponent()->bNotifyApex = true;
}

void AGCBaseCharacter::Landed(const FHitResult& Hit)
{
	Super::Landed(Hit);
	float FallHeight = (CurrentFallApex - GetActorLocation()).Z * 0.01f;
	if (IsValid(FallDamageCurve))
	{
		float DamageAmount = FallDamageCurve->GetFloatValue(FallHeight);
		AActor* DamageCauser = Hit.GetActor();
		TakeDamage(DamageAmount, FDamageEvent(), GetController(), DamageCauser);
	}
	if (FallHeight > HardLandingMinHeight)
	{
		HardLandingStart();
	}
}

void AGCBaseCharacter::NotifyJumpApex()
{
	Super::NotifyJumpApex();
	CurrentFallApex = GetActorLocation();
	ApexCharacterZPozition = GetActorLocation().Z;
}

bool AGCBaseCharacter::IsSwimmingUnderWater() const
{
	
	if (GetCharacterMovement()->IsSwimming())
	{
		FVector HeadPosition = GetMesh()->GetSocketLocation(FName("head")); // FName(�head�) should be saved as a string constant 
		APhysicsVolume* Volume = GetCharacterMovement()->GetPhysicsVolume();
		float VolumeTopPlane = Volume->GetActorLocation().Z + Volume->GetBounds().BoxExtent.Z * Volume->GetActorScale3D().Z;
		if (HeadPosition.Z < VolumeTopPlane)
		{
			return true;
		}
	}
	return false;
}

void AGCBaseCharacter::OnRep_IsMantling(bool bWasMantling)
{
	if (GetLocalRole() == ROLE_SimulatedProxy)
	{
		if (!bWasMantling && bIsMantling)
		{
			Mantle(true);
		}
	}
}

bool AGCBaseCharacter::CanJumpInternal_Implementation() const
{
	return Super::CanJumpInternal_Implementation() &&
		!GCBaseCharacterMovementComponent->IsMantling() &&
		!GCBaseCharacterMovementComponent->IsCrouching() &&
		!GCBaseCharacterMovementComponent->IsOutOfStamina();
}

void AGCBaseCharacter::RegisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvaiableInteractiveActors.Add(InteractiveActor);
}

void AGCBaseCharacter::UnregisterInteractiveActor(AInteractiveActor* InteractiveActor)
{
	AvaiableInteractiveActors.Remove(InteractiveActor);
}

void AGCBaseCharacter::ClimbLadderUp(float Value)
{
	if (!FMath::IsNearlyZero(Value) && GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		FVector LadderUpVector = GetBaseCharacterMovementComponent()->GetCurrentLadder()->GetActorUpVector();
		AddMovementInput(LadderUpVector, Value);
	}
}

void AGCBaseCharacter::InteractWithLadder()
{
	if (GetBaseCharacterMovementComponent()->IsOnLadder())
	{
		GetBaseCharacterMovementComponent()->DetachFromLadder(EDetachFromLadderMethod::JumpOff);
	}
	else
	{
		const ALadder* AvailableLadder = GetAvailableLadder();
		if (IsValid(AvailableLadder))
		{
			if (AvailableLadder->GetIsOnTop())
			{
				PlayAnimMontage(AvailableLadder->GetAttachFromTopAnimMontage());
			}
			GetBaseCharacterMovementComponent()->AttachToLadder(AvailableLadder);
		}
	}
}

void AGCBaseCharacter::InteractWithZipline()
{
	if (GetBaseCharacterMovementComponent()->IsOnZipline())
	{
		GetBaseCharacterMovementComponent()->DetachFromZipline();
	}
	else
	{
		const AZipline* AvailableZipline = GetAvailableZipline();
		if (IsValid(AvailableZipline))
		{
			GetBaseCharacterMovementComponent()->AttachToZipline(AvailableZipline);
		}
	}
}

void AGCBaseCharacter::HardLandingStart()
{
	UAnimInstance* AnimInstance = GetMesh()->GetAnimInstance();
	if (IsValid(AnimInstance) && HardLandingMontage)
	{
		if (GCPlayerController.IsValid())
		{
			GCPlayerController->SetIgnoreMoveInput(true);
		}
		float MontageDuration = AnimInstance->Montage_Play(HardLandingMontage, 1.0f, EMontagePlayReturnType::Duration, 0.0f);
		GetWorld()->GetTimerManager().SetTimer(HardLandingTimer, this, &AGCBaseCharacter::HardLandingEnd, MontageDuration, false);
	}
}

void AGCBaseCharacter::OnDeath()
{
	float Duration = PlayAnimMontage(OnDeathAnimMontage);
	if (Duration == 0.0f)
	{
		EnableRagdoll();
	}
}

void AGCBaseCharacter::OnStaminaOutOrMax(bool MaxOrOut)
{
	GetBaseCharacterMovementComponent()->SetIsOutOfStamina(!MaxOrOut);
}

void AGCBaseCharacter::HardLandingEnd()
{
	if (GCPlayerController.IsValid())
	{
		GCPlayerController->SetIgnoreMoveInput(false);
	}
}

const ALadder* AGCBaseCharacter::GetAvailableLadder() const
{
	const ALadder* Result = nullptr;
	for (const AInteractiveActor* InteractiveActor : AvaiableInteractiveActors)
	{
		if (InteractiveActor->IsA<ALadder>())
		{
			Result = StaticCast<const ALadder*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

const AZipline* AGCBaseCharacter::GetAvailableZipline() const
{
	const AZipline* Result = nullptr;
	for (const AInteractiveActor* InteractiveActor : AvaiableInteractiveActors)
	{
		if (InteractiveActor->IsA<AZipline>())
		{
			Result = StaticCast<const AZipline*>(InteractiveActor);
			break;
		}
	}
	return Result;
}

void  AGCBaseCharacter::JumpWhileWallRunning()
{
	if (GCBaseCharacterMovementComponent->IsWallRunning())
	{
		GCBaseCharacterMovementComponent->JumpWhileWallRun();
	}
}

