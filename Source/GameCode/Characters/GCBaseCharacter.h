// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "../Components/MovementComponents/GCBaseCharacterMovementComponent.h"
#include "GCBaseCharacter.generated.h"

USTRUCT(BlueprintType)
struct FMantlingSettings
{
	GENERATED_BODY()

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* MantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UAnimMontage* FPMantlingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	class UCurveVector* MantlingCurve;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionXY = 65.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float AnimationCorrectionZ = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeight = 200.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeight = 100.0f; 

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MaxHeightStartTime = 0.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float MinHeightStartTime = 0.6f;
};

class AInteractiveActor;
class UCharacterEquipmentComponent;
class UCharacterAttributesComponent;

UCLASS(Abstract, NotBlueprintable)
class GAMECODE_API AGCBaseCharacter : public ACharacter
{
	GENERATED_BODY()

public:
	AGCBaseCharacter(const FObjectInitializer& ObjectInitializer);
	
	virtual void Tick(float DeltaTime) override;
	virtual void BeginPlay() override;
	virtual void PossessedBy(AController* NewController) override;

	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;

	virtual void MoveForward(float Value) {};
	virtual void MoveRight(float Value) {};
	virtual void Turn(float Value) {};
	virtual void LookUp(float Value) {};
	virtual void TurnAtRate(float Value) {};
	virtual void LookUpAtRate(float Value) {};
	virtual void ChangeCrouchState();

	virtual void StartSprint();
	virtual void StopSprint();
	virtual void StartSlide();
	virtual void StopSlide();
	virtual void SwimForward(float Value);
	virtual void SwimRight(float Value);
	virtual void SwimUp(float Value);

	void Mantle(bool bForce = false);

	void StartFire();
	void StopFire();

	void StartAiming();
	void StopAiming();

	bool IsAiming() const;

	virtual void OnMantle(const FMantlingSettings& MantlingSettings, float MantlingAnimationStartTime);

	virtual void StartMantleRequesting();

	virtual void StopMantleRequesting();

	void TryWallRun();

	virtual void Falling() override;
	virtual void Landed(const FHitResult& Hit);

	virtual void NotifyJumpApex() override;

	bool IsSwimmingUnderWater() const;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKLeftFootOffset() const { return IKLeftFootOffset;}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKRightFootOffset() const { return IKRightFootOffset;}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	float GetIKPelvisOffset() const { return IKPelvisOffset; }

	bool IsLedgeDetectionDebugDrawEnabled() const { return bIsLedgeDetectionDebugDrawEnabled; }

	UPROPERTY(ReplicatedUsing=OnRep_IsMantling)
	bool bIsMantling;

	UFUNCTION()
	void OnRep_IsMantling(bool bWasMantling);
	
	virtual bool CanJumpInternal_Implementation() const;

	class UGCBaseCharacterMovementComponent* GetBaseCharacterMovementComponent() const;

	void RegisterInteractiveActor(AInteractiveActor* InteractiveActor);
	void UnregisterInteractiveActor(AInteractiveActor* InteractiveActor);

	void ClimbLadderUp(float Value);
	void InteractWithLadder();
	void InteractWithZipline();
	const class ALadder* GetAvailableLadder() const;

	const class AZipline* GetAvailableZipline() const;

	void JumpWhileWallRunning();
	UWallRunComponent* GetWallRunComponent();

	const UCharacterEquipmentComponent* GetCharacterEquipmentComponent() const;
	const UCharacterAttributesComponent* GetCharacterAttributesComponent() const;
protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK settings")
	bool bIKEnabled = false;

	UFUNCTION(BlueprintNativeEvent, Category = "Character | Movement")
	void OnSprintStart();
	virtual void OnSprintStart_Implementation();

	UFUNCTION(BlueprintnativeEvent, Category = "Character | Movement")
	void OnSprintEnd();

	virtual void OnSprintEnd_Implementation();

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement")
	float MaxStamina = 100.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement")
	float StaminaRestoreVelocity = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement")
	float SprintStaminaConsumptionVelocity = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseTurnRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Controls")
	float BaseLookUpRate = 45.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement")
	float SprintSpeed = 800.0f;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Character | Movement")
	float HardLandingMinHeight = 800.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK settings")
	FName LeftFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK settings")
	FName RightFootSocketName;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKTraceDistance = 60.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "IK settings", meta = (ClampMin = 0.f, UIMin = 0.f))
	float IKInterpSpeed = 30.0f;

	virtual bool CanSprint();

	virtual bool CanCrouch();

	bool CanMantle() const; 

	virtual void HardLandingStart();

	UGCBaseCharacterMovementComponent* GCBaseCharacterMovementComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class ULedgeDetectorComponent* LedgeDetectorComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UWallRunComponent* WallRunComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UCharacterAttributesComponent* CharacterAttributesComponent;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings HighMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling")
	FMantlingSettings LowMantleSettings;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | Mantling", meta = (ClampMin = 0.0f, UIMin = 0.0f))
	float LowMantleMaxHeight = 125;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | HardLanding")
	class UAnimMontage* HardLandingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Movement | HardLanding")
	class UAnimMontage* FPHardLandingMontage;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Animations")
	class UAnimMontage* OnDeathAnimMontage;

	//Damage depending from fall height (in meters)
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Character | Attributes")
	class UCurveFloat* FallDamageCurve;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Character | Components")
	class UCharacterEquipmentComponent* CharacterEquipmentComponent;

	virtual void OnDeath();
	virtual void OnStaminaOutOrMax(bool MaxOrOut);
private:

	const FMantlingSettings& GetMantlingSettings(float LedgeHeight) const;
	void EnableRagdoll();

	UFUNCTION(exec)
	void ToggleLedgeDetectionDebugDraw() { bIsLedgeDetectionDebugDrawEnabled = !bIsLedgeDetectionDebugDrawEnabled;  }

	bool bIsAiming = false;
	
	bool bIsSprintRequested = false;
	bool bIsMantleRequested = false;
	void TryChangeSprintState(float DeltaTime);
	bool bIsLedgeDetectionDebugDrawEnabled = false;
	float CalculateIKOffsetForASocket(const FName& SocketName) const;
	float CalculateIKPelvisOffset() const;
	void UpdateIKSettings(float DeltaSeconds);
	void HardLandingEnd();
	float IKLeftFootOffset = 0.0f;
	float IKRightFootOffset = 0.0f;
	float IKPelvisOffset = 0.0f;
	float CurrentStamina;
	float ApexCharacterZPozition;
	FVector CurrentFallApex;

	FTimerHandle HardLandingTimer;


	TArray<AInteractiveActor*, TInlineAllocator<10>> AvaiableInteractiveActors;
	TWeakObjectPtr<class AGCPlayerController> GCPlayerController;
};
