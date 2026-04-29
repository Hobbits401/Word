// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/WA_GameMode.h"

#include "Core/WordManager.h"
#include "Gameplay/WA_GameState.h"
#include "Gameplay/WA_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Save/WA_SaveGame.h"
#include "Systems/DifficultySystem.h"
#include "Systems/TimerComponent.h"

AWA_GameMode::AWA_GameMode()
{
	GameStateClass = AWA_GameState::StaticClass();
	PlayerControllerClass = AWA_PlayerController::StaticClass();
	TimerComponent = CreateDefaultSubobject<UTimerComponent>(TEXT("TimerComponent"));
}

void AWA_GameMode::BeginPlay()
{
	Super::BeginPlay();

	WordManager = NewObject<UWordManager>(this);
	WordManager->Initialize();

	DifficultySystem = NewObject<UDifficultySystem>(this);

	TimerComponent->OnTimerUpdated.AddDynamic(this, &AWA_GameMode::HandleTimerUpdated);
	TimerComponent->OnTimerExpired.AddDynamic(this, &AWA_GameMode::HandleTimerExpired);

	LoadBestScore();
	if (AWA_GameState* WAGameState = GetWAGameState())
	{
		WAGameState->SetBestScore(CurrentSaveGame ? CurrentSaveGame->BestScore : 0);
		WAGameState->SetScore(0);
		WAGameState->SetLives(MaxLives);
		WAGameState->SetCombo(0, 0.0f);
		WAGameState->SetRoundTimeLimit(StartTime);
		WAGameState->SetGameActive(false);
		WAGameState->SetPaused(false);
		WAGameState->SetScreen(EWA_GameScreen::MainMenu);
	}
}

void AWA_GameMode::StartGame()
{
	LoadBestScore();

	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WordManager || !DifficultySystem)
	{
		return;
	}

	WordManager->StartGame();
	DifficultySystem->UpdateDifficultyByScore(0);
	TimerComponent->SetDrainMultiplier(DifficultySystem->GetTimerDrainMultiplier());

	WAGameState->SetBestScore(CurrentSaveGame ? CurrentSaveGame->BestScore : 0);
	WAGameState->SetScore(0);
	WAGameState->SetLives(MaxLives);
	WAGameState->SetCombo(0, 0.0f);
	WAGameState->SetRoundTimeLimit(StartTime);
	WAGameState->SetPaused(false);
	WAGameState->SetGameActive(true);
	WAGameState->SetScreen(EWA_GameScreen::Playing);

	StartRound();
}

void AWA_GameMode::StartRound()
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WAGameState->bIsGameActive || WAGameState->bIsPaused || !WordManager)
	{
		return;
	}

	FWordRound NewRound;
	if (WordManager->GenerateRound(NewRound))
	{
		WAGameState->SetRound(NewRound);
		ResetRoundTimer();
	}
	else
	{
		EndGame(TEXT("Round generation failed"));
	}
}

void AWA_GameMode::HandleAnswer(const FString& SelectedWord)
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WAGameState->bIsGameActive || WAGameState->bIsPaused || !DifficultySystem)
	{
		return;
	}

	if (SelectedWord == WAGameState->CurrentRound.CorrectWord)
	{
		const int32 NewScore = WAGameState->Score + ScorePerCorrect;
		const int32 NewCombo = WAGameState->Combo + 1;
		WAGameState->SetScore(NewScore);
		WAGameState->SetCombo(NewCombo, CalculateComboProgress(NewCombo));
		DifficultySystem->UpdateDifficultyByScore(NewScore);
		TimerComponent->SetDrainMultiplier(DifficultySystem->GetTimerDrainMultiplier());
		StartRound();
		return;
	}

	LoseLife(TEXT("Wrong answer"));
}

void AWA_GameMode::EndGame(const FString& Reason)
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WAGameState->bIsGameActive)
	{
		return;
	}

	TimerComponent->StopTimer();
	WAGameState->SetGameActive(false);
	WAGameState->SetPaused(false);
	WAGameState->SetScreen(EWA_GameScreen::GameOver);

	bool bNewBestScore = false;
	if (CurrentSaveGame && WAGameState->Score > CurrentSaveGame->BestScore)
	{
		CurrentSaveGame->BestScore = WAGameState->Score;
		WAGameState->SetBestScore(CurrentSaveGame->BestScore);
		SaveBestScore();
		bNewBestScore = true;
	}

	WAGameState->BroadcastGameOver(Reason, bNewBestScore);
}

void AWA_GameMode::RestartGame()
{
	StartGame();
}

void AWA_GameMode::PauseGame()
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WAGameState->bIsGameActive || WAGameState->bIsPaused)
	{
		return;
	}

	TimerComponent->StopTimer();
	WAGameState->SetPaused(true);
	WAGameState->SetScreen(EWA_GameScreen::Paused);
}

void AWA_GameMode::ResumeGame()
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WAGameState->bIsGameActive || !WAGameState->bIsPaused)
	{
		return;
	}

	WAGameState->SetPaused(false);
	WAGameState->SetScreen(EWA_GameScreen::Playing);
	TimerComponent->ResumeTimer();
}

void AWA_GameMode::ReturnToMainMenu()
{
	TimerComponent->StopTimer();

	if (AWA_GameState* WAGameState = GetWAGameState())
	{
		WAGameState->SetGameActive(false);
		WAGameState->SetPaused(false);
		WAGameState->SetScreen(EWA_GameScreen::MainMenu);
	}
}

void AWA_GameMode::HandleTimerUpdated(float TimeLeft)
{
	if (AWA_GameState* WAGameState = GetWAGameState())
	{
		WAGameState->SetTimeLeft(TimeLeft);
	}
}

void AWA_GameMode::HandleTimerExpired()
{
	LoseLife(TEXT("Time is over"));
}

AWA_GameState* AWA_GameMode::GetWAGameState() const
{
	return GetGameState<AWA_GameState>();
}

void AWA_GameMode::LoadBestScore()
{
	if (UGameplayStatics::DoesSaveGameExist(SaveSlotName, SaveUserIndex))
	{
		CurrentSaveGame = Cast<UWA_SaveGame>(UGameplayStatics::LoadGameFromSlot(SaveSlotName, SaveUserIndex));
	}

	if (!CurrentSaveGame)
	{
		CurrentSaveGame = Cast<UWA_SaveGame>(UGameplayStatics::CreateSaveGameObject(UWA_SaveGame::StaticClass()));
	}
}

void AWA_GameMode::SaveBestScore()
{
	if (CurrentSaveGame)
	{
		UGameplayStatics::SaveGameToSlot(CurrentSaveGame, SaveSlotName, SaveUserIndex);
	}
}

void AWA_GameMode::LoseLife(const FString& Reason)
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WAGameState->bIsGameActive)
	{
		return;
	}

	const int32 NewLives = WAGameState->Lives - 1;
	WAGameState->SetLives(NewLives);
	WAGameState->SetCombo(0, 0.0f);

	if (NewLives <= 0)
	{
		EndGame(Reason);
		return;
	}

	StartRound();
}

void AWA_GameMode::ResetRoundTimer()
{
	if (AWA_GameState* WAGameState = GetWAGameState())
	{
		WAGameState->SetRoundTimeLimit(StartTime);
	}
	TimerComponent->StartTimer(StartTime);
}

float AWA_GameMode::CalculateComboProgress(int32 Combo) const
{
	if (ComboProgressTarget <= 0)
	{
		return 0.0f;
	}

	return static_cast<float>(Combo % ComboProgressTarget) / static_cast<float>(ComboProgressTarget);
}
