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

	StartGame();
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
	WAGameState->SetGameActive(true);
	TimerComponent->StartTimer(StartTime);

	StartRound();
}

void AWA_GameMode::StartRound()
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WAGameState->bIsGameActive || !WordManager)
	{
		return;
	}

	FWordRound NewRound;
	if (WordManager->GenerateRound(NewRound))
	{
		WAGameState->SetRound(NewRound);
	}
	else
	{
		EndGame(TEXT("Не удалось создать раунд"));
	}
}

void AWA_GameMode::HandleAnswer(const FString& SelectedWord)
{
	AWA_GameState* WAGameState = GetWAGameState();
	if (!WAGameState || !WAGameState->bIsGameActive || !DifficultySystem)
	{
		return;
	}

	if (SelectedWord == WAGameState->CurrentRound.CorrectWord)
	{
		const int32 NewScore = WAGameState->Score + ScorePerCorrect;
		WAGameState->SetScore(NewScore);
		DifficultySystem->UpdateDifficultyByScore(NewScore);
		TimerComponent->SetDrainMultiplier(DifficultySystem->GetTimerDrainMultiplier());
		TimerComponent->AddTime(DifficultySystem->GetCorrectAnswerBonusTime());
		StartRound();
		return;
	}

	EndGame(TEXT("Неверный ответ"));
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

	if (CurrentSaveGame && WAGameState->Score > CurrentSaveGame->BestScore)
	{
		CurrentSaveGame->BestScore = WAGameState->Score;
		WAGameState->SetBestScore(CurrentSaveGame->BestScore);
		SaveBestScore();
	}

	WAGameState->BroadcastGameOver(Reason);
}

void AWA_GameMode::RestartGame()
{
	StartGame();
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
	EndGame(TEXT("Время вышло"));
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
