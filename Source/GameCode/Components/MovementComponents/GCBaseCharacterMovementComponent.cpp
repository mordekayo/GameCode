// Fill out your copyright notice in the Description page of Project Settings.


#include "GCBaseCharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "Components/CapsuleComponent.h"
#include "Curves/CurveVector.h"
#include "../../Actors/Interactive/Environment/Ladder.h"
#include "../../Actors/Interactive/Environment/Zipline.h"
#include "GameFramework/Actor.h"
#include "../../Characters/GCBaseCharacter.h"
#include "DrawDebugHelpers.h"
#include "../WallRunComponent.h"
#include "../../Utils/GCTraceUtils.h"
#include "../../Subsystems/DebugSubsystem.h"
#include "Kismet/GameplayStatics.h"
#include "../../GameCodeTypes.h"
#include "GameFramework/PhysicsVolume.h"

float UGCBaseCharacterMovementComponent::GetMaxSpeed() const
{
	float Result = Super::GetMaxSpeed();
	if (bIsSprinting)
	{
		Result = SprintSpeed;
	}
	else if (bIsOutOfStamina)
	{
		Result = OutOfStaminaMaxSpeed;
	}
	else if (IsOnLadder())
	{
		Result = ClimbingOnLadderMaxSpeed;
	}
	else if (IsWallRunning())
	{
		Result = MaxWallRunSpeed;
	}
	return Result;
}

void UGCBaseCharacterMovementComponent::SetIsOutOfStamina(bool bIsOutOfStamina_In)
{
	bIsOutOfStamina = bIsOutOfStamina_In;
}

void UGCBaseCharacterMovementComponent::StartSprint()
{
	bIsSprinting = true;
	bForceMaxAccel = 1;
}

void UGCBaseCharacterMovementComponent::StopSprint()
{
	bIsSprinting = false;
	bForceMaxAccel = 0;
}

void UGCBaseCharacterMovementComponent::StartSlide()
{
	bIsSliding = true;
	GetWorld()->GetTimerManager().SetTimer(SlidingTimer, this,
		&UGCBaseCharacterMovementComponent::StopSlide, SlideMaxTime, false);
	SetMovementMode(EMovementMode::MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Sliding);
}


void UGCBaseCharacterMovementComponent::StopSlide()
{
	SetMovementMode(EMovementMode::MOVE_Walking);
	GetWorld()->GetTimerManager().ClearTimer(SlidingTimer);
	GetWorld()->GetTimerManager().SetTimer(SlidingEndTimer, this,
		&UGCBaseCharacterMovementComponent::OnSlidingStandUpTimerElapsed, SlideStandUpTime, false);
}

void UGCBaseCharacterMovementComponent::OnSlidingStandUpTimerElapsed()
{
	bIsSliding = false;
}

void UGCBaseCharacterMovementComponent::StartMantle(const FMantlingMovementParameters& MantlingParameters)
{
	CurrentMantlingParameters = MantlingParameters;
	SetMovementMode(EMovementMode::MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_Mantling);
}

void UGCBaseCharacterMovementComponent::EndMantle()
{
	SetMovementMode(EMovementMode::MOVE_Walking);
}

bool UGCBaseCharacterMovementComponent::IsMantling() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_Mantling;
}

void UGCBaseCharacterMovementComponent::AttachToLadder(const class ALadder* Ladder)
{
	CurrentLadder = Ladder;

	FRotator TargetOrientationRotation = CurrentLadder->GetActorForwardVector().ToOrientationRotator();
	TargetOrientationRotation.Yaw += 180.0f;

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	FVector LadderForwardVector = CurrentLadder->GetActorUpVector();
	float Projection = GetProjectionToActor(CurrentLadder, GetActorLocation());

	FVector NewCharacterLocation = CurrentLadder->GetActorLocation() + Projection * LadderUpVector +
		LadderToCharacterOffset * CurrentLadder->GetActorForwardVector() + LadderUpVector * LadderStartTranslation;
	if (CurrentLadder->GetIsOnTop())
	{
		NewCharacterLocation = CurrentLadder->GetAttachFromTopAnimMontageStartingLocation();
	}
	GetOwner()->SetActorLocation(NewCharacterLocation);
	GetOwner()->SetActorRotation(TargetOrientationRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_OnLadder);
}

