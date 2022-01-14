// Fill out your copyright notice in the Description page of Project Settings.


#include "PlayerHUDWidget.h"
#include "Characters/GCBaseCharacter.h"
#include "Components/CharacterComponents/CharacterAttributesComponent.h"

float UPlayerHUDWidget::GetHealthPercent() const
{
	float Result = 1.0f;
	APawn* Pawn = GetOwningPlayerPawn();
	AGCBaseCharacter* BaseCharacter = Cast<AGCBaseCharacter>(Pawn);
	if (IsValid(BaseCharacter))
	{
		const UCharacterAttributesComponent* CharacterAttributes = BaseCharacter->GetCharacterAttributesComponent();
		Result = CharacterAttributes->GetHealthPercent();
	}

	return Result;
}
