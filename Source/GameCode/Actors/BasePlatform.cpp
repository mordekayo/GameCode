// Fill out your copyright notice in the Description page of Project Settings.

#include "BasePlatform.h"
#include "PlatformInvocator.h"
#include "Components/TimelineComponent.h"



// Sets default values
ABasePlatform::ABasePlatform()
{
 	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;

	USceneComponent* DefaultPlatformRoot = CreateDefaultSubobject<USceneComponent>(TEXT("Platform root"));
	RootComponent = DefaultPlatformRoot;

	PlatformMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Platform"));
	PlatformMesh->SetupAttachment(DefaultPlatformRoot);
}

// Called when the game starts or when spawned
void ABasePlatform::BeginPlay()
{
	Super::BeginPlay();
	StartLocation = PlatformMesh->GetRelativeLocation();

	if (IsValid(TimelineCurve))
	{
		FOnTimelineFloatStatic PlatformMovementTimelineUpdate;
		if (PlatformBehavior == EPlatformBehavior::OnDemand)
		{
			PlatformMovementTimelineUpdate.BindUObject(this, &ABasePlatform::PlatformTimelineUpdate);
		}
		else
		{
			PlatformMovementTimelineUpdate.BindUObject(this, &ABasePlatform::PlatformTimelineUpdateWithLooping);
		}
		
		PlatformTimeline.AddInterpFloat(TimelineCurve, PlatformMovementTimelineUpdate);
	}

	if (IsValid(Invocator))
	{
		Invocator->OnInvoctorActivated.AddUObject(this, &ABasePlatform::OnPlatformInvoked);
	}
};

// Called every frame
void ABasePlatform::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	PlatformTimeline.TickTimeline(DeltaTime);
}

void ABasePlatform::PlatformTimelineUpdate(const float Alpha)
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	PlatformMesh->SetRelativeLocation(PlatformTargetLocation);
}

void ABasePlatform::PlatformTimelineUpdateWithLooping(const float Alpha)
{
	const FVector PlatformTargetLocation = FMath::Lerp(StartLocation, EndLocation, Alpha);
	PlatformMesh->SetRelativeLocation(PlatformTargetLocation);
	if (PlatformBehavior == EPlatformBehavior::Loop)
	{
		if (PlatformTargetLocation == EndLocation)
		{
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &ABasePlatform::ReversePlatformTimeline, MovementDelay, false, -1);
		}
		if (PlatformTargetLocation == StartLocation)
		{
			GetWorld()->GetTimerManager().SetTimer(DelayTimer, this, &ABasePlatform::PlayPlatformTimeline, MovementDelay, false, -1);
		}
	}
}

void ABasePlatform::OnPlatformInvoked()
{
	PlatformTimeline.Play();
}