void UGCBaseCharacterMovementComponent::AttachToZipline(const class AZipline* Zipline)
{
	CurrentZipline = Zipline;
	
	FRotator TargetOrientationRotation = CurrentZipline->GetSlidingDirection().ToOrientationRotator();
	TargetOrientationRotation.Pitch = 0;
	TargetOrientationRotation.Roll = 0;

	FVector NewCharacterLocation = CurrentZipline->GetAttachmentPoint() + CurrentZipline->GetSlidingDirection() * GetProjectionToZiplineCable(Zipline, GetActorLocation());;

	GetOwner()->SetActorLocation(NewCharacterLocation);
	GetOwner()->SetActorRotation(TargetOrientationRotation);

	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_OnZipline);
}

float UGCBaseCharacterMovementComponent::GetProjectionToActor(const AActor* Actor, const FVector& Location) const
{
	checkf(IsValid(Actor), TEXT("UGCBaseCharacterMovementComponent::GetCharacterToActorProjection() cant be invoked then actor in NULL"));

	FVector ActorUpVector = Actor->GetActorUpVector();
	FVector ActorToCharacterDistance = Location - Actor->GetActorLocation();

	return FVector::DotProduct(ActorUpVector, ActorToCharacterDistance);
}

float UGCBaseCharacterMovementComponent::GetProjectionToZiplineCable(const AZipline* Zipline, const FVector& CharacterLocation) const
{
	checkf(IsValid(Zipline), TEXT("UGCBaseCharacterMovementComponent::GetCharacterToActorProjection() cant be invoked then actor in NULL"));

	FVector ZiplineSlidingDirection = Zipline->GetSlidingDirection();
	FVector ActorToCharacterDistance = CharacterLocation - Zipline->GetHigherPoleTop();

	return FVector::DotProduct(ActorToCharacterDistance, ZiplineSlidingDirection);
}

float UGCBaseCharacterMovementComponent::GetOnLadderSpeedRatio() const
{
	checkf(IsValid(CurrentLadder), TEXT("UGCBaseCharacterMovementComponent::GetOnLadderSpeedRatio cant be invoked then ladder in NULL"));

	FVector LadderUpVector = CurrentLadder->GetActorUpVector();
	return FVector::DotProduct(LadderUpVector, Velocity) / ClimbingOnLadderMaxSpeed;
}

void UGCBaseCharacterMovementComponent::DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod)
{
	switch (DetachFromLadderMethod)
	{
		case EDetachFromLadderMethod::JumpOff:
		{
			FVector JumpDirection = CurrentLadder->GetActorForwardVector();
			SetMovementMode(MOVE_Falling);
			FVector JumpVelocity = JumpDirection * JumpOffFromLadderSpeed;

			ForcedTargetRotation = JumpDirection.ToOrientationRotator();
			bForceRotation = true;
			Launch(JumpVelocity);
			break;
		}

		case EDetachFromLadderMethod::ReachingTheTop:
		{
			GetBaseCharacterOwner()->Mantle(true);
			break;
		}

		case EDetachFromLadderMethod::ReachingTheBottom:
		{
			SetMovementMode(MOVE_Walking);
			break;
		}

		case EDetachFromLadderMethod::Fall:
		default:
		{
			SetMovementMode(MOVE_Falling);
		}
	}
}

void UGCBaseCharacterMovementComponent::DetachFromZipline()
{
	SetMovementMode(MOVE_Falling);
}

bool UGCBaseCharacterMovementComponent::IsOnLadder() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_OnLadder;
}

bool UGCBaseCharacterMovementComponent::IsOnZipline() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_OnZipline;
}

bool UGCBaseCharacterMovementComponent::IsWallRunning() const
{
	return UpdatedComponent && MovementMode == MOVE_Custom && CustomMovementMode == (uint8)ECustomMovementMode::CMOVE_WallRun;
}

const class ALadder* UGCBaseCharacterMovementComponent::GetCurrentLadder()
{
	return CurrentLadder;
}

