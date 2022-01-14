// Fill out your copyright notice in the Description page of Project Settings.


#include "CharacterAttributesComponent.h"
#include "../../Characters/GCBaseCharacter.h"
#include "Kismet/GameplayStatics.h"
#include "../../Subsystems/DebugSubsystem.h"
#include "Components/CapsuleComponent.h" 
#include "DrawDebugHelpers.h"
#include "../../GameCodeTypes.h"
#include "GameFramework/PhysicsVolume.h"

// Sets default values for this component's properties
UCharacterAttributesComponent::UCharacterAttributesComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}


// Called when the game starts
void UCharacterAttributesComponent::BeginPlay()
{
	Super::BeginPlay();

	checkf(MaxHealth > 0.0f, TEXT("UCharacterAttributesComponent::BeginPlay max health cannot be equal to 0"));
	checkf(GetOwner()->IsA<AGCBaseCharacter>(), TEXT("UCharacterAttributesComponent::BeginPlay() UCharacterAttributesComponent can be used only with AGCBaseCharacter"))
	CachedBaseCharacterOwner = StaticCast<AGCBaseCharacter*>(GetOwner());
	CachedBaseCharacterOwner->OnTakeAnyDamage.AddDynamic(this, &UCharacterAttributesComponent::OnTakeAnyDamage);

	Health = MaxHealth;
	Stamina = MaxStamina;
	Oxygen = MaxOxygen;
	
}

void UCharacterAttributesComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	DebugDrawAttributes();
#endif
	UpdateStaminaValue(DeltaTime);	
	UpdateOxygenValue(DeltaTime);
}


float UCharacterAttributesComponent::GetHealthPercent() const
{
	return Health / MaxHealth;
}

void UCharacterAttributesComponent::UpdateStaminaValue(float DeltaTime)
{
	if (CachedBaseCharacterOwner->GetBaseCharacterMovementComponent()->IsSprinting())
	{ 
		Stamina -= SprintStaminaConsumptionVelocity * DeltaTime;
		Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
		if (Stamina <= 0)
		{
			if (OnOutOfStaminaEvent.IsBound())
			{
				OnOutOfStaminaEvent.Broadcast(false);
			}
		}
	}
	else
	{
		Stamina += StaminaRestoreVelocity * DeltaTime;
		Stamina = FMath::Clamp(Stamina, 0.0f, MaxStamina);
		if (FMath::IsNearlyEqual(Stamina,MaxStamina))
		{
			if (OnOutOfStaminaEvent.IsBound())
			{
				OnOutOfStaminaEvent.Broadcast(true);
			}
		}
	}
}

void UCharacterAttributesComponent::UpdateOxygenValue(float DeltaTime)
{
	if (CachedBaseCharacterOwner->IsSwimmingUnderWater())
	{
		if (Oxygen <= 0)
		{
			if (GetWorld()->GetTimerManager().GetTimerRemaining(OutOfOxygenDamageTimer) < 0)
			{
				OutOfOxygenTakeDamage();
				GetWorld()->GetTimerManager().SetTimer(OutOfOxygenDamageTimer, this, &UCharacterAttributesComponent::OutOfOxygenTakeDamage, OutOfOxygenSecondsInTick, true);
			}
		}
		Oxygen -= SwimOxygenConsumptionVelocity * DeltaTime;
		Oxygen = FMath::Clamp(Oxygen, 0.0f, MaxOxygen);
	}
	else
	{
		GetWorld()->GetTimerManager().ClearTimer(OutOfOxygenDamageTimer);
		Oxygen += OxygenRestoreVelocity * DeltaTime;
		Oxygen = FMath::Clamp(Oxygen, 0.0f, MaxOxygen);
	}
}

void UCharacterAttributesComponent::OutOfOxygenTakeDamage()
{
	APhysicsVolume* Volume = CachedBaseCharacterOwner->GetCharacterMovement()->GetPhysicsVolume();
	CachedBaseCharacterOwner->TakeDamage(OutOfOxygenDamage, FDamageEvent(), CachedBaseCharacterOwner->GetController(), Cast<AActor>(Volume));
}

void UCharacterAttributesComponent::DebugDrawAttributes()
{
	UDebugSubsystem* DebugSubsystem = UGameplayStatics::GetGameInstance(GetWorld())->GetSubsystem<UDebugSubsystem>();
	if (!DebugSubsystem->IsCategoryEnabled(DebugCategoryCharacterAttributes))
	{
		return;
	}

	FVector HealthTextLocation = CachedBaseCharacterOwner->GetActorLocation() +
		(CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight()) * FVector::UpVector;
	DrawDebugString(GetWorld(), HealthTextLocation, FString::Printf(TEXT("Health: %.2f"), Health), nullptr, FColor::Green, 0.0f, true);	

	FVector StaminaTextLocation = CachedBaseCharacterOwner->GetActorLocation() +
		(CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 8.0f) * FVector::UpVector;
	DrawDebugString(GetWorld(), StaminaTextLocation, FString::Printf(TEXT("Stamina: %.2f"), Stamina), nullptr, FColor::Blue, 0.0f, true);

	FVector OxygenTextLocation = CachedBaseCharacterOwner->GetActorLocation() +
		(CachedBaseCharacterOwner->GetCapsuleComponent()->GetScaledCapsuleHalfHeight() + 15.0f) * FVector::UpVector;
	DrawDebugString(GetWorld(), OxygenTextLocation, FString::Printf(TEXT("Oxygen: %.2f"), Oxygen), nullptr, FColor::Cyan, 0.0f, true);
}

void UCharacterAttributesComponent::OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser)
{
	if (!IsAlive())
	{
		return;
	}

	Health = FMath::Clamp(Health - Damage, 0.0f, MaxHealth);
	//UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributesComponent::OnTakeAnyDamage %s received %.2f amount of damage from %s"), *CachedBaseCharacterOwner->GetName(), Damage, *DamageCauser->GetName());

	if (Health <= 0.0f)
	{
		//UE_LOG(LogDamage, Warning, TEXT("UCharacterAttributesComponent::OnTakeAnyDamage character %s is killed by an actor %s"), *CachedBaseCharacterOwner->GetName(), *DamageCauser->GetName());
		if (OnDeathEvent.IsBound())
		{
			OnDeathEvent.Broadcast();
		}
	}
}



