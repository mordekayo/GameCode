// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "../LedgeDetectorComponent.h"
#include "../WallRunComponent.h"
#include "GCBaseCharacterMovementComponent.generated.h"

struct FMantlingMovementParameters
{
	FVector InitialCharacterLocation = FVector::ZeroVector;
	FRotator InitialCharacterRotation = FRotator::ZeroRotator;

	FVector TargetCharacterLocation = FVector::ZeroVector;
	FRotator TargetCharacterRotation = FRotator::ZeroRotator;

	TWeakObjectPtr<AActor> LedgeActor;
	FVector LedgeActorInitialLocation = FVector::ZeroVector;

	float Duration = 1.0f;
	float StartTime = 0.0f;

	FVector InitialAnimationCharacterLocation = FVector::ZeroVector;

	UCurveVector* MantlingCurve;
};

UENUM(BlueprintType)
enum class ECustomMovementMode: uint8
{
	CMOVE_None = 0 UMETA(DisplayName = "None"),
	CMOVE_Mantling UMETA(DisplayName = "Mantling"),
	CMOVE_WallRun UMETA(DisplayName = "Wall run"),
	CMOVE_OnLadder UMETA(DisplayName = "Ladder"),
	CMOVE_OnZipline UMETA(DisplayName = "Zipline"),
	CMOVE_Sliding UMETA(DisplayName = "Sliding"),
	CMOVE_Max UMETA(Hidden)
};

UENUM(BlueprintType)
enum class EDetachFromLadderMethod : uint8
{
	Fall = 0,
	ReachingTheTop,
	ReachingTheBottom,
	JumpOff
};

/**
 * 
 */
UCLASS()
class GAMECODE_API UGCBaseCharacterMovementComponent : public UCharacterMovementComponent
{
	GENERATED_BODY()

public:
	void CacheForwardMovementInput(float MoveForwardInput);
	void CacheRightMovementInput(float MoveRigthInput);
	bool IsSurfaceWallRunnable(const FVector& SurfaceNormal);
	bool AreWallRunRequiredKeysDown(EWallRunSide Side);
	void StartWallrun(EWallRunSide Side, FVector Direction, FVector WallNormal);
	void StopWallrun();
	EWallRunSide GetCurrentWallRunSide() const;
	void JumpWhileWallRun();
	bool IsWallRunning() const;

	FORCEINLINE bool IsOutOfStamina() const { return bIsOutOfStamina; }
	void SetIsOutOfStamina(bool bIsOutOfStamina_In);

	bool IsProning() const {return bIsProning; }
	bool IsSprinting() const { return bIsSprinting; }
	bool IsSwimming() const { return bIsSwimming; }
	virtual float GetMaxSpeed() const override;

	bool bWantsToProne = false;
	
	void StartSprint();
	void StopSprint();

	void StartSlide();
	void StopSlide();
	bool IsSliding() const;
	void OnSlidingStandUpTimerElapsed();

	void StartMantle(const FMantlingMovementParameters& MantlingParameters);
	void EndMantle();
	bool IsMantling() const;
	void AttachToLadder(const class ALadder* Ladder);
	float GetOnLadderSpeedRatio() const;
	void DetachFromLadder(EDetachFromLadderMethod DetachFromLadderMethod = EDetachFromLadderMethod::Fall);
	bool IsOnLadder() const;
	const class ALadder* GetCurrentLadder();

	void AttachToZipline(const class AZipline* Zipline);
	float GetProjectionToZiplineCable(const AZipline* Zipline, const FVector& Location) const;
	void DetachFromZipline();
	bool IsOnZipline() const;
	
	float GetProjectionToActor(const AActor* Actor, const FVector& CharacterLocation) const;

	virtual void PhysicsRotation(float DeltaTime) override;

	float GetCurrentMoveForwardInput();
	float GetCurrentMoveRightInput();

