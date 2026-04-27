// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/WordTypes.h"
#include "WA_GameState.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWARoundUpdatedSignature, const FWordRound&, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWAScoreChangedSignature, int32, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWAGameTimerUpdatedSignature, float, TimeLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWAGameOverSignature, const FString&, Reason, int32, FinalScore, int32, BestScore);

UCLASS()
class WORD_API AWA_GameState : public AGameStateBase
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Events")
	FWARoundUpdatedSignature OnRoundUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Events")
	FWAScoreChangedSignature OnScoreChanged;

	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Events")
	FWAGameTimerUpdatedSignature OnTimerUpdated;

	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Events")
	FWAGameOverSignature OnGameOver;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	float TimeLeft = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	bool bIsGameActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	FWordRound CurrentRound;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	FString GameOverReason;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	int32 BestScore = 0;

	void SetRound(const FWordRound& NewRound);
	void SetScore(int32 NewScore);
	void SetTimeLeft(float NewTimeLeft);
	void SetGameActive(bool bNewIsGameActive);
	void SetBestScore(int32 NewBestScore);
	void BroadcastGameOver(const FString& Reason);
};
