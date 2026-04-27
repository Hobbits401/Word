// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/WA_PlayerController.h"

#include "Blueprint/UserWidget.h"
#include "Gameplay/WA_GameMode.h"
#include "Gameplay/WA_GameState.h"
#include "UI/Widgets/W_GameHUD.h"
#include "UI/Widgets/W_GameOver.h"

void AWA_PlayerController::BeginPlay()
{
	Super::BeginPlay();

	bShowMouseCursor = true;
	SetInputMode(FInputModeUIOnly());

	CreateWidgets();
	BindGameStateEvents();
	RefreshWidgetsFromState();
}

void AWA_PlayerController::HandleWordSelected(const FString& SelectedWord)
{
	if (AWA_GameMode* WAGameMode = GetWAGameMode())
	{
		WAGameMode->HandleAnswer(SelectedWord);
	}
}

void AWA_PlayerController::HandleRestartRequested()
{
	if (AWA_GameMode* WAGameMode = GetWAGameMode())
	{
		WAGameMode->RestartGame();
	}
}

void AWA_PlayerController::HandleRoundUpdated(const FWordRound& Round)
{
	if (GameHUD)
	{
		GameHUD->UpdateRound(Round);
	}
}

void AWA_PlayerController::HandleScoreChanged(int32 Score)
{
	if (GameHUD)
	{
		GameHUD->UpdateScore(Score);
	}
}

void AWA_PlayerController::HandleTimerUpdated(float TimeLeft)
{
	if (GameHUD)
	{
		GameHUD->UpdateTimer(TimeLeft);
	}
}

void AWA_PlayerController::HandleGameOver(const FString& Reason, int32 FinalScore, int32 BestScore)
{
	if (GameHUD)
	{
		GameHUD->DisableButtons();
	}

	if (GameOverWidget)
	{
		GameOverWidget->ShowGameOver(FinalScore, BestScore, Reason);
	}
}

void AWA_PlayerController::CreateWidgets()
{
	if (GameHUDClass)
	{
		GameHUD = CreateWidget<UW_GameHUD>(this, GameHUDClass);
		if (GameHUD)
		{
			GameHUD->AddToViewport(0);
			GameHUD->OnWordSelected.RemoveAll(this);
			GameHUD->OnWordSelected.AddDynamic(this, &AWA_PlayerController::HandleWordSelected);
		}
	}

	if (GameOverClass)
	{
		GameOverWidget = CreateWidget<UW_GameOver>(this, GameOverClass);
		if (GameOverWidget)
		{
			GameOverWidget->AddToViewport(10);
			GameOverWidget->OnRestartRequested.RemoveAll(this);
			GameOverWidget->OnRestartRequested.AddDynamic(this, &AWA_PlayerController::HandleRestartRequested);
		}
	}
}

void AWA_PlayerController::BindGameStateEvents()
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState)
	{
		return;
	}

	WAGameState->OnRoundUpdated.RemoveAll(this);
	WAGameState->OnRoundUpdated.AddDynamic(this, &AWA_PlayerController::HandleRoundUpdated);
	WAGameState->OnScoreChanged.RemoveAll(this);
	WAGameState->OnScoreChanged.AddDynamic(this, &AWA_PlayerController::HandleScoreChanged);
	WAGameState->OnTimerUpdated.RemoveAll(this);
	WAGameState->OnTimerUpdated.AddDynamic(this, &AWA_PlayerController::HandleTimerUpdated);
	WAGameState->OnGameOver.RemoveAll(this);
	WAGameState->OnGameOver.AddDynamic(this, &AWA_PlayerController::HandleGameOver);
}

void AWA_PlayerController::RefreshWidgetsFromState()
{
	const AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState)
	{
		return;
	}

	if (GameHUD)
	{
		GameHUD->UpdateScore(WAGameState->Score);
		GameHUD->UpdateTimer(WAGameState->TimeLeft);
		if (!WAGameState->CurrentRound.Options.IsEmpty())
		{
			GameHUD->UpdateRound(WAGameState->CurrentRound);
		}
	}
}

AWA_GameMode* AWA_PlayerController::GetWAGameMode() const
{
	return GetWorld() ? GetWorld()->GetAuthGameMode<AWA_GameMode>() : nullptr;
}

AWA_GameState* AWA_PlayerController::GetWAGameState() const
{
	return GetWorld() ? GetWorld()->GetGameState<AWA_GameState>() : nullptr;
}
