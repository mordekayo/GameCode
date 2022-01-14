// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "GameCodeTypes.h"
#include "EquipableItem.generated.h"

UCLASS()
class GAMECODE_API AEquipableItem : public AActor
{
	GENERATED_BODY()
	
public:
	EEquipableItemType GetItemType() const;


protected:
	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly, Category = "Equipable item")
	EEquipableItemType ItemType = EEquipableItemType::None;
public:	


};
