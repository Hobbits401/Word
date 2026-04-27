// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "DifficultySystem.generated.h"

UCLASS()
class WORD_API UDifficultySystem : public UObject
{
	GENERATED_BODY()

public:
	void UpdateDifficultyByScore(int32 Score);

	float GetTimerDrainMultiplier() const { return TimerDrainMultiplier; }
	float GetCorrectAnswerBonusTime() const { return CorrectAnswerBonusTime; }

private:
	float DifficultyProgress = 0.0f;
	float TimerDrainMultiplier = 1.0f;
	float CorrectAnswerBonusTime = 0.5f;
};
