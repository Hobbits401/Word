// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "WordTypes.generated.h"

USTRUCT(BlueprintType)
struct FWordRound
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack")
	FString CorrectWord;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack")
	TArray<FString> Options;

	TArray<TCHAR> Letters;
};