	virtual void UpdateCharacterStateBeforeMovement(float DeltaSeconds) override;
	virtual void UpdateCharacterStateAfterMovement(float DeltaSeconds) override;

	bool CanProneInCurrentState() const;
	
	UPROPERTY(Category="Character Movement (General Settings)", VisibleInstanceOnly, BlueprintReadWrite, AdvancedDisplay)
	uint8 bProneMaintainsBaseLocation:1;
protected:

	virtual void PhysCustom(float DeltaTime, int32 Iterations) override;

	virtual void PhysFalling(float DeltaTime, int32 Iterations) override;

	void PhysMantling(float DeltaTime, int32 Iterations);
	void PhysOnLadder(float DeltaTime, int32 Iterations);
	void PhysOnZipline(float DeltaTime, int32 Iterations);
	void PhysWallRun(float DeltaSeconds, int32 iterations);
	void PhysSliding(float DeltaTime, int32 Iterations);
	

	virtual void OnMovementModeChanged(EMovementMode PreviousMovementMode, uint8 PreviousCustomMode) override;

	void UpdateWallrun();
	void OnZiplineDelayTimerElapsed();
	class AGCBaseCharacter* GetBaseCharacterOwner() const;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: prone", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ProneCapsuleRadius = 40.0f; // Радуиус капсулы в состоянии лежа
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: prone", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ProneCapsuleHalfHeight = 40.0f; // Половина высоты капсулы в состоянии лежа.
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: sprint", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SprintSpeed = 1000.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Character movement: sprint", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float OutOfStaminaMaxSpeed = 150.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Swimming", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SwimmingCapsuleRadius = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Swimming", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float SwimmingCapsuleHalfHeight = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ClimbingOnLadderMaxSpeed = 300.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float ClimbingOnLadderBreakingDecelaration = 2048.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LadderToCharacterOffset = 60.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxLadderTopOffset = 120.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinLadderBottomOffset = 90.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float JumpOffFromLadderSpeed = 500.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: Ladder", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LadderStartTranslation = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement : Zipline")
	float ZiplineSpeed = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: WallRun")
	float MaxWallRunSpeed = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: WallRun", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxWallRunTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: WallRun", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float CharacterToWallOffset = 5.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character movement: WallRun", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float WallRunJumpOffSpeed = 400.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: WallRun", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float WallRunJumpOffVerticalSpeed = 100.0f;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: WallRun", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float WallRunJumpCameraYawRotation = 10.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character movement: WallRun", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float EnoughTimeToSaveWallRunSide = 0.05f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideSpeed = 1000.0f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideCaspsuleHalfHeight = 50.0f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideMaxTime = 1.53f;

	UPROPERTY(Category = "Character Movement: Slide", EditAnywhere, BlueprintReadWrite, meta = (ClampMin = "0", UIMin = "0"))
	float SlideStandUpTime = 0.2f;
private:

	float CurrentMoveForwardInput = 0.0f;
	float CurrentMoveRightInput = 0.0f;

	bool bIsWallRunning;
	FWallRunParameters CurrentWallRunParameters;
	FTimerHandle WallRunTimer;
	FTimerHandle WallRunJumpCameraTimer;

	bool bIsSprinting;
	bool bIsSwimming = false;
	bool bIsOutOfStamina;
	bool bIsSliding;
	bool bIsProning;
	
	FTimerHandle SlidingTimer;
	FTimerHandle SlidingEndTimer;
	FVector CachedSlidingMeshPosition = FVector::ZeroVector;

	FMantlingMovementParameters CurrentMantlingParameters;
	FTimerHandle MantlingTimer;

	const ALadder* CurrentLadder = nullptr;
	FRotator ForcedTargetRotation = FRotator::ZeroRotator;
	bool bForceRotation = false;

	FVector ZiplineDirection = FVector::ZeroVector;
	const AZipline* CurrentZipline = nullptr;
	FTimerHandle ZiplineDelayTimer;
	bool bIsZiplineDelayTimerElapsed = false;
};
