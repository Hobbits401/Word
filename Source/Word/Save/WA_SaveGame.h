// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/SaveGame.h"
#include "WA_SaveGame.generated.h"

UCLASS()
class WORD_API UWA_SaveGame : public USaveGame
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, Category = "Word Attack|Save")
	int32 BestScore = 0;
};
