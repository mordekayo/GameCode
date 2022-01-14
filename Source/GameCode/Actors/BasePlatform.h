// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "Components/TimelineComponent.h"
#include "BasePlatform.generated.h"

UENUM()
enum class EPlatformBehavior : uint8
{
	OnDemand = 0,//платформа перемещается только при запросе
	Loop//платформа перемещается по циклу
};

class APlatformInvocator;

UCLASS()
class GAMECODE_API ABasePlatform : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABasePlatform();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (MakeEditWidget))
		UStaticMeshComponent* PlatformMesh;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly)
	class UBoxComponent* BoxComponent;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Transient)
		FVector StartLocation;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly, meta = (MakeEditWidget))
		FVector EndLocation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		UCurveFloat* TimelineCurve;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
		EPlatformBehavior PlatformBehavior = EPlatformBehavior::OnDemand;

	UPROPERTY(EditInstanceOnly, BlueprintReadOnly)
	APlatformInvocator* Invocator;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta = (UIMin = 0.0f, ClampMin = 0.0f));
	float MovementDelay = 1.0f;

	UPROPERTY(BlueprintReadWrite)
	FTimerHandle DelayTimer;

	UFUNCTION()
	void OnPlatformInvoked();

	UFUNCTION(BlueprintCallable)
	void PlayPlatformTimeline() { PlatformTimeline.Play(); };

	UFUNCTION(BlueprintCallable)
	void ReversePlatformTimeline() { PlatformTimeline.Reverse(); };

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;

private:

	FTimeline PlatformTimeline;

	

	void PlatformTimelineUpdate(float Alpha);
	void PlatformTimelineUpdateWithLooping(const float Alpha);

};
