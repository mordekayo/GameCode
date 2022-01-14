// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "PlatformInvocator.generated.h"
	
DECLARE_MULTICAST_DELEGATE(FOnInvocatorActivatedSignature);

UCLASS()
class GAMECODE_API APlatformInvocator : public AActor
{
	GENERATED_BODY()
	
public:	

protected:

public:	
	FOnInvocatorActivatedSignature OnInvoctorActivated;

	UFUNCTION(BlueprintCallable)
		void Invoke();

};
