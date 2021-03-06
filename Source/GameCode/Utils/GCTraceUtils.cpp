#include "GCTraceUtils.h"
#include "DrawDebugHelpers.h"

bool GCTraceUtils::SweepCapsuleSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start,
	const FVector& End, ECollisionChannel TraceChannel, float CapsuleRadius, float CapsuleHalfHeight, const FQuat& Rot,
	const FCollisionQueryParams& Params /*= FCollisionQueryParams::DefaultQueryParam*/,
	const FCollisionResponseParams& ResponseParam /*= FCollisionResponseParams::DefaultResponseParam*/,
	bool bDrawDebug /*= false*/, float DrawTime /*= -1.0f*/, FColor TraceColor /*= FColor::Black*/,
	FColor HitColor /*= FColor::Red*/)
{
	bool bResult = false;
	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult = World->SweepSingleByChannel(OutHit, Start, End, Rot, TraceChannel, CollisionShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{

		DrawDebugCapsule(World, Start, CapsuleHalfHeight, CapsuleRadius,
			FQuat::Identity, TraceColor, false, DrawTime);
		DrawDebugCapsule(World, End, CapsuleHalfHeight, CapsuleRadius,
			FQuat::Identity, TraceColor, false, DrawTime);
		DrawDebugLine(World, Start, End, TraceColor, false, DrawTime);
		if (bResult)
		{
			DrawDebugCapsule(World, OutHit.Location, CapsuleHalfHeight, CapsuleRadius,
				FQuat::Identity, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.0f, HitColor, false, DrawTime);
		}
	}
#endif
	return bResult;
}

bool GCTraceUtils::SweepBoxSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start, const FVector& End,
	ECollisionChannel TraceChannel, float BoxLength, float BoxWidth, float BoxHeight, const FQuat& Rot,
	const FCollisionQueryParams& Params /*= FCollisionQueryParams::DefaultQueryParam*/,
	const FCollisionResponseParams& ResponseParam /*= FCollisionResponseParams::DefaultResponseParam*/, bool bDrawDebug /*= false*/, 
	float DrawTime /*= -1.0f*/, FColor TraceColor /*= FColor::Black*/, FColor HitColor /*= FColor::Red*/)
{
	bool bResult = false;
	FVector BoxExtent = FVector(BoxLength, BoxWidth, BoxHeight) / 2;
	FCollisionShape CollisionShape = FCollisionShape::MakeBox(BoxExtent);
	bResult = World->SweepSingleByChannel(OutHit, Start, End, Rot, TraceChannel, CollisionShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{

		DrawDebugBox(World, Start, BoxExtent,
			FQuat::Identity, TraceColor, false, DrawTime);
		DrawDebugBox(World, End, BoxExtent,
			FQuat::Identity, TraceColor, false, DrawTime);
		DrawDebugLine(World, Start, End, TraceColor, false, DrawTime);
		if (bResult)
		{
			DrawDebugBox(World, OutHit.Location, BoxExtent,
				FQuat::Identity, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 10.0f, HitColor, false, DrawTime);
		}
	}
#endif
	return bResult;
}

bool GCTraceUtils::SweepSphereSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start,
	const FVector& End, ECollisionChannel TraceChannel, float SphereRadius,
	const FCollisionQueryParams& Params /*= FCollisionQueryParams::DefaultQueryParam*/,
	const FCollisionResponseParams& ResponseParam /*= FCollisionResponseParams::DefaultResponseParam*/,
	bool bDrawDebug /*= false*/, float DrawTime /*= -1.0f*/, FColor TraceColor /*= FColor::Black*/,
	FColor HitColor /*= FColor::Red*/)
{
	bool bResult = false;
	FCollisionShape CollisionShape = FCollisionShape::MakeSphere(SphereRadius);
	bResult = World->SweepSingleByChannel(OutHit, Start, End, FQuat::Identity, TraceChannel, CollisionShape, Params, ResponseParam);

#if ENABLE_DRAW_DEBUG
	if (bDrawDebug)
	{
		FVector DebugCenter = (Start + End) * 0.5f;
		FVector TraceVector = End - Start;
		float DebugCapsuleHalfHeight = TraceVector.Size() * 0.5f;

		FQuat DebugCapsuleRotation = FRotationMatrix::MakeFromZ(TraceVector).ToQuat();

		DrawDebugCapsule(World, DebugCenter, DebugCapsuleHalfHeight, SphereRadius, DebugCapsuleRotation,
			TraceColor, false, DrawTime);

		if (bDrawDebug)
		{
			DrawDebugSphere(World, OutHit.Location, SphereRadius, 32, HitColor, false, DrawTime);
			DrawDebugPoint(World, OutHit.ImpactPoint, 15.0f, HitColor, false, DrawTime);
		}
	}
#endif
	return bResult;
}

bool GCTraceUtils::OverlapCapsuleAnyByProfile(const UWorld* World, const FVector& Pos, float CapsuleRadius, float CapsuleHalfHeight, FQuat Rotation, FName ProfileName, const FCollisionQueryParams QueryParams, bool bDrawDebug /*= false*/, float DrawTime /*= -1.0f*/, FColor HitColor /*= FColor::Magenta*/)
{
	bool bResult = false;

	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult = World->OverlapAnyTestByProfile(Pos, Rotation, ProfileName, CollisionShape, QueryParams);
#if ENABLE_DRAW_DEBUG
	if (bDrawDebug && bResult)
	{
		DrawDebugCapsule(World, Pos, CapsuleHalfHeight, CapsuleRadius, Rotation, HitColor, false, DrawTime);
	}
#endif
	return bResult;
}

bool GCTraceUtils::OverlapCapsuleBlockingByProfile(const UWorld* World, const FVector& Pos, float CapsuleRadius, float CapsuleHalfHeight, FQuat Rotation, FName ProfileName, const FCollisionQueryParams QueryParams, bool bDrawDebug /*= false*/, float DrawTime /*= -1.0f*/, FColor HitColor /*= FColor::Magenta*/)
{
	bool bResult = false;

	FCollisionShape CollisionShape = FCollisionShape::MakeCapsule(CapsuleRadius, CapsuleHalfHeight);
	bResult = World->OverlapBlockingTestByProfile(Pos, Rotation, ProfileName, CollisionShape, QueryParams);
#if ENABLE_DRAW_DEBUG
	if (bDrawDebug && bResult)
	{
		DrawDebugCapsule(World, Pos, CapsuleHalfHeight, CapsuleRadius, Rotation, HitColor, false, DrawTime);
	}
#endif
	return bResult;
}

