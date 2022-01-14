// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "WallRunComponent.generated.h"

UENUM(BlueprintType)
enum class EWallRunSide : uint8
{
	None,
	Left,
	Right
};

USTRUCT(BlueprintType)
struct FWallRunParameters
{
	GENERATED_BODY()

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall description")
	EWallRunSide Side = EWallRunSide::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall description")
	EWallRunSide PreviousSide = EWallRunSide::None;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall description")
	FVector Direction = FVector::ZeroVector;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Wall description")
	FVector WallNormal = FVector::ZeroVector;
};

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UWallRunComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	bool DetectWall(OUT FWallRunParameters& WallRunParameters);
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection settings", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float WallSearchTraceDistance = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection settings", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float WallSearchStartOffset = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Detection settings", meta = (UIMin = 0.0f, ClampMin = 0.0f))
	float FloorSearchTraceDistance = 20.0f;

private:
	EWallRunSide CurrentWallRunSide = EWallRunSide::None;
	FVector WallRunDirection = FVector::ZeroVector;
	TWeakObjectPtr<class ACharacter> CachedCharacterOwner;
};
