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
}

void AWA_GameState::SetBestScore(int32 NewBestScore)
{
	BestScore = NewBestScore;
}

void AWA_GameState::BroadcastGameOver(const FString& Reason)
{
	GameOverReason = Reason;
	OnGameOver.Broadcast(GameOverReason, Score, BestScore);
}