void UGCBaseCharacterMovementComponent::PhysicsRotation(float DeltaTime)
{
	if (bForceRotation)
	{
		FRotator CurrentRotation = UpdatedComponent->GetComponentRotation(); // Normalized
		CurrentRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): CurrentRotation"));

		FRotator DeltaRot = GetDeltaRotation(DeltaTime);
		DeltaRot.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): GetDeltaRotation"));

		// Accumulate a desired new rotation.
		const float AngleTolerance = 1e-3f;

		if (!CurrentRotation.Equals(ForcedTargetRotation, AngleTolerance))
		{
			FRotator DesiredRotation = ForcedTargetRotation;
			// PITCH
			if (!FMath::IsNearlyEqual(CurrentRotation.Pitch, DesiredRotation.Pitch, AngleTolerance))
			{
				DesiredRotation.Pitch = FMath::FixedTurn(CurrentRotation.Pitch, DesiredRotation.Pitch, DeltaRot.Pitch);
			}

			// YAW
			if (!FMath::IsNearlyEqual(CurrentRotation.Yaw, DesiredRotation.Yaw, AngleTolerance))
			{
				DesiredRotation.Yaw = FMath::FixedTurn(CurrentRotation.Yaw, DesiredRotation.Yaw, DeltaRot.Yaw);
			}

			// ROLL
			if (!FMath::IsNearlyEqual(CurrentRotation.Roll, DesiredRotation.Roll, AngleTolerance))
			{
				DesiredRotation.Roll = FMath::FixedTurn(CurrentRotation.Roll, DesiredRotation.Roll, DeltaRot.Roll);
			}

			// Set the new rotation.
			DesiredRotation.DiagnosticCheckNaN(TEXT("CharacterMovementComponent::PhysicsRotation(): DesiredRotation"));
			MoveUpdatedComponent(FVector::ZeroVector, DesiredRotation, /*bSweep*/ false);
		}
		else
		{
			ForcedTargetRotation = FRotator::ZeroRotator;
			bForceRotation = false;
		}
		return;
	}

	if (IsOnLadder() || IsOnZipline() || IsWallRunning())
	{
		return;
	}

	if (IsSwimming())
	{
		RotationRate.Roll = 0.0f;
	}

	Super::PhysicsRotation(DeltaTime);
}

float UGCBaseCharacterMovementComponent::GetCurrentMoveForwardInput()
{
	return CurrentMoveForwardInput;
}

float UGCBaseCharacterMovementComponent::GetCurrentMoveRightInput()
{
	return CurrentMoveRightInput;
}

void UGCBaseCharacterMovementComponent::PhysCustom(float DeltaTime, int32 Iterations)
{
	switch (CustomMovementMode)
	{
		case (uint8)ECustomMovementMode::CMOVE_Mantling:
		{
			PhysMantling(DeltaTime, Iterations);
			break;
		}
		case (uint8)ECustomMovementMode::CMOVE_OnLadder:
		{
			PhysOnLadder(DeltaTime, Iterations);
			break;
		}
		case (uint8)ECustomMovementMode::CMOVE_OnZipline:
		{
			PhysOnZipline(DeltaTime, Iterations);
			break;
		}
		case (uint8)ECustomMovementMode::CMOVE_WallRun:
		{
			PhysWallRun(DeltaTime, Iterations);
			break;
		}
		case (uint8)ECustomMovementMode::CMOVE_Sliding:
		{
			PhysSliding(DeltaTime, Iterations);
			break;
		}
		default:
			break;
	}
	Super::PhysCustom(DeltaTime, Iterations);
}

void UGCBaseCharacterMovementComponent::PhysFalling(float DeltaTime, int32 Iterations)
{
	Super::PhysFalling(DeltaTime, Iterations);
	if (!bIsWallRunning)
	{
		UpdateWallrun();
	}
}

