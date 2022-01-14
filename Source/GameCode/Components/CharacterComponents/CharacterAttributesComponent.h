// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "CharacterAttributesComponent.generated.h"

DECLARE_MULTICAST_DELEGATE(FOnDeathEventSignature);
DECLARE_MULTICAST_DELEGATE_OneParam(FOutOfStaminaEventSignature, bool);

UCLASS( ClassGroup=(Custom), meta=(BlueprintSpawnableComponent) )
class GAMECODE_API UCharacterAttributesComponent : public UActorComponent
{
	GENERATED_BODY()

public:	
	UCharacterAttributesComponent();
	virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

	FOnDeathEventSignature OnDeathEvent;
	FOutOfStaminaEventSignature OnOutOfStaminaEvent;

	bool IsAlive() { return Health > 0.0f; }

	float GetHealthPercent() const;
protected:
	virtual void BeginPlay() override;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Health", meta = (UIMin = 0.0f))
	float MaxHealth = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float MaxStamina = 100.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float StaminaRestoreVelocity = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Stamina", meta = (UIMin = 0.0f))
	float SprintStaminaConsumptionVelocity = 30.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float MaxOxygen = 50.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float OxygenRestoreVelocity = 15.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float SwimOxygenConsumptionVelocity = 10.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float OutOfOxygenSecondsInTick = 2.0f;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Oxygen", meta = (UIMin = 0.0f))
	float OutOfOxygenDamage = 5.0f;

private:
	float Health = 0.0f;
	float Stamina = 0.0f;
	void UpdateStaminaValue(float DeltaTime);
	bool StaminaMaxOrZero;
	float Oxygen = 0.0f;

	FTimerHandle OutOfOxygenDamageTimer;
	void UpdateOxygenValue(float DeltaTime);
	void OutOfOxygenTakeDamage();
#if UE_BUILD_DEBUG || UE_BUILD_DEVELOPMENT
	void DebugDrawAttributes();
#endif

	UFUNCTION()
	void OnTakeAnyDamage(AActor* DamagedActor, float Damage, const class UDamageType* DamageType, class AController* InstigatedBy, AActor* DamageCauser);

	TWeakObjectPtr<class AGCBaseCharacter> CachedBaseCharacterOwner;
};
