// Fill out your copyright notice in the Description page of Project Settings.


#include "Zipline.h"
#include "Components/SceneComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/InstancedStaticMeshComponent.h"
#include "Components/BoxComponent.h"
#include "GameCode/GameCodeTypes.h"
#include "Components/ArrowComponent.h"
#include "DrawDebugHelpers.h"

AZipline::AZipline()
{
	RootComponent = CreateDefaultSubobject<USceneComponent>(TEXT("LadderRoot"));

	HigherPoleStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("HigherPole"));;
	HigherPoleStaticMeshComponent->SetupAttachment(RootComponent);

	LowerPoleStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("LowerPole"));
	LowerPoleStaticMeshComponent->SetupAttachment(RootComponent);

	CableStaticMeshComponent = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Cable"));
	CableStaticMeshComponent->SetupAttachment(RootComponent);

	InteractionVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("InteractionVolume"));
	InteractionVolume->SetupAttachment(RootComponent);
	InteractionVolume->SetCollisionProfileName(CollisionProfilePawnInteractionVolume);
	InteractionVolume->SetGenerateOverlapEvents(true);

	SlidingDirectionArrow = CreateDefaultSubobject<UArrowComponent>(TEXT("SlidingDirectionArrow"));
	SlidingDirectionArrow->SetupAttachment(RootComponent);
}

void AZipline::BeginPlay()
{
	Super::BeginPlay();

	HigherPoleTop = HigherPoleStaticMeshComponent->GetComponentLocation() + FVector(0.0f, 0.0f, HigherPoleHeight / 2);
	LowerPoleTop = LowerPoleStaticMeshComponent->GetComponentLocation() + FVector(0.0f, 0.0f, LowerPoleHeight / 2);
	CableVector = (LowerPoleTop - HigherPoleTop);
	SlidingDirection = CableVector.GetClampedToSize(0.0f, 1.0f);
	AttachmentPoint = HigherPoleTop + CharacterAttachToZiplineZOffset;
	DestinationPoint = LowerPoleTop;
}

void AZipline::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);

	DrawDebugSphere(GetWorld(), GetActorLocation() + HigherPoleStaticMeshComponent->GetRelativeLocation(), 32.0f, 8.0f, FColor::Red, false, 5.0f, (uint8)0U, 5.0f); 
}

void AZipline::OnConstruction(const FTransform& Transform)
{
	UStaticMesh* HigherPoleMesh = HigherPoleStaticMeshComponent->GetStaticMesh();
	UStaticMesh* LowerPoleMesh = LowerPoleStaticMeshComponent->GetStaticMesh();
	UStaticMesh* CableMesh = CableStaticMeshComponent->GetStaticMesh();

	if (IsValid(HigherPoleMesh))
	{
		float MeshHeight = HigherPoleMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			HigherPoleStaticMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, HigherPoleHeight / MeshHeight));
		}
	}

	if (IsValid(LowerPoleMesh))
	{
		float MeshHeight = LowerPoleMesh->GetBoundingBox().GetSize().Z;
		if (!FMath::IsNearlyZero(MeshHeight))
		{
			LowerPoleStaticMeshComponent->SetRelativeScale3D(FVector(1.0f, 1.0f, LowerPoleHeight / MeshHeight));
		}
	}

	HigherPoleStaticMeshComponent->SetRelativeLocation(FVector(0.0f, 0.0f, HigherPoleHeight / 2));
	LowerPoleStaticMeshComponent->SetRelativeLocation(FVector(LowerPoleLocation.X,
	LowerPoleLocation.Y, LowerPoleLocation.Z + LowerPoleHeight / 2));

	HigherPoleTop = HigherPoleStaticMeshComponent->GetRelativeLocation() + FVector(0.0f,0.0f, HigherPoleHeight / 2);
	LowerPoleTop = LowerPoleStaticMeshComponent->GetRelativeLocation() + FVector(0.0f, 0.0f, LowerPoleHeight / 2);

	CableVector = LowerPoleTop - HigherPoleTop;
	FVector CableCenter = HigherPoleTop + CableVector / 2;
	CableStaticMeshComponent->SetRelativeLocation(CableCenter);
	HigherPoleStaticMeshComponent->SetRelativeRotation(FRotator(0.0f, FRotationMatrix::MakeFromX(CableVector).Rotator().Pitch, 0.0f), false);
	CableStaticMeshComponent->SetRelativeRotation(FRotationMatrix::MakeFromY(CableVector).Rotator(), false);

	if (IsValid(CableMesh))
	{
		float MeshWidth = CableMesh->GetBoundingBox().GetSize().Y;
		if (!FMath::IsNearlyZero(MeshWidth))
		{
			CableStaticMeshComponent->SetRelativeScale3D(FVector(0.3f, CableVector.Size() / MeshWidth, 0.3f));
		}
	}

	InteractionVolume->SetRelativeLocation(CableStaticMeshComponent->GetRelativeLocation());
	InteractionVolume->SetRelativeRotation(CableStaticMeshComponent->GetRelativeRotation());
	GetZiplineInteractionBox()->SetBoxExtent(FVector(CableInteractionBoxWidth, CableVector.Size() / 2 + OutOfCableInteractionArea * 2, CableInteractionBoxWidth));

	SlidingDirectionArrow->SetRelativeRotation(CableVector.ToOrientationRotator());
}

UBoxComponent* AZipline::GetZiplineInteractionBox() const
{
	return StaticCast<UBoxComponent*>(InteractionVolume);
}

FVector AZipline::GetSlidingDirection() const
{
	return SlidingDirection;
}

FVector AZipline::GetHigherPoleTop() const
{
	return HigherPoleTop;
}

FVector AZipline::GetLowerPoleTop() const
{
	return LowerPoleTop;
}

FVector AZipline::GetDestinationPoint() const
{
	return DestinationPoint;
}

FVector AZipline::GetAttachmentPoint() const
{
	return AttachmentPoint;
}

float AZipline::GetZiplineLength() const 
{
	return CableVector.Size();
}

float AZipline::GetAttachmentDelayTime() const
{
	return AttachmentDelayTime;
}

void AZipline::OnInteractionVolumeOverlapBegin(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex, bool bFromSweep, const FHitResult& SweepResult)
{
	Super::OnInteractionVolumeOverlapBegin(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex, bFromSweep, SweepResult);

	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
	{
		return;
	}
}

void AZipline::OnInteractionVolumeOverlapEnd(UPrimitiveComponent* OverlappedComponent, AActor* OtherActor, UPrimitiveComponent* OtherComp, int32 OtherBodyIndex)
{
	Super::OnInteractionVolumeOverlapEnd(OverlappedComponent, OtherActor, OtherComp, OtherBodyIndex);

	if (!IsOverlappingCharacterCapsule(OtherActor, OtherComp))
	{
		return;
	}
}