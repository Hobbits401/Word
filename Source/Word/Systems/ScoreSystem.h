// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "ScoreSystem.generated.h"

UCLASS()
class WORD_API UScoreSystem : public UObject
{
	GENERATED_BODY()

public:
	void AddScore(int32 Amount = 1) { Score += Amount; }
	void ResetScore() { Score = 0; }
	int32 GetScore() const { return Score; }

private:
	int32 Score = 0;
};
