// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/WA_GameState.h"

void AWA_GameState::SetRound(const FWordRound& NewRound)
{
	CurrentRound = NewRound;
	OnRoundUpdated.Broadcast(CurrentRound);
}

void AWA_GameState::SetScore(int32 NewScore)
{
	Score = NewScore;
	OnScoreChanged.Broadcast(Score);
}

void AWA_GameState::SetTimeLeft(float NewTimeLeft)
{
	TimeLeft = NewTimeLeft;
	OnTimerUpdated.Broadcast(TimeLeft);
}

void AWA_GameState::SetGameActive(bool bNewIsGameActive)
{
	bIsGameActive = bNewIsGameActive;
	OnScreenChanged.Broadcast(CurrentScreen, bIsPaused, bIsGameActive);
}

void AWA_GameState::SetPaused(bool bNewIsPaused)
{
	bIsPaused = bNewIsPaused;
	OnScreenChanged.Broadcast(CurrentScreen, bIsPaused, bIsGameActive);
}

void AWA_GameState::SetScreen(EWA_GameScreen NewScreen)
{
	CurrentScreen = NewScreen;
	OnScreenChanged.Broadcast(CurrentScreen, bIsPaused, bIsGameActive);
}

void AWA_GameState::SetBestScore(int32 NewBestScore)
{
	BestScore = NewBestScore;
}

void AWA_GameState::SetLives(int32 NewLives)
{
	Lives = FMath::Max(0, NewLives);
	OnLivesChanged.Broadcast(Lives);
}

void AWA_GameState::SetCombo(int32 NewCombo, float NewProgress)
{
	Combo = FMath::Max(0, NewCombo);
	ComboProgress = FMath::Clamp(NewProgress, 0.0f, 1.0f);
	OnComboChanged.Broadcast(Combo, ComboProgress);
}

void AWA_GameState::SetRoundTimeLimit(float NewRoundTimeLimit)
{
	RoundTimeLimit = FMath::Max(0.1f, NewRoundTimeLimit);
}

void AWA_GameState::BroadcastGameOver(const FString& Reason, bool bNewBestScore)
{
	GameOverReason = Reason;
	OnGameOver.Broadcast(GameOverReason, Score, BestScore, bNewBestScore);
}
