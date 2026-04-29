// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameStateBase.h"
#include "Core/WordTypes.h"
#include "WA_GameState.generated.h"

UENUM(BlueprintType)
enum class EWA_GameScreen : uint8
{
	MainMenu,
	Playing,
	Paused,
	GameOver
};

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWARoundUpdatedSignature, const FWordRound&, Round);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWAScoreChangedSignature, int32, Score);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWAGameTimerUpdatedSignature, float, TimeLeft);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FWALivesChangedSignature, int32, Lives);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FWAComboChangedSignature, int32, Combo, float, Progress);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FWAScreenChangedSignature, EWA_GameScreen, Screen, bool, bIsPaused, bool, bIsGameActive);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FWAGameOverSignature, const FString&, Reason, int32, FinalScore, int32, BestScore, bool, bNewBestScore);

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
	FWALivesChangedSignature OnLivesChanged;

	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Events")
	FWAComboChangedSignature OnComboChanged;

	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Events")
	FWAScreenChangedSignature OnScreenChanged;

	UPROPERTY(BlueprintAssignable, Category = "Word Attack|Events")
	FWAGameOverSignature OnGameOver;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	int32 Score = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	float TimeLeft = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	bool bIsGameActive = false;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	bool bIsPaused = false;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	EWA_GameScreen CurrentScreen = EWA_GameScreen::MainMenu;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	FWordRound CurrentRound;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	FString GameOverReason;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	int32 BestScore = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	int32 Lives = 3;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	int32 Combo = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	float ComboProgress = 0.0f;

	UPROPERTY(BlueprintReadOnly, Category = "Word Attack|State")
	float RoundTimeLimit = 20.0f;

	void SetRound(const FWordRound& NewRound);
	void SetScore(int32 NewScore);
	void SetTimeLeft(float NewTimeLeft);
	void SetGameActive(bool bNewIsGameActive);
	void SetPaused(bool bNewIsPaused);
	void SetScreen(EWA_GameScreen NewScreen);
	void SetBestScore(int32 NewBestScore);
	void SetLives(int32 NewLives);
	void SetCombo(int32 NewCombo, float NewProgress);
	void SetRoundTimeLimit(float NewRoundTimeLimit);
	void BroadcastGameOver(const FString& Reason, bool bNewBestScore);
};
