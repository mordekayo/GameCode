// Fill out your copyright notice in the Description page of Project Settings.


#include "PlatformInvocator.h"


void APlatformInvocator::Invoke()
{
	OnInvoctorActivated.Broadcast();
}