void UGCBaseCharacterMovementComponent::PhysMantling(float DeltaTime, int32 Iterations)
{
	float ElapsedTime = GetWorld()->GetTimerManager().GetTimerElapsed(MantlingTimer) + CurrentMantlingParameters.StartTime;
	FVector MantlingCurveValue = CurrentMantlingParameters.MantlingCurve->GetVectorValue(ElapsedTime);

	float PositionAlpha = MantlingCurveValue.X;
	float XYCorrectionAlpha = MantlingCurveValue.Y;
	float ZCorrectionAlpha = MantlingCurveValue.Z;

	FVector TargetLocation = FVector::ZeroVector;
	
	if (CurrentMantlingParameters.LedgeActor.IsValid())
	{
		TWeakObjectPtr<AActor> Actor = CurrentMantlingParameters.LedgeActor;
		UActorComponent* MeshComponent = CurrentMantlingParameters.LedgeActor->GetComponentByClass(UStaticMeshComponent::StaticClass());

		UPrimitiveComponent* PrimitiveComponent = StaticCast<UPrimitiveComponent*>(MeshComponent);

		FVector LocalInitialTargetLocation = CurrentMantlingParameters.TargetCharacterLocation - CurrentMantlingParameters.LedgeActorInitialLocation;
		TargetLocation = PrimitiveComponent->GetComponentLocation() + LocalInitialTargetLocation;
	}
	else
	{
		TargetLocation = CurrentMantlingParameters.TargetCharacterLocation;
	}

	FVector CorrectedInitialLocation = FMath::Lerp(CurrentMantlingParameters.InitialCharacterLocation, CurrentMantlingParameters.InitialAnimationCharacterLocation, XYCorrectionAlpha);
	CorrectedInitialLocation.Z = FMath::Lerp(CurrentMantlingParameters.InitialCharacterLocation.Z, CurrentMantlingParameters.InitialAnimationCharacterLocation.Z, ZCorrectionAlpha);

	FVector NewLocation = FMath::Lerp(CorrectedInitialLocation, TargetLocation, PositionAlpha);
	FRotator NewRotation = FMath::Lerp(CurrentMantlingParameters.InitialCharacterRotation, CurrentMantlingParameters.TargetCharacterRotation, PositionAlpha);

	FVector Delta = NewLocation - GetActorLocation();
	FHitResult HitResult;

	SafeMoveUpdatedComponent(Delta, NewRotation, false, HitResult);
}

void UGCBaseCharacterMovementComponent::PhysOnLadder(float DeltaTime, int32 Iterations)
{
	CalcVelocity(DeltaTime, 1.0f, false, ClimbingOnLadderBreakingDecelaration);
	FVector Delta = Velocity * DeltaTime;

	if (HasAnimRootMotion())
	{
		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), false, Hit);
		return;
	}

	FVector NewPos = GetActorLocation() + Delta;
	float NewPosProjection = GetProjectionToActor(CurrentLadder, NewPos);

	if (NewPosProjection < MinLadderBottomOffset)
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheBottom);
		return;
	}
	else if (NewPosProjection > (CurrentLadder->GetLadderHeight() - MaxLadderTopOffset))	
	{
		DetachFromLadder(EDetachFromLadderMethod::ReachingTheTop);
	}
	FHitResult Hit;
	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

void UGCBaseCharacterMovementComponent::PhysOnZipline(float DeltaTime, int32 Iterations)
{
	if (bIsZiplineDelayTimerElapsed)
	{
		FVector Delta = CurrentZipline->GetSlidingDirection() * ZiplineSpeed * DeltaTime;

		FVector NewPos = GetActorLocation() + Delta;

		FHitResult Hit;
		float NewPosProjection = GetProjectionToZiplineCable(CurrentZipline, NewPos);

		if (NewPosProjection > CurrentZipline->GetZiplineLength())
		{
			SetMovementMode(MOVE_Falling);
			return;
		}
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
	}
}

void UGCBaseCharacterMovementComponent::PhysWallRun(float DeltaTime, int32 Iterations)
{
	UpdateWallrun();

	if (IsWallRunning())
	{
		FVector Delta = CurrentWallRunParameters.Direction * MaxWallRunSpeed * DeltaTime;

		FHitResult Hit;
		SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
	}
}

