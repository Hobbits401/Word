// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "WA_GameMode.generated.h"

class AWA_GameState;
class UDifficultySystem;
class UTimerComponent;
class UWA_SaveGame;
class UWordManager;

UCLASS()
class WORD_API AWA_GameMode : public AGameModeBase
{
	GENERATED_BODY()

public:
	AWA_GameMode();

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Game")
	void StartGame();

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Game")
	void StartRound();

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Game")
	void HandleAnswer(const FString& SelectedWord);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Game")
	void EndGame(const FString& Reason);

	UFUNCTION(BlueprintCallable, Category = "Word Attack|Game")
	void RestartGame();

protected:
	virtual void BeginPlay() override;

private:
	UPROPERTY()
	TObjectPtr<UWordManager> WordManager;

	UPROPERTY()
	TObjectPtr<UDifficultySystem> DifficultySystem;

	UPROPERTY(VisibleAnywhere, Category = "Word Attack")
	TObjectPtr<UTimerComponent> TimerComponent;

	UPROPERTY()
	TObjectPtr<UWA_SaveGame> CurrentSaveGame;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|Rules")
	float StartTime = 3.0f;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|Rules")
	int32 ScorePerCorrect = 1;

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|Save")
	FString SaveSlotName = TEXT("WordAttackSave");

	UPROPERTY(EditDefaultsOnly, Category = "Word Attack|Save")
	int32 SaveUserIndex = 0;

	UFUNCTION()
	void HandleTimerUpdated(float TimeLeft);

	UFUNCTION()
	void HandleTimerExpired();

	AWA_GameState* GetWAGameState() const;
	void LoadBestScore();
	void SaveBestScore();
};
