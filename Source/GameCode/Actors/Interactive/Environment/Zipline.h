// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "../InteractiveActor.h"
#include "Zipline.generated.h"

class UBoxComponent;
class UStaticMeshComponent;
class UArrowComponent;

/**
 * 
 */
UCLASS(Blueprintable)
class GAMECODE_API AZipline : public AInteractiveActor
{
	GENERATED_BODY()
	
public:
	AZipline();

	virtual void OnConstruction(const FTransform& Transform) override;

	virtual void BeginPlay() override;
	virtual void Tick(float DeltaTime) override;

	class UBoxComponent* GetZiplineInteractionBox() const;

	FVector GetSlidingDirection() const;
	FVector GetHigherPoleTop() const;
	FVector GetLowerPoleTop() const;
	FVector GetDestinationPoint() const;
	FVector GetAttachmentPoint() const;
	float GetZiplineLength() const;
	float GetAttachmentDelayTime() const;
protected:
	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UStaticMeshComponent* HigherPoleStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UStaticMeshComponent* LowerPoleStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)	
	UStaticMeshComponent* CableStaticMeshComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadonly)
	UArrowComponent* SlidingDirectionArrow;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float HigherPoleHeight = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float LowerPoleHeight = 250.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters", meta = (MakeEditWidget))
	FVector LowerPoleLocation = FVector(300.0f, 0.0f, 0.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float OutOfCableInteractionArea = 50.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float CableInteractionBoxWidth = 30.0f;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	FVector CharacterAttachToZiplineZOffset = FVector(0.0f,0.0f,-127.0f);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "Zipline parameters")
	float AttachmentDelayTime = 100.0f;

	virtual void OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult);
	virtual void OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex);

private:
	FVector HigherPoleTop = FVector::ZeroVector;
	FVector LowerPoleTop = FVector::ZeroVector;
	FVector CableVector = FVector::ZeroVector;
	FVector SlidingDirection = FVector::ZeroVector;
	FVector AttachmentPoint = FVector::ZeroVector;
	FVector DestinationPoint = FVector::ZeroVector;
};