void UGCBaseCharacterMovementComponent::OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode)
{
	Super::OnMovementModeChanged(PreviousMovementMode, PreviousCustomMode);
	ACharacter* DefaultCharacter = CharacterOwner->GetClass()->GetDefaultObject<ACharacter>();

	if (MovementMode == MOVE_Swimming)
	{
		GEngine->AddOnScreenDebugMessage(801, 5.0f, FColor::Blue, FString::Printf(TEXT("SWIMMING")));
		bIsSwimming = true;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(SwimmingCapsuleRadius, SwimmingCapsuleHalfHeight);
	}
	else if (PreviousMovementMode == MOVE_Swimming)
	{
		bIsSwimming = false;
		CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
		DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());
	}

	if (MovementMode == MOVE_Walking && PreviousMovementMode == MOVE_Falling)
	{
		CurrentWallRunParameters.PreviousSide = EWallRunSide::None;
	}

	if (MovementMode == MOVE_Falling)
	{
		GEngine->AddOnScreenDebugMessage(801, 5.0f, FColor::Magenta, FString::Printf(TEXT("FALLING")));
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_OnLadder)
	{
		CurrentLadder = nullptr;
	}

	if (PreviousMovementMode == MOVE_Custom && PreviousCustomMode == (uint8)ECustomMovementMode::CMOVE_OnZipline)
	{
		CurrentZipline = nullptr;
	}

	if (MovementMode == MOVE_Custom)
	{
		switch (CustomMovementMode)
		{
		case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{
				GetWorld()->GetTimerManager().SetTimer(MantlingTimer, this, &UGCBaseCharacterMovementComponent::EndMantle, CurrentMantlingParameters.Duration, false);
				break;
			}
		case (uint8)ECustomMovementMode::CMOVE_OnZipline:
			{	
				bIsZiplineDelayTimerElapsed = false;
				GetWorld()->GetTimerManager().SetTimer(ZiplineDelayTimer, this, &UGCBaseCharacterMovementComponent::OnZiplineDelayTimerElapsed, CurrentZipline->GetAttachmentDelayTime(), false);
				break;
			}
		case (uint8)ECustomMovementMode::CMOVE_Sliding:
			{

				CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(
					DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
					SlideCaspsuleHalfHeight);

				CachedSlidingMeshPosition = CharacterOwner->GetMesh()->GetRelativeLocation();

				CharacterOwner->GetMesh()->MoveComponent(FVector(0.0f, 0.0f, -(DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - SlideCaspsuleHalfHeight)),
					CharacterOwner->GetMesh()->GetComponentQuat(), true, nullptr, EMoveComponentFlags::MOVECOMP_NoFlags, ETeleportType::TeleportPhysics);	
			}
		break;
		default:
				break;
		}
	}

	if (PreviousMovementMode == MOVE_Custom)
	{
		switch (PreviousCustomMode)
		{
			case (uint8)ECustomMovementMode::CMOVE_Mantling:
			{

			}
			break;
			case (uint8)ECustomMovementMode::CMOVE_OnZipline:
			{

			}
			break;
			case (uint8)ECustomMovementMode::CMOVE_Sliding:
			{
				FHitResult HitResult;
				FCollisionQueryParams QueryParams;
				QueryParams.bTraceComplex = true;
				QueryParams.AddIgnoredActor(GetOwner());
				UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
				bool bIsDebugEnabled = DebugSubsystem->IsCategoryEnabled(DebugCategorySlide);

				CharacterOwner->GetCapsuleComponent()->SetCapsuleSize(
					DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
					DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight());

				if (GCTraceUtils::SweepCapsuleSingleByChannel(GetWorld(), HitResult, CharacterOwner->GetActorLocation() + FVector(0.0f, 0.0f, DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - SlideCaspsuleHalfHeight),
					CharacterOwner->GetActorLocation() + FVector(0.0f, 0.0f, DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight() - SlideCaspsuleHalfHeight), ECC_Visibility, DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleRadius(),
					DefaultCharacter->GetCapsuleComponent()->GetUnscaledCapsuleHalfHeight(), FQuat::Identity,
					QueryParams, FCollisionResponseParams::DefaultResponseParam, bIsDebugEnabled, 3.0f))
				{
					Crouch();
				}

				CharacterOwner->GetMesh()->SetRelativeLocation(CachedSlidingMeshPosition);
			}
			break;
			default:
				break;
		}	
	}
}

void UGCBaseCharacterMovementComponent::OnZiplineDelayTimerElapsed()
{
	bIsZiplineDelayTimerElapsed = true;	
}

AGCBaseCharacter* UGCBaseCharacterMovementComponent::GetBaseCharacterOwner() const
{
	return StaticCast<AGCBaseCharacter*>(CharacterOwner);
}

void UGCBaseCharacterMovementComponent::UpdateWallrun()
{
	FWallRunParameters WallRunParameters;
	if (GetBaseCharacterOwner()->GetWallRunComponent()->DetectWall(WallRunParameters))
	{

		if ((!IsSurfaceWallRunnable(WallRunParameters.WallNormal))  ||
			(!AreWallRunRequiredKeysDown(WallRunParameters.Side)))
		{
			if (IsWallRunning())
			{
				StopWallrun();
			}
			return;
		}

		if (WallRunParameters.Side == CurrentWallRunParameters.PreviousSide)
		{
			return;
		}

		CurrentWallRunParameters.Side = WallRunParameters.Side;
		CurrentWallRunParameters.Direction = WallRunParameters.Direction;
		CurrentWallRunParameters.WallNormal = WallRunParameters.WallNormal;
		GetOwner()->SetActorRotation(CurrentWallRunParameters.Direction.ToOrientationRotator());

		if (!IsWallRunning())
		{
			StartWallrun(WallRunParameters.Side, WallRunParameters.Direction, WallRunParameters.WallNormal);
		}
	}
	else
	{
		if (IsWallRunning())
		{
			StopWallrun();
		}
	}
}

