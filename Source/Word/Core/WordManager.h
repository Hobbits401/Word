// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "WordTypes.h"
#include "WordManager.generated.h"

UCLASS()
class WORD_API UWordManager : public UObject
{
	GENERATED_BODY()

public:
	void Initialize();
	void StartGame();
	bool GenerateRound(FWordRound& OutRound);

private:
	UPROPERTY()
	TArray<FString> WordList;

	UPROPERTY()
	TArray<FString> ShuffledWords;

	int32 CurrentIndex = 0;

	void BuildDefaultWordList();
	void ShuffleWords();
	FString GetNextWord();
	TArray<TCHAR> ExtractLetters(const FString& Word) const;
	bool ContainsAllLetters(const FString& Word, const TArray<TCHAR>& Letters) const;
};
