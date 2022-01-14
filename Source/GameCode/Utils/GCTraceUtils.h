#pragma once

namespace GCTraceUtils
{
	bool SweepCapsuleSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start,
		const FVector& End, ECollisionChannel TraceChannel, float CapsuleRadius, float CapsuleHalfHeight, const FQuat& Rot,
		const FCollisionQueryParams& Params = FCollisionQueryParams::DefaultQueryParam,
		const FCollisionResponseParams& ResponseParam = FCollisionResponseParams::DefaultResponseParam, bool bDrawDebug = false,
		float DrawTime = -1.0f, FColor TraceColor = FColor::Black, FColor HitColor = FColor::Red);

	bool SweepBoxSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start,
		const FVector& End, ECollisionChannel TraceChannel, float BoxLength, float BoxWidth, float BoxHeight, const FQuat& Rot,
		const FCollisionQueryParams& Params = FCollisionQueryParams::DefaultQueryParam,
		const FCollisionResponseParams& ResponseParam = FCollisionResponseParams::DefaultResponseParam, bool bDrawDebug = false,
		float DrawTime = -1.0f, FColor TraceColor = FColor::Black, FColor HitColor = FColor::Red);

	bool SweepSphereSingleByChannel(const UWorld* World, struct FHitResult& OutHit, const FVector& Start,
		const FVector& End, ECollisionChannel TraceChannel, float SphereRadius,
		const FCollisionQueryParams& Params = FCollisionQueryParams::DefaultQueryParam,
		const FCollisionResponseParams& ResponseParam = FCollisionResponseParams::DefaultResponseParam, bool bDrawDebug = false,
		float DrawTime = -1.0f, FColor TraceColor = FColor::Black, FColor HitColor = FColor::Red);

	bool OverlapCapsuleAnyByProfile(const UWorld* World, const FVector& Pos, float CapsuleRadius, float CapsuleHalfHeight,
		FQuat Rotation, FName ProfileName, const FCollisionQueryParams QueryParams, bool bDrawDebug = false, float DrawTime = -1.0f,
		FColor HitColor = FColor::Magenta);

	bool OverlapCapsuleBlockingByProfile(const UWorld* World, const FVector& Pos, float CapsuleRadius, float CapsuleHalfHeight,
		FQuat Rotation, FName ProfileName, const FCollisionQueryParams QueryParams, bool bDrawDebug = false, float DrawTime = -1.0f,
		FColor HitColor = FColor::Magenta);
}