bool UGCBaseCharacterMovementComponent::IsSurfaceWallRunnable(const FVector& SurfaceNormal)
{
	if (SurfaceNormal.Z > GetWalkableFloorZ() || SurfaceNormal.Z < -0.005f)
	{
		return false;
	}

	return true;
}

void UGCBaseCharacterMovementComponent::CacheForwardMovementInput(float MoveForwardInput)
{
	CurrentMoveForwardInput = MoveForwardInput;
}

void UGCBaseCharacterMovementComponent::CacheRightMovementInput(float MoveRightInput)
{
	CurrentMoveRightInput = MoveRightInput;
}

bool UGCBaseCharacterMovementComponent::AreWallRunRequiredKeysDown(EWallRunSide Side)
{
    if (CurrentMoveForwardInput < 0.1f) 
	{
		return false; 
	}
	
	if (Side == EWallRunSide::Left && CurrentMoveRightInput > 0.0f)
	{
		return false;
	}

	if (Side == EWallRunSide::Right && CurrentMoveRightInput < 0.0f)
	{
		return false;
	}

	return true;
}

void UGCBaseCharacterMovementComponent::StartWallrun(EWallRunSide Side, FVector Direction, FVector WallNormal)
{
	bIsWallRunning = true;

	GetOwner()->SetActorRotation(Direction.ToOrientationRotator());
	GetOwner()->SetActorLocation(GetOwner()->GetActorLocation() + WallNormal * CharacterToWallOffset);

	GetWorld()->GetTimerManager().SetTimer(WallRunTimer, this, &UGCBaseCharacterMovementComponent::StopWallrun, MaxWallRunTime, false);
	SetMovementMode(MOVE_Custom, (uint8)ECustomMovementMode::CMOVE_WallRun);
}

void UGCBaseCharacterMovementComponent::StopWallrun()
{
	bIsWallRunning = false;
	if (GetWorld()->GetTimerManager().GetTimerElapsed(WallRunTimer) > EnoughTimeToSaveWallRunSide)
	{
		CurrentWallRunParameters.PreviousSide = CurrentWallRunParameters.Side;
	}
	else
	{
		CurrentWallRunParameters.PreviousSide = EWallRunSide::None;
	}
	CurrentWallRunParameters.Side = EWallRunSide::None;
	CurrentWallRunParameters.Direction = FVector::ZeroVector;
	GetWorld()->GetTimerManager().ClearTimer(WallRunTimer);
	SetMovementMode(MOVE_Falling);
}

EWallRunSide UGCBaseCharacterMovementComponent::GetCurrentWallRunSide() const
{
	return CurrentWallRunParameters.Side;
}

void UGCBaseCharacterMovementComponent::JumpWhileWallRun()
{
	if (GetWorld()->GetTimerManager().GetTimerElapsed(WallRunTimer) < EnoughTimeToSaveWallRunSide)
	{
		return;
	}
	FVector JumpDirectionHorizontal = CurrentWallRunParameters.Direction + CurrentWallRunParameters.WallNormal;
	JumpDirectionHorizontal.Normalize();
	FVector JumpDirectionVertical = FVector::UpVector / 2;

	FVector JumpDirection = JumpDirectionHorizontal + JumpDirectionVertical;

	FVector JumpVelocity = JumpDirection * WallRunJumpOffSpeed;

	ForcedTargetRotation = JumpDirection.ToOrientationRotator();
	bForceRotation = true;
	Launch(JumpVelocity);
	StopWallrun();
}

bool UGCBaseCharacterMovementComponent::IsSliding() const
{
	return bIsSliding;
}

void UGCBaseCharacterMovementComponent::PhysSliding(float DeltaTime, int32 Iterations)
{
	FVector Delta = GetCharacterOwner()->GetActorForwardVector() * SlideSpeed * DeltaTime;

	FVector NewPos = GetActorLocation() + Delta;

	FHitResult Hit;

	SafeMoveUpdatedComponent(Delta, GetOwner()->GetActorRotation(), true, Hit);
}

