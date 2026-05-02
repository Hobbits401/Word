// Copyright Epic Games, Inc. All Rights Reserved.

#include "Gameplay/WA_GameMode.h"

#include "Core/WordManager.h"
#include "Gameplay/WA_GameState.h"
#include "Gameplay/WA_PlayerController.h"
#include "Kismet/GameplayStatics.h"
#include "Save/WA_SaveGame.h"
#include "Sound/SoundBase.h"
#include "Systems/DifficultySystem.h"
#include "Systems/TimerComponent.h"

AWA_GameMode::AWA_GameMode()
{
	GameStateClass = AWA_GameState::StaticClass();
	PlayerControllerClass = AWA_PlayerController::StaticClass();
	TimerComponent = CreateDefaultSubobject<UTimerComponent>(TEXT("TimerComponent"));
	LoadTeacherSounds();
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
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DelayedGameOverTimerHandle);
	}
	bIsGameOverPending = false;
	PendingGameOverReason.Reset();

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
	if (!WAGameState || !WAGameState->bIsGameActive || WAGameState->bIsPaused || bIsGameOverPending || !WordManager)
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
	if (!WAGameState || !WAGameState->bIsGameActive || WAGameState->bIsPaused || bIsGameOverPending || !DifficultySystem)
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
		if (NewCombo > 0 && NewCombo % 2 == 0)
		{
			PlayRandomSound(GoodWordSounds);
		}
		StartRound();
		return;
	}

	LoseLife(TEXT("Wrong answer"));
}

void AWA_GameMode::EndGame(const FString& Reason)
{
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DelayedGameOverTimerHandle);
	}
	bIsGameOverPending = false;

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

	if (bNewBestScore)
	{
		PlaySound(NewRecordSound);
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
	if (GetWorld())
	{
		GetWorld()->GetTimerManager().ClearTimer(DelayedGameOverTimerHandle);
	}
	bIsGameOverPending = false;
	PendingGameOverReason.Reset();

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
	PlaySound(TimerIsOverSound);
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
		TimerComponent->StopTimer();
		PlaySound(GameOverSound);
		bIsGameOverPending = true;
		PendingGameOverReason = Reason;
		if (GetWorld())
		{
			GetWorld()->GetTimerManager().ClearTimer(DelayedGameOverTimerHandle);
			GetWorld()->GetTimerManager().SetTimer(
				DelayedGameOverTimerHandle,
				this,
				&AWA_GameMode::FinishDelayedGameOver,
				GameOverDelay,
				false);
		}
		else
		{
			FinishDelayedGameOver();
		}
		return;
	}

	PlayRandomSound(LifeLostSounds);
	StartRound();
}

void AWA_GameMode::FinishDelayedGameOver()
{
	EndGame(PendingGameOverReason.IsEmpty() ? TEXT("Game over") : PendingGameOverReason);
}

void AWA_GameMode::LoadTeacherSounds()
{
	GoodWordSounds.Reset();
	const TCHAR* GoodWordSoundPaths[] = {
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_1_YouAreCool.T_1_YouAreCool"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_2_StopIt_1.T_2_StopIt_1"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_3_StopIt_2.T_3_StopIt_2"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_4_Good.T_4_Good"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_5_Continue.T_5_Continue"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_6_More.T_6_More"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_7_MoreMore.T_7_MoreMore"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_8_MoreMoreMore.T_8_MoreMoreMore"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_9_YouAreMyBestStudent.T_9_YouAreMyBestStudent"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_10_Perfect.T_10_Perfect"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_11_MyBestStudent.T_11_MyBestStudent"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_12_Oya.T_12_Oya"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_13_Eeee.T_13_Eeee"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_14_Aa.T_14_Aa"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_15_A.T_15_A"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_16_A.T_16_A"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_17_Why.T_17_Why"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_18_Why.T_18_Why"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_19_Why.T_19_Why"),
		TEXT("/Game/Music/Sounds/Teacher/GoodWords/T_20_EE.T_20_EE")
	};

	for (const TCHAR* SoundPath : GoodWordSoundPaths)
	{
		if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, SoundPath))
		{
			GoodWordSounds.Add(Sound);
		}
	}

	LifeLostSounds.Reset();
	if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/Sounds/Teacher/T_GoHome.T_GoHome")))
	{
		LifeLostSounds.Add(Sound);
	}
	if (USoundBase* Sound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/Sounds/Teacher/T_GoAway.T_GoAway")))
	{
		LifeLostSounds.Add(Sound);
	}

	GameOverSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/Sounds/GameOver.GameOver"));
	NewRecordSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/Sounds/New_Record.New_Record"));
	TimerIsOverSound = LoadObject<USoundBase>(nullptr, TEXT("/Game/Music/Sounds/TimerIsOver.TimerIsOver"));
}

void AWA_GameMode::PlayRandomSound(const TArray<TObjectPtr<USoundBase>>& Sounds) const
{
	if (Sounds.IsEmpty())
	{
		return;
	}

	PlaySound(Sounds[FMath::RandRange(0, Sounds.Num() - 1)]);
}

void AWA_GameMode::PlaySound(USoundBase* Sound) const
{
	if (Sound)
	{
		UGameplayStatics::PlaySound2D(this, Sound);
	}
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
