// Copyright Epic Games, Inc. All Rights Reserved.

#include "Systems/DifficultySystem.h"

void UDifficultySystem::UpdateDifficultyByScore(int32 Score)
{
	DifficultyProgress = FMath::Clamp(static_cast<float>(Score) / 50.0f, 0.0f, 1.0f);
	TimerDrainMultiplier = 1.0f + DifficultyProgress * 0.6f;
	CorrectAnswerBonusTime = 0.5f - DifficultyProgress * 0.25f;
}
